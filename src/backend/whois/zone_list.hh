#ifndef WHOIS_ZONE_LIST_7260025279f640bb8e22233e3221fb68
#define WHOIS_ZONE_LIST_7260025279f640bb8e22233e3221fb68

#include <string>
#include <vector>
#include "src/libfred/opcontext.hh"

namespace Whois {
    //list of managed zone names
    std::vector<std::string> get_managed_zone_list(
        LibFred::OperationContext& ctx);
}
#endif
