#pragma once

#include "http_parser.hh"
#include <lexbor/html/html.h>
#include <unordered_map>
#include <memory>
#include <string>

class lexbor_http_parser final : public http_parser
{
public:
  lexbor_http_parser ();
  ~lexbor_http_parser ();

  lexbor_http_parser (lexbor_http_parser &) = delete;
  lexbor_http_parser (lexbor_http_parser &&) = delete;

  lexbor_http_parser &operator= (lexbor_http_parser &) = delete;

  void parse (std::string const &url, std::string const &contents) override;

  url_metadata *get_url_metadata (std::string const &url) const override { return _metadata.at (url).get (); }

private:
  std::string get_protocol (std::string const &domain) const
  {
    auto pos = domain.find (':');
    return domain.substr (0, pos);
  }

  void extract (lxb_dom_node_t *node, std::string const &url, std::string const &domain, std::string const &protocol);

  void process_node (lxb_dom_node_t *node, std::string const &url, std::string const &domain,
                     std::string const &protocol);

  void process_link (std::string link, std::string const &url, std::string const &domain, std::string const &protocol);

  std::unordered_map<std::string, std::unique_ptr<url_metadata>> _metadata;
  lxb_html_document_t *_document;
};
