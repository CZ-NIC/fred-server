#ifndef PUBLIC_REQUEST_AUTHINFO_IMPL_HH_0C27FF53A0AD41F195A8864D0B21C4B4
#define PUBLIC_REQUEST_AUTHINFO_IMPL_HH_0C27FF53A0AD41F195A8864D0B21C4B4

#include "src/libfred/public_request/public_request.hh"
#include "src/util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(authinfo)


extern const Type PRT_AUTHINFO_AUTO_RIF;
extern const Type PRT_AUTHINFO_AUTO_PIF;
extern const Type PRT_AUTHINFO_EMAIL_PIF;
extern const Type PRT_AUTHINFO_POST_PIF;


} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif
