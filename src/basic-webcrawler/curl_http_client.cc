#include "curl_http_client.hh"
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

using namespace std::string_literals;

curl_http_client::curl_http_client ()
  : _curl{nullptr}
{
  _curl = curl_easy_init ();
  assert (_curl && "couldn't initialise CURL\n");
}

curl_http_client::~curl_http_client () { curl_easy_cleanup (_curl); }

std::optional<std::string>
curl_http_client::get (std::string const &url)
{
  size_t constexpr max_tries{3};
  std::string buffer;
  size_t tries{0};
  bool done{false};
  curl_easy_setopt (_curl, CURLOPT_URL, url.c_str ());
  curl_easy_setopt (_curl, CURLOPT_WRITEFUNCTION, curl_http_client::write_callback);
  curl_easy_setopt (_curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_setopt (_curl,
                    CURLOPT_USERAGENT,
                    "Mozilla/5.0 (iPad; CPU OS 12_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) "
                    "Mobile/15E148");                  // crap is needed to avoid 403 forbidden errors
  curl_easy_setopt (_curl, CURLOPT_FOLLOWLOCATION, 1); // follow redirects
  curl_easy_setopt (_curl, CURLOPT_LOCALPORT, 64000);
  curl_easy_setopt (_curl, CURLOPT_LOCALPORTRANGE, 100);
  while (!done && tries < max_tries)
    {
      if (tries > 0)
        {
          std::this_thread::sleep_for (std::chrono::seconds (1));
          buffer.clear ();
        }
      auto code = curl_easy_perform (_curl);
      if (code != CURLE_OK)
        {
          std::cerr << "couldn't perform http request to URL " << url << " because: " << curl_easy_strerror (code)
                    << '\n';
          if (code == CURLE_COULDNT_CONNECT || code == CURLE_OPERATION_TIMEDOUT)
            {
              return std::nullopt;
            }
          else
            {
              return std::nullopt;
            }
        }
      long http_code{0};
      curl_easy_getinfo (_curl, CURLINFO_RESPONSE_CODE, &http_code);
      switch (http_code)
        {
        case 200:
          return buffer;
        case 301:
        case 302:
        case 303:
        case 308:
          std::cerr << "redirected from url: " << url << " to somewhere else\n";
          done = true;
          break;
        case 400:
          std::cerr << "bad request from url " << url << '\n';
          done = true;
          break;
        case 401:
          std::cerr << "unauthorised access to url: " << url << '\n';
          done = true;
          break;
        case 403:
          std::cerr << "forbidden access to url: " << url << '\n';
          done = true;
          break;
        case 404:
          std::cerr << "url: " << url << " not found\n";
          done = true;
          break;
        case 500:
          std::cerr << "internal server error at url: " << url << '\n';
          break;
        case 503:
          std::cerr << "service unavailable at url: " << url << '\n';
          break;
        default:
          std::cerr << "unhandled http response code " << http_code << " for url: " << url << '\n';
          done = true;
          break;
        }
      ++tries;
    }

  return std::nullopt;
}

std::size_t
curl_http_client::write_callback (void *contents, std::size_t size, std::size_t nmemb, void *user_buffer)
{
  reinterpret_cast<std::string *> (user_buffer)->append (reinterpret_cast<char *> (contents), size * nmemb);
  return size * nmemb;
}
