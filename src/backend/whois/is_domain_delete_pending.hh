#ifndef IS_DOMAIN_DELETE_PENDING_HH_AA08E1F658E14E74BE2C58B65ED936F7
#define IS_DOMAIN_DELETE_PENDING_HH_AA08E1F658E14E74BE2C58B65ED936F7

#include "src/libfred/registrable_object/domain.hh"
#include "src/libfred/opcontext.hh"

#include <string>

namespace Whois {

    bool is_domain_delete_pending(const std::string &_fqdn, LibFred::OperationContext& _ctx, const std::string& _timezone);

}

#endif
