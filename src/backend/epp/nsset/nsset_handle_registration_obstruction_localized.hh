/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/nsset/nsset_handle_registration_obstruction.hh"

#include <string>

#ifndef NSSET_HANDLE_REGISTRATION_OBSTRUCTION_LOCALIZED_HH_1D523471986C43E9A8CA63149E2B38F2
#define NSSET_HANDLE_REGISTRATION_OBSTRUCTION_LOCALIZED_HH_1D523471986C43E9A8CA63149E2B38F2

namespace Epp {
namespace Nsset {

struct NssetHandleRegistrationObstructionLocalized
{
    const NssetHandleRegistrationObstruction::Enum state;
    const std::string description;


    NssetHandleRegistrationObstructionLocalized(
            const NssetHandleRegistrationObstruction::Enum _state,
            const std::string& _description)
        : state(_state),
          description(_description)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
