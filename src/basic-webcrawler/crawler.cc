#include "crawler.hh"
#include "robots_parser.hh"
#include <cassert>
#include <iostream>
#include "common.hh"

using namespace std::string_literals;

crawler::crawler (std::unique_ptr<http_client> http_client, std::unique_ptr<http_parser> http_parser,
                  std::vector<std::string> const &seeds, robots_parser &robots_parser, size_t depth_limit,
                  std::string metadata_filename)
  : _pages{}, _seeds{std::move (seeds)}, _metadata_filename{metadata_filename}, _metadata_file{},
    _http_client{std::move (http_client)}, _http_parser{std::move (http_parser)}, _robots_parser{robots_parser},
    _depth_limit{depth_limit}
{
  assert (!metadata_filename.empty ());

  _metadata_file.open (_metadata_filename);

  if (!_metadata_file)
    {
      throw std::runtime_error ("couldn't open file to store crawler's output.\n");
    }

  // REVIEW: trying
  std::vector<char> buff (8192);
  _metadata_file.rdbuf ()->pubsetbuf (buff.data (), (long) buff.size ());
}

crawler::~crawler () {}

void
crawler::process_robots_file (std::string const &domain)
{
  auto const robots_domain = domain + "/robots.txt";
  auto const robots_contents = _http_client->get (robots_domain);
  _robots_parser.parse (domain, robots_contents);
}

void
crawler::run ()
{
  std::unordered_set<std::string> visited;
  std::string prev_domain;

  for (auto const &seed : _seeds)
    {
      _pages.emplace (seed, 0);
    }

  while (!_pages.empty ())
    {
      auto [link, depth] = _pages.front ();

      _pages.pop ();

      if (visited.count (link) > 0)
        {
          continue;
        }

      if (depth > _depth_limit)
        {
          continue;
        }

      auto domain = get_domain (link);

      if (!domain_was_visited (domain, visited))
        {
          process_robots_file (domain);
        }

      visited.emplace (link);

      if (domain == prev_domain)
        {
          delay_between_requests (_robots_parser.get_domains_delay (domain));
        }

      auto page_contents = _http_client->get (link);

      if (!page_contents)
        {
          continue;
        }

      _http_parser->parse (link, *page_contents);

      auto *link_metadata = _http_parser->get_url_metadata (link);

      process_page_metadata (link, link_metadata->_title);

      auto links = link_metadata->_links;

      for (auto const &new_link : links)
        {
          if (path_is_allowed (new_link) && visited.count (new_link) == 0)
            {
              _pages.emplace (new_link, depth + 1);
            }
        }

      prev_domain = domain;
    }
}

bool
crawler::path_is_allowed (std::string const &link) const
{
  std::string best_match;

  for (auto const &prefix : _robots_parser.get_disallowed_pages ())
    {
      if (is_prefix (prefix, link))
        {
          best_match = prefix;
          break;
        }
    }

  return best_match.empty () ? true : false;
}

bool
crawler::domain_was_visited (std::string const &domain, std::unordered_set<std::string> &visited) const
{
  std::string best_match;

  for (auto const &prefix : visited)
    {
      if (is_prefix (prefix, domain))
        {
          best_match = prefix;
          break;
        }
    }

  return best_match.empty () ? false : true;
}

void
crawler::process_page_metadata (std::string const &link, std::string const &title)
{
  // very simple stuff here, just write link + page title
  _metadata_file << "Link: " << link << ", title: " << title << '\n';
}
