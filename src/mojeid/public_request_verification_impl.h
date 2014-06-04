#ifndef PUBLIC_REQUEST_VERIFICATION_IMPL_H_
#define PUBLIC_REQUEST_VERIFICATION_IMPL_H_

#include "src/fredlib/public_request/public_request.h"
#include "factory.h"


namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(verification)

const Type PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION = "mojeid_contact_conditional_identification";
const Type PRT_MOJEID_CONTACT_IDENTIFICATION = "mojeid_contact_identification";
const Type PRT_MOJEID_CONTACT_VALIDATION = "mojeid_contact_validation";
const Type PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER = "mojeid_conditionally_identified_contact_transfer";
const Type PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER = "mojeid_identified_contact_transfer";
const Type PRT_MOJEID_CONTACT_REIDENTIFICATION = "mojeid_contact_reidentification";

}
}

#endif /* PUBLIC_REQUEST_VERIFICATION_IMPL_H_ */

