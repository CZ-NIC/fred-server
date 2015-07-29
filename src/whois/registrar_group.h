#ifndef WHOIS_REGISTRAR_GROUP_8bad5bc6a31b449295e6f7e61e07514b
#define WHOIS_REGISTRAR_GROUP_8bad5bc6a31b449295e6f7e61e07514b

#include <string>
#include <vector>
#include <map>
#include "src/fredlib/opcontext.h"

namespace Whois {
    //maps registrar_group to list of registrar handles, current registrar group members
    std::map<std::string, std::vector<std::string> > get_registrar_groups(
        Fred::OperationContext& ctx);
}
#endif
