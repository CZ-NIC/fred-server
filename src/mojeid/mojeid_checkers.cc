#include "src/mojeid/mojeid_checkers.h"
#include "util/types/birthdate.h"
#include <boost/algorithm/string.hpp>

namespace Fred {
namespace Contact {
namespace Verification {



/**
 * openid username format check
 */
bool contact_checker_username(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;
    /* contact handle has to be in domain-token
     * pattern ^[a-z0-9](-?[a-z0-9])*$ and max length 30
     */
    if (boost::algorithm::trim_copy(_data.handle).empty()) {
        _errors[field_username] = REQUIRED;
        result = false;
    }
    else if (_data.handle.length() > 30) {
        _errors[field_username] = INVALID;
        result = false;
    }
    else if (!boost::regex_search(
                boost::to_lower_copy(_data.handle),
                USERNAME_PATTERN)) {
        _errors[field_username] = INVALID;
        result = false;
    }

    return result;
}


bool contact_checker_birthday(const Contact &_data, FieldErrorMap &_errors)
{
    if (!_data.ssntype.isnull() && _data.ssntype.get_value() == "BIRTHDAY") {
        try {
            boost::gregorian::date tmp = birthdate_from_string_to_date(_data.ssn.get_value());
            if (tmp.is_special()) {
                throw 0;
            }
        }
        catch (...) {
            _errors[field_birth_date] = INVALID;
            return false;
        }
    }

    return true;
}


}
}
}

