#pragma once

#include <string>
#include <unordered_set>

struct http_parsed_data final
{
  http_parsed_data ()
    : _links{},
      _title{}
  {}
  std::unordered_set<std::string> _links;
  std::string _title;
};

class http_parser
{
public:
  virtual ~http_parser () = default;
  virtual http_parsed_data parse (std::string const &url, std::string const &contents) = 0;
};
