#ifndef IS_DOMAIN_DELETE_PENDING_HH_AA08E1F658E14E74BE2C58B65ED936F7
#define IS_DOMAIN_DELETE_PENDING_HH_AA08E1F658E14E74BE2C58B65ED936F7

#include "libfred/opcontext.hh"
#include "src/deprecated/libfred/registrable_object/domain.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace Whois {

bool is_domain_delete_pending(const std::string& _fqdn, LibFred::OperationContext& _ctx, const std::string& _timezone);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
