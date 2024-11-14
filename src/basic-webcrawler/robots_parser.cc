#include "robots_parser.hh"
#include <sstream>
#include <cassert>

using namespace std::string_literals;

robots_parser::robots_parser (int default_delay) : _disallowed_pages{}, _domains_delay{}, _default_delay{default_delay}
{}

void
robots_parser::parse (std::string const &domain, std::optional<std::string> const &robots_contents)
{
  if (!robots_contents)
    {
      _domains_delay[domain] = _default_delay;
      return;
    }

  //
  // NOTE: this routine is hardcoded to look for User-agent: * because that's where
  // the rules for my crawler are going to be.
  //
  // NOTE: don't care about the allow pages for now.
  //
  // NOTE: assuming robots.txt file that is being read is well formatted...
  //
  std::istringstream stream (*robots_contents);
  std::string current_line;
  bool need_to_append{false};

  while (std::getline (stream, current_line))
    {
      if (current_line.find ("#"s) != std::string::npos)
        {
          continue;
        }

      if (current_line.find ("Crawl-delay:"s) != std::string::npos)
        {
          _domains_delay[domain] = std::stoi (current_line.substr (13));
        }
      else if (current_line.find ("User-agent:"s) != std::string::npos)
        {
          auto user_agent = current_line.substr (12);

          if (user_agent == "*"s)
            {
              need_to_append = true;
            }
          else
            {
              need_to_append = false;
            }
        }
      else if (need_to_append)
        {
          if (current_line.find ("Disallow:"s) != std::string::npos)
            {
              if (current_line.size () > 9) // if the field is empty, don't process it
                {
                  auto path = current_line.substr (10);
                  if (path.back () == '/')
                    {
                      path.pop_back ();
                    }
                  _disallowed_pages.emplace (domain + path);
                }
            }
        }
    }

  if (_domains_delay.count (domain) == 0)
    {
      _domains_delay[domain] = _default_delay;
    }
}
