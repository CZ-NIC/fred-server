#ifndef PUBLIC_REQUEST_AUTHINFO_IMPL_H_
#define PUBLIC_REQUEST_AUTHINFO_IMPL_H_

#include "src/libfred/public_request/public_request.hh"
#include "src/util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(authinfo)


const Type PRT_AUTHINFO_AUTO_RIF = "authinfo_auto_rif";
const Type PRT_AUTHINFO_AUTO_PIF = "authinfo_auto_pif";
const Type PRT_AUTHINFO_EMAIL_PIF = "authinfo_email_pif";
const Type PRT_AUTHINFO_POST_PIF = "authinfo_post_pif";


}
}


#endif /* PUBLIC_REQUEST_AUTHINFO_IMPL_H_ */

