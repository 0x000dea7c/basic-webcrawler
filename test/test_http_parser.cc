#include "http_parser.hh"
#include "lexbor_http_parser.hh"
#include <cstdlib>
#include <cassert>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_set>

using namespace std::string_literals;

static void
test_links_are_extracted_correctly (std::unordered_set<std::string> &links)
{
  // ofc i know ahead of time the links it should extract! these are not all of them, but it should be enough
  assert (links.count ("http://www.fsf.org/help-menu"s) != 0);
  assert (links.count ("https://my.fsf.org/civicrm/member-dashboard"s) != 0);
  assert (links.count ("https://www.fsf.org/@@search"s) != 0);
  assert (links.count ("https://www.fsf.org"s) != 0);
  assert (links.count ("https://gnu.org"s) != 0);
  assert (links.count ("https://www.gnu.org/philosophy/philosophy.html"s) != 0);
  assert (links.count ("http://endsoftpatents.org"s) != 0);
}

static void
test_links_are_normalised (std::unordered_set<std::string> &links)
{
  // these links that i'm testing appear in the html as relative, so they need to be normalised
  assert (links.count ("https://shop.fsf.org") != 0);
  assert (links.count ("https://www.gnu.org/licenses/license-list.html") != 0); // without #OpinionLicenses at the end
  assert (links.count ("https://www.fsf.org/licensing") != 0);
  assert (links.count ("https://www.fsf.org/associate") != 0);
  assert (links.count ("https://www.fsf.org/resources") != 0);
  assert (links.count ("https://www.fsf.org/community") != 0);
  assert (links.count ("https://www.fsf.org/donate") != 0);
  assert (links.count ("https://www.fsf.org/about/staff-and-board") != 0);
  assert (links.count ("https://www.fsf.org/about/financial") != 0);
}

static void
test_page_title (std::unique_ptr<http_parser> &parser, std::string const &contents)
{
  assert (
    parser->get_page_title (contents)
    == "Free software is a matter of liberty, not price — Free Software Foundation — Working together for free software"s);
}

int
main ()
{
  std::unique_ptr<http_parser> parser{std::make_unique<lexbor_http_parser> ()};

  std::ifstream file ("test.html"s);

  if (!file)
    {
      std::cerr << "couldn't open test file\n";
      return EXIT_FAILURE;
    }

  std::stringstream file_contents;
  file_contents << file.rdbuf ();
  auto file_contents_str = file_contents.str ();

  auto links = parser->extract_links (file_contents_str, "https://www.fsf.org"s);

  test_links_are_extracted_correctly (links);
  test_links_are_normalised (links);
  test_page_title (parser, file_contents_str);

  return EXIT_SUCCESS;
}
