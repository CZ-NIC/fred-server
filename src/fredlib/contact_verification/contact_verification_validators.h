#ifndef CONTACT_VERIFICATION_VALIDATORS_H_2258484658
#define CONTACT_VERIFICATION_VALIDATORS_H_2258484658

#include "src/fredlib/contact_verification/contact_validator.h"
#include "src/fredlib/contact_verification/contact_verification_checkers.h"
#include "src/fredlib/contact.h"


namespace Fred {
namespace Contact {
namespace Verification {


ContactValidator create_default_contact_validator();
ContactValidator create_conditional_identification_validator();
ContactValidator create_identification_validator();
ContactValidator create_finish_identification_validator();


}
}
}


#endif /*CONTACT_VERIFICATION_VALIDATORS_H__*/

