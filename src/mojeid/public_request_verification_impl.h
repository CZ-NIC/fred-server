#ifndef PUBLIC_REQUEST_VERIFICATION_IMPL_H_
#define PUBLIC_REQUEST_VERIFICATION_IMPL_H_

#include "public_request/public_request.h"
#include "factory.h"


namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(verification)


const Type PRT_CONDITIONAL_CONTACT_IDENTIFICATION = "contact_conditional_identification";
const Type PRT_CONTACT_IDENTIFICATION = "contact_identification";
const Type PRT_CONTACT_VALIDATION = "contact_validation";


}
}

#endif /* PUBLIC_REQUEST_VERIFICATION_IMPL_H_ */

