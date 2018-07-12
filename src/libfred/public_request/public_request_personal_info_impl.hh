#ifndef PUBLIC_REQUEST_PERSONAL_INFO_IMPL_HH_80066B98FFF38FCAF44A43B14027DC88
#define PUBLIC_REQUEST_PERSONAL_INFO_IMPL_HH_80066B98FFF38FCAF44A43B14027DC88

#include "src/libfred/public_request/public_request.hh"
#include "src/util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(personal_info)


extern const Type PRT_PERSONALINFO_AUTO_PIF;
extern const Type PRT_PERSONALINFO_EMAIL_PIF;
extern const Type PRT_PERSONALINFO_POST_PIF;
extern const Type PRT_PERSONALINFO_GOVERNMENT_PIF;


} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif
