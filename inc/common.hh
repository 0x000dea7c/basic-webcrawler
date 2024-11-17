#pragma once

#include <string>

inline std::string
get_domain (const std::string &url)
{
  using namespace std::string_literals;
  auto protocol_end = url.find ("://"s);
  if (protocol_end == std::string::npos)
    {
      return url;
    }
  auto domain_start = protocol_end + 3;
  auto domain_end = url.find ('/', domain_start);
  if (domain_end == std::string::npos)
    {
      domain_end = url.length ();
    }
  auto domain_with_protocol = url.substr (0, domain_end);
  auto port_pos = domain_with_protocol.find (':', domain_start);
  if (port_pos != std::string::npos && port_pos < domain_end)
    {
      domain_with_protocol = domain_with_protocol.substr (0, port_pos);
    }
  return domain_with_protocol;
}
