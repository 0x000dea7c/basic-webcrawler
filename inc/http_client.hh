#pragma once

#include <string>
#include <optional>

class http_client
{
public:
  virtual ~http_client () = default;
  virtual std::optional<std::string> get (std::string const &url) = 0;
};
