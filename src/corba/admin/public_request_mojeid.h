#ifndef PUBLIC_REQUEST_MOJEID_IMPL_H_D6492DD8E64BF712362DEC5F3C671289
#define PUBLIC_REQUEST_MOJEID_IMPL_H_D6492DD8E64BF712362DEC5F3C671289

#include "src/fredlib/public_request/public_request.h"
#include "util/factory.h"

namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(mojeid)

const Type PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION = "mojeid_contact_conditional_identification";
const Type PRT_MOJEID_CONTACT_IDENTIFICATION = "mojeid_contact_identification";
const Type PRT_MOJEID_CONTACT_VALIDATION = "mojeid_contact_validation";
const Type PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER = "mojeid_conditionally_identified_contact_transfer";
const Type PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER = "mojeid_identified_contact_transfer";
const Type PRT_MOJEID_CONTACT_REIDENTIFICATION = "mojeid_contact_reidentification";
const Type PRT_MOJEID_CONTACT_PREVALIDATED_UNIDENTIFIED_TRANSFER = "mojeid_prevalidated_unidentified_contact_transfer";
const Type PRT_MOJEID_CONTACT_PREVALIDATED_TRANSFER = "mojeid_prevalidated_contact_transfer";

}
}

#endif//PUBLIC_REQUEST_MOJEID_IMPL_H_D6492DD8E64BF712362DEC5F3C671289
