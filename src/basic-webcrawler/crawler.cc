#include "crawler.hh"
#include "config.hh"
#include <cassert>
#include <thread>
#include <sstream>
#include "common.hh"

using namespace std::string_literals;

crawler::crawler (uint32_t depth_limit, uint32_t default_request_delay, std::string metadata_filename)
  : _links_queue{},
    _metadata_file{},
    _metadata_filename{metadata_filename},
    _active_threads{0},
    _depth_limit{depth_limit},
    _default_request_delay{default_request_delay}
{
  assert (!metadata_filename.empty ());
  _metadata_file.open (_metadata_filename);
  assert (_metadata_file && "couldn't open file to store crawler's output.\n");
  _links_queue.emplace (SEED_URL_1, 0);
}

crawler::~crawler () = default;

void
crawler::run (std::unique_ptr<http_client_factory> http_client_factory,
              std::unique_ptr<http_parser_factory> http_parser_factory)
{
  // the idea (prolly stupid) is to have a http client+parser instance per thread, so i don't have
  // to worry about locking in these situations. i think it doesn't make sense to share state between
  // threads when they're doing http requests or parsing an html, so that's why i'm doing it. however,
  // the rest of information needs to be shared.
  auto num_threads = std::min<uint32_t> (5, std::thread::hardware_concurrency ());
  std::vector<std::thread> pool;
  for (uint32_t i = 0; i < num_threads; ++i)
    {
      auto cli = http_client_factory->create_instance ();
      auto par = http_parser_factory->create_instance ();
      pool.emplace_back (&crawler::worker, this, std::move (cli), std::move (par));
    }
  for (auto &thread : pool)
    {
      thread.join ();
    }
}

bool
crawler::is_visited (std::string const &link, bool insert)
{
  std::lock_guard<std::mutex> lg (_visited_mutex);
  auto it = _visited_links.find (link);
  if (it != _visited_links.end ())
    {
      return true;
    }
  if (insert)
    {
      _visited_links.emplace (link);
    }
  return false;
}

void
crawler::worker (std::unique_ptr<http_client> http_client, std::unique_ptr<http_parser> http_parser)
{
  while (true)
    {
      std::pair<std::string, uint32_t> curr; // [link, current_depth]
      bool should_process{false};
      {
        // NOTE: active threads is used for checking if there aren't any threads that are currently
        // working, but didn't add any link to the queue yet.
        std::unique_lock<std::mutex> lock (_queue_mutex);
        _queue_cv.wait (lock, [this] {
          return !_links_queue.empty () || (_active_threads == 0 && _links_queue.empty ());
        }); // be extremely careful with this condition
        if (_links_queue.empty () && _active_threads == 0)
          {
            _queue_cv.notify_all ();
            return;
          }
        if (!_links_queue.empty ())
          {
            curr = _links_queue.front ();
            _links_queue.pop ();
            // notice that i'm always manipulating _active_threads when i'm locking the queue mutex, so no need for it
            // to be atomic
            ++_active_threads;
            should_process = true;
          }
      }
      if (should_process && curr.second <= _depth_limit && !is_visited (curr.first, true))
        {
          process_link (std::move (curr), http_client, http_parser);
        }
      if (should_process)
        {
          std::lock_guard<std::mutex> lock (_queue_mutex);
          --_active_threads;
          if (_active_threads == 0 && _links_queue.empty ())
            {
              _queue_cv.notify_all ();
            }
        }
    }
}

