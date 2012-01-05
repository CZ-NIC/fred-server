#ifndef CONTACT_VERIFICATION_H__
#define CONTACT_VERIFICATION_H__

#include "data_validation.h"


namespace Fred {
namespace Contact {
namespace Verification {


ContactValidator create_conditional_identification_validator();
ContactValidator create_identification_validator();
ContactValidator create_finish_identification_validator();
ContactValidator create_contact_update_validator();

bool check_validated_contact_diff(const Contact &_c1, const Contact &_c2);


}
}
}


#endif /*CONTACT_VERIFICATION_H__*/

