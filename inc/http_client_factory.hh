#pragma once

#include <memory>
#include "http_client.hh"

class http_client_factory
{
public:
  virtual ~http_client_factory () = default;
  virtual std::unique_ptr<http_client> create_instance () = 0;
};
