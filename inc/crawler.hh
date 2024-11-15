#pragma once

#include "http_client.hh"
#include "http_parser.hh"
#include <memory>
#include <queue>
#include <vector>
#include <thread>
#include <fstream>
#include "robots_parser.hh"
#include <unordered_set>

class crawler final
{
public:
  crawler (std::unique_ptr<http_client> http_client, std::unique_ptr<http_parser> http_parser,
           std::vector<std::string> const &seeds, robots_parser &robots_parser, size_t depth_limit,
           std::string metadata_filename);
  ~crawler ();

  crawler (crawler &) = delete;
  crawler (crawler &&) = delete;

  crawler &operator= (crawler &) = delete;

  void run ();

private:
  void process_robots_file (std::string const &url);

  bool is_prefix (std::string const &prefix, std::string const &str) const
  {
    return str.compare (0, prefix.length (), prefix) == 0;
  }

  bool path_is_allowed (std::string const &link) const;

  void delay_between_requests (int time) const
  {
    std::chrono::seconds t (time);
    std::this_thread::sleep_for (t);
  }

  bool domain_was_visited (std::string const &domain, std::unordered_set<std::string> &visited) const;

  void process_page_metadata (std::string const &link, std::string const &title);

  std::queue<std::pair<std::string, size_t>> _pages;
  std::vector<std::string> const _seeds;
  std::string _metadata_filename;
  std::ofstream _metadata_file;
  std::unique_ptr<http_client> _http_client;
  std::unique_ptr<http_parser> _http_parser;
  robots_parser &_robots_parser;
  size_t _depth_limit;
};
