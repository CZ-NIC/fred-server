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

/**
 *  @file
 */

#ifndef EPP_CONTACT_HANDLE_STATE_TO_CHECK_RESULT_33847O798
#define EPP_CONTACT_HANDLE_STATE_TO_CHECK_RESULT_33847O798

#include <stdexcept>

#include "src/epp/contact/contact_handle_registration_obstruction.h"
#include "src/fredlib/contact/handle_state.h"

#include "util/db/nullable.h"

namespace Epp {

inline Nullable<ContactHandleRegistrationObstruction::Enum> contact_handle_state_to_check_result(
    const Fred::ContactHandleState::SyntaxValidity::Enum _handle_validity,
    const Fred::ContactHandleState::Registrability::Enum _handle_in_registry
) {
    if(_handle_validity == Fred::ContactHandleState::SyntaxValidity::invalid) {

        return ContactHandleRegistrationObstruction::invalid_handle;

    } else if(_handle_validity == Fred::ContactHandleState::SyntaxValidity::valid) {

        switch(_handle_in_registry) {
            case Fred::ContactHandleState::Registrability::registered           : return ContactHandleRegistrationObstruction::registered_handle;
            case Fred::ContactHandleState::Registrability::in_protection_period : return ContactHandleRegistrationObstruction::protected_handle;
            case Fred::ContactHandleState::Registrability::available         : return Nullable<ContactHandleRegistrationObstruction::Enum>();
            default: throw std::runtime_error("Invalid Handle::{SyntaxValidity, InRegistry} combination.");
        }

    } else {

        throw std::runtime_error("Invalid Handle::{SyntaxValidity, InRegistry} combination.");
    }
}

}

#endif
