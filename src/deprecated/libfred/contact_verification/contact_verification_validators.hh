/*
 * Copyright (C) 2012  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
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

