#ifndef PUBLIC_REQUEST_VERIFICATION_IMPL_H_
#define PUBLIC_REQUEST_VERIFICATION_IMPL_H_

#include "src/fredlib/public_request/public_request.h"
#include "factory.h"


namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(verification)

extern const Type PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION;
extern const Type PRT_MOJEID_CONTACT_IDENTIFICATION;
extern const Type PRT_MOJEID_CONTACT_VALIDATION;
extern const Type PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
extern const Type PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER;

}
}

#endif /* PUBLIC_REQUEST_VERIFICATION_IMPL_H_ */

