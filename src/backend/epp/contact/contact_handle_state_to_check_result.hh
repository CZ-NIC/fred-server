/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef CONTACT_HANDLE_STATE_TO_CHECK_RESULT_HH_6E6370A4037D4A0894B15D8C633C4ACD
#define CONTACT_HANDLE_STATE_TO_CHECK_RESULT_HH_6E6370A4037D4A0894B15D8C633C4ACD

#include <stdexcept>

#include "src/backend/epp/contact/contact_handle_registration_obstruction.hh"
#include "libfred/registrable_object/contact/handle_state.hh"

#include "util/db/nullable.hh"

namespace Epp {
namespace Contact {

inline Nullable<ContactHandleRegistrationObstruction::Enum> contact_handle_state_to_check_result(
        LibFred::ContactHandleState::SyntaxValidity::Enum _handle_validity,
        LibFred::ContactHandleState::Registrability::Enum _handle_registrability)
{
    switch (_handle_registrability)
    {
        case LibFred::ContactHandleState::Registrability::registered:
            return ContactHandleRegistrationObstruction::registered_handle;

        case LibFred::ContactHandleState::Registrability::in_protection_period:
            return ContactHandleRegistrationObstruction::protected_handle;

        case LibFred::ContactHandleState::Registrability::available:
            switch (_handle_validity)
            {
                case LibFred::ContactHandleState::SyntaxValidity::invalid:
                    return ContactHandleRegistrationObstruction::invalid_handle;

                case LibFred::ContactHandleState::SyntaxValidity::valid:
                    return Nullable<ContactHandleRegistrationObstruction::Enum>();
            }
            throw std::runtime_error("Invalid LibFred::ContactHandleState::SyntaxValidity::Enum value.");
    }
    throw std::runtime_error("Invalid LibFred::ContactHandleState::Registrability::Enum value.");
}

} // namespace Epp::Contact
} // namespace Epp
#endif
