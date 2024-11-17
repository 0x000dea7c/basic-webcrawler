#pragma once

#include <memory>
#include "http_parser.hh"

class http_parser_factory
{
public:
  virtual ~http_parser_factory () = default;
  virtual std::unique_ptr<http_parser> create_instance () = 0;
};
