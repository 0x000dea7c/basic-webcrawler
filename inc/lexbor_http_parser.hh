#pragma once

#include "http_parser.hh"
#include <lexbor/html/html.h>
#include <string>

class lexbor_http_parser final : public http_parser
{
public:
  lexbor_http_parser ();
  ~lexbor_http_parser ();

  lexbor_http_parser (lexbor_http_parser &) = delete;
  lexbor_http_parser (lexbor_http_parser &&) = delete;

  lexbor_http_parser &operator= (lexbor_http_parser &) = delete;

  http_parsed_data parse (std::string const &url, std::string const &contents) override;

private:
  std::string get_protocol (std::string const &domain) const
  {
    auto pos = domain.find (':');
    return domain.substr (0, pos);
  }

  void extract (lxb_dom_node_t *node, std::string const &domain, std::string const &protocol, http_parsed_data &data);

  void
  process_node (lxb_dom_node_t *node, std::string const &domain, std::string const &protocol, http_parsed_data &data);

  void process_link (std::string link, std::string const &domain, std::string const &protocol, http_parsed_data &data);

  lxb_html_document_t *_document;
};
