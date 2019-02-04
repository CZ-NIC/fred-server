#ifndef CONTACT_VERIFICATION_VALIDATORS_HH_8A5530A7519C42B8A3D05D1F6AA05558
#define CONTACT_VERIFICATION_VALIDATORS_HH_8A5530A7519C42B8A3D05D1F6AA05558

#include "src/deprecated/libfred/contact_verification/contact_validator.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_checkers.hh"
#include "src/deprecated/libfred/registrable_object/contact.hh"


namespace LibFred {
namespace Contact {
namespace Verification {


ContactValidator create_default_contact_validator();
ContactValidator create_conditional_identification_validator();
ContactValidator create_identification_validator();
ContactValidator create_finish_identification_validator();

bool check_conditionally_identified_contact_diff(
        const Contact &_c1, const Contact &_c2);
bool check_validated_contact_diff(const Contact &_c1, const Contact &_c2);

typedef bool AreTheSame;
AreTheSame check_identified_contact_diff(const Contact &_c1, const Contact &_c2);

}
}
}


#endif /*CONTACT_VERIFICATION_VALIDATORS_H__*/

