#ifndef CONTACT_VERIFICATION_VALIDATORS_H__
#define CONTACT_VERIFICATION_VALIDATORS_H__

#include "contact_validator.h"
#include "contact_verification_checkers.h"
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

