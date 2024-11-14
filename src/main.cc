#include "curl_http_client.hh"
#include "http_client.hh"
#include "http_parser.hh"
#include "lexbor_http_parser.hh"
#include "crawler.hh"
#include "config.hh"
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std::string_literals;

static void
init_app ()
{
  std::cin.tie (nullptr);
}

int
main ()
{
  init_app ();

  std::unique_ptr<http_client> httpcli{std::make_unique<curl_http_client> ()};

  std::unique_ptr<http_parser> httppar{std::make_unique<lexbor_http_parser> ()};

  std::vector<std::string> const seeds{SEED_URL_1, SEED_URL_2, SEED_URL_3};

  robots_parser robots_parser (DEFAULT_REQUEST_DELAY);

  crawler c (std::move (httpcli), std::move (httppar), std::move (seeds), robots_parser, DEFAULT_DEPTH_LIMIT);

  c.run ();

  return EXIT_SUCCESS;
}