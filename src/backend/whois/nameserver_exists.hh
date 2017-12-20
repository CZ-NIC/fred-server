#ifndef WHOIS_NAMESERVER_EXISTS_56468541244351
#define WHOIS_NAMESERVER_EXISTS_56468541244351

#include <string>

#include "src/libfred/opcontext.hh"

namespace Whois {
    bool nameserver_exists(
        const std::string& ns_fqdn,
        LibFred::OperationContext& ctx);
}
#endif
