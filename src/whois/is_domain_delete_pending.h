#ifndef WHOIS_IS_DOMAIN_DELETE_PENDING_H_45474530115341031
#define  WHOIS_IS_DOMAIN_DELETE_PENDING_H_45474530115341031

#include <string>
#include <src/fredlib/domain.h>
#include "src/fredlib/opcontext.h"

namespace Whois {

    bool is_domain_delete_pending(const std::string &_fqdn, Fred::OperationContext& _ctx);

}

#endif
