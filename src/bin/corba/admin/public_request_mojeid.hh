#ifndef PUBLIC_REQUEST_MOJEID_HH_B49E93ABF36F4D18A0E3190E6351A5AA
#define PUBLIC_REQUEST_MOJEID_HH_B49E93ABF36F4D18A0E3190E6351A5AA

#include "src/deprecated/libfred/public_request/public_request.hh"
#include "util/factory.hh"

namespace CorbaConversion {
namespace Admin {

FACTORY_MODULE_INIT_DECL(mojeid)

const LibFred::PublicRequest::Type PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION = "mojeid_contact_conditional_identification";
const LibFred::PublicRequest::Type PRT_MOJEID_CONTACT_IDENTIFICATION = "mojeid_contact_identification";
const LibFred::PublicRequest::Type PRT_MOJEID_CONTACT_VALIDATION = "mojeid_contact_validation";
const LibFred::PublicRequest::Type PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER = "mojeid_conditionally_identified_contact_transfer";
const LibFred::PublicRequest::Type PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER = "mojeid_identified_contact_transfer";
const LibFred::PublicRequest::Type PRT_MOJEID_CONTACT_REIDENTIFICATION = "mojeid_contact_reidentification";
const LibFred::PublicRequest::Type PRT_MOJEID_CONTACT_PREVALIDATED_UNIDENTIFIED_TRANSFER = "mojeid_prevalidated_unidentified_contact_transfer";
const LibFred::PublicRequest::Type PRT_MOJEID_CONTACT_PREVALIDATED_TRANSFER = "mojeid_prevalidated_contact_transfer";

} // namespace CorbaConversion::Admin
} // namespace CorbaConversion

#endif
