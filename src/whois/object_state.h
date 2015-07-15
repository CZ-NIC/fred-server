#ifndef WHOIS_OBJECT_STATE_a29861fe6a764f9e8f78acf8100c9e83
#define WHOIS_OBJECT_STATE_a29861fe6a764f9e8f78acf8100c9e83

#include <string>
#include "src/fredlib/opcontext.h"

namespace Whois {
    std::string get_object_state_name_by_state_id(
        unsigned long long state_id,
        Fred::OperationContext& ctx);
}
#endif
