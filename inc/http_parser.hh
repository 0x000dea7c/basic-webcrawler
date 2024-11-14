#pragma once

#include <string>
#include <unordered_set>

class http_parser
{
public:
  virtual ~http_parser () = default;
  virtual std::unordered_set<std::string> extract_links (std::string const &contents, std::string const &domain) = 0;
  virtual std::string get_page_title (std::string const &contents) = 0;
};
