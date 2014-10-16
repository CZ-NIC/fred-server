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
#include "util/types/birthdate.h"

namespace Fred {
namespace Contact {
namespace Verification {

ContactValidator create_conditional_identification_validator_mojeid()
{
    ContactValidator tmp = create_conditional_identification_validator();
    tmp.add_checker(contact_checker_username);
    tmp.add_checker(contact_checker_birthday);
    return tmp;
}

ContactValidator create_finish_identification_validator_mojeid()
{
    ContactValidator tmp = create_default_contact_validator();
    return tmp;
}

ContactValidator create_verified_transfer_validator_mojeid()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_username);
    tmp.add_checker(contact_checker_birthday);
    return tmp;
}

ContactValidator create_contact_update_validator_mojeid()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_birthday);
    return tmp;
}


}
}
}
