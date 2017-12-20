#ifndef CONTACT_VERIFICATION_VALIDATORS_H_2258484658
#define CONTACT_VERIFICATION_VALIDATORS_H_2258484658

#include "src/libfred/contact_verification/contact_validator.hh"
#include "src/libfred/contact_verification/contact_verification_checkers.hh"
#include "src/libfred/registrable_object/contact.hh"


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

