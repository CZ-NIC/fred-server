/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef NSSET_HANDLE_STATE_TO_CHECK_RESULT_HH_7BBE81F082D246E1A75E720CBCDD0887
#define NSSET_HANDLE_STATE_TO_CHECK_RESULT_HH_7BBE81F082D246E1A75E720CBCDD0887

#include <stdexcept>

#include "src/backend/epp/nsset/nsset_handle_registration_obstruction.hh"
#include "libfred/registrable_object/nsset/handle_state.hh"

#include "util/db/nullable.hh"

namespace Epp {
namespace Nsset {

inline Nullable<NssetHandleRegistrationObstruction::Enum> nsset_handle_state_to_check_result(
        const LibFred::NssetHandleState::SyntaxValidity::Enum _handle_validity,
        const LibFred::NssetHandleState::Registrability::Enum _handle_in_registry)
{
    if (_handle_in_registry == LibFred::NssetHandleState::Registrability::registered)
    {
        return NssetHandleRegistrationObstruction::registered_handle;
    }

    if (_handle_in_registry == LibFred::NssetHandleState::Registrability::in_protection_period)
    {
        return NssetHandleRegistrationObstruction::protected_handle;
    }

    if (_handle_validity == LibFred::NssetHandleState::SyntaxValidity::invalid)
    {
        return NssetHandleRegistrationObstruction::invalid_handle;
    }

    if (_handle_in_registry == LibFred::NssetHandleState::Registrability::unregistered)
    {
        return Nullable<NssetHandleRegistrationObstruction::Enum>();
    }

    throw std::runtime_error("invalid NssetHandleState");
}


} // namespace Epp::Nsset
} // namespace Epp

#endif
