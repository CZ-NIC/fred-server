#ifndef PUBLIC_REQUEST_CONTACT_VERIFICATION_IMPL_HH_91F6DA6A61164878BE2AC425B01B71AA
#define PUBLIC_REQUEST_CONTACT_VERIFICATION_IMPL_HH_91F6DA6A61164878BE2AC425B01B71AA

#include "src/deprecated/libfred/public_request/public_request.hh"
#include "util/factory.hh"

namespace Fred {
namespace Backend {
namespace ContactVerification {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(contact_verification)

const LibFred::PublicRequest::Type PRT_CONTACT_CONDITIONAL_IDENTIFICATION = "contact_conditional_identification";
const LibFred::PublicRequest::Type PRT_CONTACT_IDENTIFICATION = "contact_identification";

} // namespace Fred::Backend::ContactVerification::PublicRequest
} // namespace Fred::Backend::ContactVerification
} // namespace Fred::Backend
} // namespace Fred

#endif//CONTACT_VERIFICATION_PUBLIC_REQUEST_IMPL_H_F64C96477171CEB486A675DFFEF62965

