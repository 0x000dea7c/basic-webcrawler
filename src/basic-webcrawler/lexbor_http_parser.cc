#include "lexbor_http_parser.hh"
#include <cassert>
#include <iostream>
#include <cstring>
#include "common.hh"

using namespace std::string_literals;

lexbor_http_parser::lexbor_http_parser () : _metadata{}, _document{lxb_html_document_create ()} { assert (_document); }

lexbor_http_parser::~lexbor_http_parser () { lxb_html_document_destroy (_document); }

void
lexbor_http_parser::parse (std::string const &url, std::string const &contents)
{
  assert (!contents.empty ());
  assert (_metadata.count (url) == 0);

  _metadata[url] = std::make_unique<url_metadata> ();

  auto status = lxb_html_document_parse (_document, (lxb_char_t const *) contents.c_str (), contents.length ());

  if (status != LXB_STATUS_OK)
    {
      std::cerr << "failed to parse html document\n";
      return;
    }

  auto *root = lxb_dom_interface_element (_document->body);

  if (!root)
    {
      std::cerr << "couldn't get root dom element.\n";
      return;
    }

  {
    // get the title
    size_t title_len = 0;
    auto *lxb_title = lxb_html_document_title (_document, &title_len);
    if (lxb_title)
      {
        _metadata[url]->_title = std::string ((char const *) lxb_title);
      }
  }

  {
    // get links
    auto domain = get_domain (url);
    auto protocol = get_protocol (domain);
    extract (lxb_dom_interface_node (root), url, domain, protocol);
  }
}

void
lexbor_http_parser::extract (lxb_dom_node_t *node, std::string const &url, std::string const &domain,
                             std::string const &protocol)
{
  // REVIEW: refactor this crap
  while (node)
    {
      if (node->type == LXB_DOM_NODE_TYPE_ELEMENT)
        {
          auto *element = lxb_dom_interface_element (node);
          auto const *tag_name = lxb_dom_element_qualified_name (element, nullptr);

          if (std::strncmp ((char const *) tag_name, "a", 1) == 0)
            {
              auto *attribute = lxb_dom_element_attr_by_name (element, (lxb_char_t const *) "href", 4);

              if (attribute)
                {
                  auto const *link_value = lxb_dom_attr_value (attribute, nullptr);

                  if (link_value)
                    {
                      auto link_str = std::string ((char const *) link_value);

                      if (!link_str.empty () && link_str.find ("mailto:"s) == std::string::npos
                          && link_str.find ("javascript:"s) == std::string::npos
                          && link_str.find (".pdf"s) == std::string::npos
                          && link_str.find ("/login"s) == std::string::npos
                          && link_str.find ("/auth"s) == std::string::npos)
                        {
                          if (link_str.front () != '#') // skip references to the same page
                            {
                              if (link_str.size () > 2 && link_str[0] == '/' && link_str[1] == '/')
                                {
                                  // NOTE: protocol relative url, need to preserve http or https from current
                                  // domain
                                  link_str = protocol + ":" + link_str;
                                }
                              else if (link_str.size () > 2 && link_str[0] == '.'
                                       && link_str[1] == '/') // REVIEW: is this correct?
                                {
                                  link_str = domain + link_str.substr (1);
                                }
                              else if (link_str[0] == '/')
                                {
                                  // NOTE: absolute path relative url, means current domain + path
                                  link_str = domain + link_str;
                                }
                              else if (link_str.find ("http"s) == std::string::npos)
                                {
                                  // links like:
                                  // terms/service-specific?gl=ES&hl=en&sa=X&ved=2ahUKEwjL3r6frdyJAxUHZkECHYUpAGAQnfsGegQIABAH
                                  // that lack of the domain
                                  link_str = domain + '/' + link_str;
                                }

                              // get rid of fragment identifiers because they're essentially the same page
                              auto fragment_id_pos = link_str.find ('#');
                              if (fragment_id_pos != std::string::npos)
                                {
                                  link_str = link_str.substr (0, fragment_id_pos);
                                }

                              if (link_str.back () == '/')
                                {
                                  // no need to store the last /
                                  link_str.pop_back ();
                                }

                              _metadata[url]->_links.emplace (link_str);
                            }
                        }
                    }
                }
            }
        }

      extract (node->first_child, url, domain, protocol);
      node = node->next;
    }
}
