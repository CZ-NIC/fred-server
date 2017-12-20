#ifndef WHOIS_IS_DOMAIN_DELETE_PENDING_H_45474530115341031
#define  WHOIS_IS_DOMAIN_DELETE_PENDING_H_45474530115341031

#include "src/libfred/registrable_object/domain.hh"
#include "src/libfred/opcontext.hh"

#include <string>

namespace Whois {

    bool is_domain_delete_pending(const std::string &_fqdn, LibFred::OperationContext& _ctx, const std::string& _timezone);

}

#endif
