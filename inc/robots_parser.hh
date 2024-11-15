#pragma once

#include <string>
#include <set>
#include <map>
#include <optional>

class robots_parser final
{
public:
  robots_parser (int default_delay);
  ~robots_parser ();

  void parse (std::string const &domain, std::optional<std::string> const &robots_contents);

  auto get_domains_delay (std::string const &domain) const { return _domains_delay.at (domain); }

  auto const &get_disallowed_pages () const { return _disallowed_pages; }

private:
  std::set<std::string> _disallowed_pages;
  std::map<std::string, int> _domains_delay;

  int _default_delay;
};
