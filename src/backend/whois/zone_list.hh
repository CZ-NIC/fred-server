#ifndef ZONE_LIST_HH_60F796AB501E4A08A32279D6D14F17B7
#define ZONE_LIST_HH_60F796AB501E4A08A32279D6D14F17B7

#include "libfred/opcontext.hh"

#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Whois {

//list of managed zone names
std::vector<std::string> get_managed_zone_list(
        LibFred::OperationContext& ctx);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
