#ifndef PUBLIC_REQUEST_PERSONAL_INFO_IMPL_HH_80066B98FFF38FCAF44A43B14027DC88
#define PUBLIC_REQUEST_PERSONAL_INFO_IMPL_HH_80066B98FFF38FCAF44A43B14027DC88

#include "src/deprecated/libfred/public_request/public_request.hh"
#include "util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(personal_info)


static const Type PRT_PERSONALINFO_AUTO_PIF = "personalinfo_auto_pif";
static const Type PRT_PERSONALINFO_EMAIL_PIF = "personalinfo_email_pif";
static const Type PRT_PERSONALINFO_POST_PIF = "personalinfo_post_pif";
static const Type PRT_PERSONALINFO_GOVERNMENT_PIF = "personalinfo_government_pif";

} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif
