/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @contact_verification_validators.cc
 *  mojeid contact verification
 */

#include "mojeid_validators.h"
#include <boost/algorithm/string.hpp>

namespace Fred {
namespace Contact {
namespace Verification {

///openid username format check
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


ContactValidator create_conditional_identification_validator_mojeid()
{
    ContactValidator tmp = create_conditional_identification_validator();
    tmp.add_checker(contact_checker_username);
    return tmp;
}

ContactValidator create_finish_identification_validator_mojeid()
{
    ContactValidator tmp = create_finish_identification_validator();
    tmp.add_checker(contact_checker_username);
    return tmp;
}

}
}
}
