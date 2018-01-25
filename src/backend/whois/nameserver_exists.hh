#ifndef NAMESERVER_EXISTS_HH_124FA62B297049ABBF8E0591538D8291
#define NAMESERVER_EXISTS_HH_124FA62B297049ABBF8E0591538D8291

#include <string>

#include "src/libfred/opcontext.hh"

namespace Whois {
    bool nameserver_exists(
        const std::string& ns_fqdn,
        LibFred::OperationContext& ctx);
}
#endif