#ifndef PUBLIC_REQUEST_AUTHINFO_IMPL_HH_0C27FF53A0AD41F195A8864D0B21C4B4
#define PUBLIC_REQUEST_AUTHINFO_IMPL_HH_0C27FF53A0AD41F195A8864D0B21C4B4

#include "src/libfred/public_request/public_request.hh"
#include "src/util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(authinfo)


static const Type PRT_AUTHINFO_AUTO_RIF = "authinfo_auto_rif";
static const Type PRT_AUTHINFO_AUTO_PIF = "authinfo_auto_pif";
static const Type PRT_AUTHINFO_EMAIL_PIF = "authinfo_email_pif";
static const Type PRT_AUTHINFO_POST_PIF = "authinfo_post_pif";
static const Type PRT_AUTHINFO_GOVERNMENT_PIF = "authinfo_government_pif";

} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif
