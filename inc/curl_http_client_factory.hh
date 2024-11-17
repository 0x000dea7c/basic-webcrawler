#pragma once

#include "http_client_factory.hh"
#include "curl_http_client.hh"

class curl_http_client_factory final : public http_client_factory
{
public:
  ~curl_http_client_factory () = default;

  std::unique_ptr<http_client> create_instance () override { return std::make_unique<curl_http_client> (); }
};
