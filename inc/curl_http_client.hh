#pragma once

#include "http_client.hh"
#include <curl/curl.h>

class curl_http_client final : public http_client
{
public:
  curl_http_client ();
  ~curl_http_client ();

  curl_http_client (curl_http_client &) = delete;
  curl_http_client (curl_http_client &&) = delete;
  bool operator= (curl_http_client const &) = delete;

  std::optional<std::string> get (std::string const &url) override;

private:
  static std::size_t write_callback (void *contents, std::size_t size, std::size_t nmemb, void *user_buffer);

  CURL *_curl;
};
