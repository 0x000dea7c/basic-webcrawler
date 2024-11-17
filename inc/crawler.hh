#pragma once

#include "http_client.hh"
#include "http_parser.hh"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include "http_client_factory.hh"
#include "http_parser_factory.hh"
#include <unordered_set>
#include <unordered_map>

class crawler final
{
public:
  crawler (uint32_t depth_limit, uint32_t default_request_delay, std::string metadata_filename);
  ~crawler ();

  crawler (crawler &) = delete;
  crawler (crawler &&) = delete;

  crawler &operator= (crawler &) = delete;

  void run (std::unique_ptr<http_client_factory> http_client_factory,
            std::unique_ptr<http_parser_factory> http_parser_factory);

private:
  bool is_visited (std::string const &link, bool insert); // HACK: get rid of this bool asap

  void worker (std::unique_ptr<http_client> http_client, std::unique_ptr<http_parser> http_parser);

  void set_domain_delay (std::string const &domain, std::chrono::seconds time)
  {
    std::lock_guard<std::mutex> lg (_domain_delay_mutex);
    _domain_delay[domain] = time;
  }

  void set_disallowed_link (std::string const &link)
  {
    std::lock_guard<std::mutex> lg (_disallowed_mutex);
    _disallowed_links.emplace (link);
  }

  void parse_robots_file (std::string const &domain, std::optional<std::string> const &robots_contents);

  std::optional<std::chrono::seconds> get_domain_delay (std::string const &domain)
  {
    std::lock_guard<std::mutex> lg (_domain_delay_mutex);
    if (_domain_delay.count (domain) == 0)
      {
        return std::nullopt;
      }
    return _domain_delay[domain];
  }

  void delay_between_requests (std::string const &domain, std::chrono::seconds delay);

  void process_robots_file (std::string const &domain, std::unique_ptr<http_client> &http_client);

  void process_link (std::pair<std::string, uint32_t> link,
                     std::unique_ptr<http_client> &http_client,
                     std::unique_ptr<http_parser> &http_parser);

  bool is_prefix (std::string const &prefix, std::string const &str) const
  {
    return str.compare (0, prefix.length (), prefix) == 0;
  }

  void process_page_metadata (std::string const &link, std::string const &title);

  bool is_allowed (std::string const &link);

  std::queue<std::pair<std::string, size_t>> _links_queue;
  std::unordered_set<std::string> _visited_links;
  std::unordered_set<std::string> _disallowed_links;
  std::unordered_map<std::string, std::chrono::seconds> _domain_delay;
  std::unordered_map<std::string, std::chrono::steady_clock::time_point> _domain_next_req_time;
  std::mutex _file_mutex;
  std::mutex _queue_mutex;
  std::mutex _visited_mutex;
  std::mutex _disallowed_mutex;
  std::mutex _domain_delay_mutex;
  std::mutex _domain_next_req_mutex;
  std::condition_variable _queue_cv;
  std::condition_variable _domain_delay_cv;
  std::ofstream _metadata_file;
  std::string const _metadata_filename;
  uint32_t _active_threads;
  uint32_t const _depth_limit;
  uint32_t const _default_request_delay;
};
