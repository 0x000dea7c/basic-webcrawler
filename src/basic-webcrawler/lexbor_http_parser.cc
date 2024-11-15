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
    size_t title_len{0};
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
  while (node)
    {
      if (node->type == LXB_DOM_NODE_TYPE_ELEMENT)
        {
          process_node (node, url, domain, protocol);
        }

      extract (node->first_child, url, domain, protocol);

      node = node->next;
    }
}

void
lexbor_http_parser::process_node (lxb_dom_node_t *node, std::string const &url, std::string const &domain,
                                  std::string const &protocol)
{
  auto *element = lxb_dom_interface_element (node);
  auto const *tag_name = lxb_dom_element_qualified_name (element, nullptr);

  if (std::strncmp ((char const *) tag_name, "a", 1) != 0)
    {
      // if the element isn't a link, no need to keep going
      return;
    }

  auto *attribute = lxb_dom_element_attr_by_name (element, (lxb_char_t const *) "href", 4);

  if (!attribute)
    {
      return;
    }

  auto const *link_value = lxb_dom_attr_value (attribute, nullptr);

  if (!link_value)
    {
      return;
    }

  process_link (std::string ((char const *) link_value), url, domain, protocol);
}

void
lexbor_http_parser::process_link (std::string link, std::string const &url, std::string const &domain,
                                  std::string const &protocol)
{
  // if the link is empty or contains crap (unwanted material), skip it
  if (link.empty () || link.front () == '#' || link.find ("mailto:"s) != std::string::npos
      || link.find ("javascript:"s) != std::string::npos || link.find (".pdf"s) != std::string::npos
      || link.find ("/login"s) != std::string::npos || link.find ("/auth"s) != std::string::npos
      || link.find (".png"s) != std::string::npos || link.find (".jpg"s) != std::string::npos)
    {
      return;
    }

  // normalise the link
  if (link.compare (0, 2, "//") == 0)
    {
      // protocol-relative URL (e.g., //example.com)
      link = protocol + ":" + link;
    }
  else if (link.compare (0, 2, "./") == 0)
    {
      // relative URL starting with './' (e.g., ./path/page)
      link = domain + link.substr (1);
    }
  else if (link[0] == '/')
    {
      // absolute path relative URL (e.g., /path/page)
      link = domain + link;
    }
  else if (link.find ("http") != 0)
    {
      // relative URL without leading slash (e.g., path/page)
      link = domain + "/" + link;
    }

  // remove fragment identifiers (anything after '#')
  auto fragment_pos = link.find ('#');
  if (fragment_pos != std::string::npos)
    {
      link.erase (fragment_pos);
    }

  // remove trailing slash unless it's the root URL
  if (link.length () > domain.length () + 1 && link.back () == '/')
    {
      link.pop_back ();
    }

  _metadata[url]->_links.emplace (std::move (link));
}