void
crawler::parse_robots_file (std::string const &domain, std::optional<std::string> const &robots_contents)
{
  if (!robots_contents)
    {
      set_domain_delay (domain, std::chrono::seconds (_default_request_delay));
      return;
    }
  //
  // NOTE: hardcoded to look for User-agent: * because i think that's where
  // the rules for my crawler are going to be. don't care about the "allow" property for now.
  // assuming robots.txt file being read is well formatted...
  //
  std::istringstream stream (*robots_contents);
  std::string current_line;
  bool need_to_append{false}, processed_delay{false};
  while (std::getline (stream, current_line))
    {
      if (current_line.find ("#"s) != std::string::npos)
        {
          continue;
        }
      if (current_line.find ("Crawl-delay:"s) != std::string::npos)
        {
          set_domain_delay (domain, std::chrono::seconds (std::stoi (current_line.substr (13))));
          processed_delay = true;
        }
      else if (current_line.find ("User-agent:"s) != std::string::npos)
        {
          auto user_agent = current_line.substr (12);

          if (user_agent == "*"s)
            {
              need_to_append = true;
            }
          else
            {
              need_to_append = false;
            }
        }
      else if (need_to_append)
        {
          if (current_line.find ("Disallow:"s) != std::string::npos)
            {
              if (current_line.size () > 9) // if the field is empty, don't process it
                {
                  auto path = current_line.substr (10);
                  if (path.back () == '/')
                    {
                      path.pop_back ();
                    }
                  set_disallowed_link (domain + path);
                }
            }
        }
    }
  if (!processed_delay)
    {
      set_domain_delay (domain, std::chrono::seconds (_default_request_delay));
    }
}

void
crawler::process_robots_file (std::string const &domain, std::unique_ptr<http_client> &http_client)
{
  auto const robots_domain = domain + "/robots.txt";
  auto const robots_contents = http_client->get (robots_domain);
  parse_robots_file (domain, robots_contents);
}

void
crawler::delay_between_requests (std::string const &domain, std::chrono::seconds delay)
{
  std::unique_lock<std::mutex> lock (_domain_next_req_mutex);
  auto now = std::chrono::steady_clock::now ();
  // get next allowed request time for this domain
  auto it = _domain_next_req_time.find (domain);
  if (it == _domain_next_req_time.end ())
    {
      _domain_next_req_time[domain] = now + delay;
      return;
    }
  else
    {
      auto next_allowed_time = it->second;
      while (now < next_allowed_time)
        {
          _domain_delay_cv.wait_until (lock, next_allowed_time);
          now = std::chrono::steady_clock::now ();
        }
      _domain_next_req_time[domain] = now + delay;
    }
}

bool
crawler::is_allowed (std::string const &link)
{
  std::lock_guard<std::mutex> lg (_disallowed_mutex);
  std::string best_match;
  for (auto const &prefix : _disallowed_links)
    {
      if (is_prefix (prefix, link))
        {
          best_match = prefix;
          break;
        }
    }
  return best_match.empty () ? true : false;
}

void
crawler::process_link (std::pair<std::string, uint32_t> link,
                       std::unique_ptr<http_client> &http_client,
                       std::unique_ptr<http_parser> &http_parser)
{
  auto domain = get_domain (link.first);
  auto delay = get_domain_delay (domain);
  if (!delay)
    {
      process_robots_file (domain, http_client);
      delay_between_requests (domain, *get_domain_delay (domain));
    }
  else
    {
      delay_between_requests (domain, *delay);
    }
  auto link_contents = http_client->get (link.first);
  if (!link_contents)
    {
      return;
    }
  auto parsed_data = http_parser->parse (link.first, *link_contents);
  process_page_metadata (link.first, parsed_data._title);
  for (auto const &new_link : parsed_data._links)
    {
      if (is_allowed (new_link) && !is_visited (new_link, false))
        {
          {
            std::lock_guard<std::mutex> lg (_queue_mutex);
            _links_queue.emplace (new_link, link.second + 1);
            _queue_cv.notify_all (); // wake up fuckers
          }
        }
    }
}

void
crawler::process_page_metadata (std::string const &link, std::string const &title)
{
  std::lock_guard<std::mutex> lg (_file_mutex);
  // very simple stuff here, just write link + page title
  _metadata_file << "Link: " << link << ", title: " << title << '\n';
}
