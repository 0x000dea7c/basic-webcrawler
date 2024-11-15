#pragma once

#include <string>
#include <unordered_set>

// i have serious doubts about this interface, but i just don't get object oriented design
// is this the best you can get?

struct url_metadata final
{
  url_metadata () : _links{}, _title{} {}

  std::unordered_set<std::string> _links;
  std::string _title;
  /* ... */
};

class http_parser
{
public:
  virtual ~http_parser () = default;
  virtual void parse (std::string const &url, std::string const &contents) = 0;
  virtual url_metadata *get_url_metadata (std::string const &url) const = 0;
};
