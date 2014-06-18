#ifndef WHOIS_NAMESERVER_EXISTS_56468541244351
#define WHOIS_NAMESERVER_EXISTS_56468541244351

#include <string>

#include "src/fredlib/opcontext.h"

namespace Whois {
    bool nameserver_exists(
        const std::string& ns_fqdn,
        Fred::OperationContext& ctx);
}
#endif
