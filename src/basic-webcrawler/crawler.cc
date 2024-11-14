#include "crawler.hh"
#include "robots_parser.hh"
#include <cassert>
#include <iostream>

using namespace std::string_literals;

crawler::crawler (std::unique_ptr<http_client> http_client, std::unique_ptr<http_parser> http_parser,
                  std::vector<std::string> const &seeds, robots_parser &robots_parser, size_t depth_limit)
  : _pages{}, _seeds{std::move (seeds)}, _http_client{std::move (http_client)}, _http_parser{std::move (http_parser)},
    _robots_parser{robots_parser}, _depth_limit{depth_limit}
{}

crawler::~crawler () {}

std::string
crawler::extract_domain (std::string const &url) const
{
  auto first_dot_pos = url.find ('.');
  auto second_dot_pos = url.find ('.', first_dot_pos);
  auto slash_pos = url.find ('/', second_dot_pos);
  return url.substr (0, slash_pos);
}

void
crawler::process_robots_file (std::string const &domain)
{
  auto const robots_domain = domain + "/robots.txt";
  auto const robots_contents = _http_client->get (robots_domain);
  _robots_parser.parse (domain, robots_contents);
}

void
crawler::run ()
{
  std::unordered_set<std::string> visited;
  std::string prev_domain;

  for (auto const &seed : _seeds)
    {
      _pages.emplace (seed, 0);
    }

  while (!_pages.empty ())
    {
      auto [link, depth] = _pages.front ();

      _pages.pop ();

      if (depth > _depth_limit)
        {
          continue;
        }

      auto domain = extract_domain (link);

      // TODO: do prefix-search, it's the same as path_is_allowed, almost, refactor this shit
      if (!domain_was_visited (domain, visited))
        {
          process_robots_file (domain);
        }

      visited.emplace (link);

      if (domain == prev_domain)
        {
          delay_between_requests (_robots_parser.get_domains_delay (domain));
        }

      auto page_contents = _http_client->get (link);

      if (!page_contents)
        {
          continue;
        }

      // NOTE: here is where i can get the information needed, in this case i'll just get the page title
      // and url and print it to stdout.
      std::cout << "visited URL " << link << ", page title: " << _http_parser->get_page_title (*page_contents) << '\n';

      auto links = _http_parser->extract_links (*page_contents, domain);

      for (auto const &new_link : links)
        {
          if (path_is_allowed (new_link) && visited.count (new_link) == 0)
            {
              _pages.emplace (new_link, depth + 1);
            }
        }

      prev_domain = domain;
    }
}

bool
crawler::path_is_allowed (std::string const &link) const
{
  std::string best_match;

  for (auto const &prefix : _robots_parser.get_disallowed_pages ())
    {
      if (is_prefix (prefix, link))
        {
          best_match = prefix;
          break;
        }
    }

  return best_match.empty () ? true : false;
}

bool
crawler::domain_was_visited (std::string const &domain, std::unordered_set<std::string> &visited) const
{
  std::string best_match;

  for (auto const &prefix : visited)
    {
      if (is_prefix (prefix, domain))
        {
          best_match = prefix;
          break;
        }
    }

  return best_match.empty () ? false : true;
}
