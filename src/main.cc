#include "curl_http_client_factory.hh"
#include "lexbor_http_parser_factory.hh"
#include "crawler.hh"
#include "config.hh"
#include <cstring>
#include <curl/curl.h>
#include <memory>

int
main ()
{
  curl_global_init (CURL_GLOBAL_DEFAULT);
  auto http_client_factory = std::make_unique<curl_http_client_factory> ();
  auto http_parser_factory = std::make_unique<lexbor_http_parser_factory> ();
  crawler c (DEFAULT_DEPTH_LIMIT, DEFAULT_REQUEST_DELAY, METADATA_FILENAME);
  c.run (std::move (http_client_factory), std::move (http_parser_factory));
  curl_global_cleanup ();
  return EXIT_SUCCESS;
}
