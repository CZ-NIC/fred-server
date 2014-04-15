#ifndef MOJEID_CHECKERS_90239480__
#define MOJEID_CHECKERS_90239480__

#include "src/fredlib/contact_verification/contact_verification_checkers.h"
#include <boost/regex.hpp>
#include <string>

namespace Fred {
namespace Contact {
namespace Verification {

const boost::regex USERNAME_PATTERN("^[a-z0-9](-?[a-z0-9])*$");

const std::string field_username     = "contact.username";
const std::string field_birth_date   = "contact.birth_date";


bool contact_checker_username(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_birthday(const Contact &_data, FieldErrorMap &_errors);


}
}
}

#endif

