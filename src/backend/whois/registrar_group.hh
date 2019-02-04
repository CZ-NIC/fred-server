#ifndef REGISTRAR_GROUP_HH_4D341FA239FE4B10ACDD71E65142B2C1
#define REGISTRAR_GROUP_HH_4D341FA239FE4B10ACDD71E65142B2C1

#include "libfred/opcontext.hh"

#include <map>
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Whois {

// maps registrar_group to list of registrar handles, current registrar group members
std::map<std::string, std::vector<std::string> > get_registrar_groups(LibFred::OperationContext& ctx);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
