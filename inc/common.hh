#pragma once

#include <string>

inline std::string
get_domain (std::string const &url)
{
  auto first_dot_pos = url.find ('.');
  auto second_dot_pos = url.find ('.', first_dot_pos);
  auto slash_pos = url.find ('/', second_dot_pos);
  return url.substr (0, slash_pos);
}
