#ifndef CONTACT_VERIFICATION_VALIDATORS_H__
#define CONTACT_VERIFICATION_VALIDATORS_H__

#include "contact_validator.h"
#include "contact_verification_checkers.h"
#include "contact.h"


namespace Fred {
namespace Contact {
namespace Verification {


ContactValidator create_default_contact_validator();
ContactValidator create_conditional_identification_validator();
ContactValidator create_identification_validator();
ContactValidator create_finish_identification_validator();

bool check_conditionally_identified_contact_diff(
        const Contact &_c1, const Contact &_c2);
bool check_validated_contact_diff(const Contact &_c1, const Contact &_c2);


}
}
}


#endif /*CONTACT_VERIFICATION_VALIDATORS_H__*/

