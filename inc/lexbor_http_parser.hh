#pragma once

#include "http_parser.hh"
#include <lexbor/html/html.h>
#include <unordered_set>
#include <string>

class lexbor_http_parser final : public http_parser
{
public:
  lexbor_http_parser ();
  ~lexbor_http_parser ();

  lexbor_http_parser (lexbor_http_parser &) = delete;
  lexbor_http_parser (lexbor_http_parser &&) = delete;

  lexbor_http_parser &operator= (lexbor_http_parser &) = delete;

  std::unordered_set<std::string> extract_links (std::string const &contents, std::string const &domain) override;

  std::string get_page_title (std::string const &contents) override;

private:
  std::string get_protocol (std::string const &domain) const
  {
    auto pos = domain.find (':');
    return domain.substr (0, pos);
  }

  void extract (lxb_dom_node_t *node, std::unordered_set<std::string> &links, std::string const &domain,
                std::string const &protocol);

  lxb_html_document_t *_document;
};
