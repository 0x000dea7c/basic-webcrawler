#include "robots_parser.hh"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <cassert>

using namespace std::string_literals;

int
main ()
{
  robots_parser robots_parser (1);
  std::ifstream file ("robots.txt"s);

  if (!file)
    {
      std::cerr << "couldn't open test file.\n";
      return EXIT_FAILURE;
    }

  {
    std::stringstream file_contents;
    file_contents << file.rdbuf ();

    robots_parser.parse ("https://www.wikipedia.org"s, file_contents.str ());

    auto disallowed_pages = robots_parser.get_disallowed_pages ();

    assert (!disallowed_pages.empty ());
    assert (disallowed_pages.count ("https://www.wikipedia.org/w"s) != 0);
    assert (disallowed_pages.count ("https://www.wikipedia.org/api"s) != 0);
    assert (disallowed_pages.count ("https://www.wikipedia.org/trap"s) != 0);
    assert (robots_parser.get_domains_delay ("https://www.wikipedia.org"s) == 69);
  }

  return EXIT_SUCCESS;
}
