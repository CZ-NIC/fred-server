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

#ifndef EPP_NSSET_HANDLE_STATE_TO_CHECK_RESULT_7a5cc83493334c2688b66475f085fe65
#define EPP_NSSET_HANDLE_STATE_TO_CHECK_RESULT_7a5cc83493334c2688b66475f085fe65

#include <stdexcept>

#include "src/epp/nsset/nsset_handle_registration_obstruction.h"
#include "src/fredlib/nsset/handle_state.h"

#include "util/db/nullable.h"

namespace Epp {

inline Nullable<NssetHandleRegistrationObstruction::Enum> nsset_handle_state_to_check_result(
    const Fred::NssetHandleState::SyntaxValidity::Enum _handle_validity,
    const Fred::NssetHandleState::Registrability::Enum _handle_in_registry
) {
    if(_handle_in_registry == Fred::NssetHandleState::Registrability::registered)
    {
        return NssetHandleRegistrationObstruction::registered_handle;
    }

    if(_handle_validity == Fred::NssetHandleState::SyntaxValidity::invalid)
    {
        return NssetHandleRegistrationObstruction::invalid_handle;
    }

    if(_handle_in_registry == Fred::NssetHandleState::Registrability::in_protection_period)
    {
        return NssetHandleRegistrationObstruction::protected_handle;
    }

    if(_handle_in_registry == Fred::NssetHandleState::Registrability::unregistered)
    {
        return Nullable<NssetHandleRegistrationObstruction::Enum>();
    }

    throw std::runtime_error("invalid NssetHandleState");
}

}

#endif
