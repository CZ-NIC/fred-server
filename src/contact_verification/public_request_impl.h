#ifndef CONATCT_VERIFICATION_PUBLIC_REQUEST_IMPL_H_
#define CONATCT_VERIFICATION_PUBLIC_REQUEST_IMPL_H_

#include "public_request/public_request.h"
#include "factory.h"


namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(contact_verification)

const Type PRT_CONDITIONAL_CONTACT_IDENTIFICATION = "contact_conditional_identification";
const Type PRT_CONTACT_IDENTIFICATION = "contact_identification";

}
}

#endif // CONATCT_VERIFICATION_PUBLIC_REQUEST_IMPL_H_

