#pragma once

#include "http_parser_factory.hh"
#include "lexbor_http_parser.hh"

class lexbor_http_parser_factory final : public http_parser_factory
{
public:
  ~lexbor_http_parser_factory () = default;
  std::unique_ptr<http_parser> create_instance () override { return std::make_unique<lexbor_http_parser> (); }
};
