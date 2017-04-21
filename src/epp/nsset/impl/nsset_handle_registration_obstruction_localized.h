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

#include "src/epp/nsset/impl/nsset_handle_registration_obstruction.h"

#include <string>

#ifndef NSSET_HANDLE_REGISTRATION_OBSTRUCTION_LOCALIZED_H_A1055DFD10954831AD06F6E614232F4A
#define NSSET_HANDLE_REGISTRATION_OBSTRUCTION_LOCALIZED_H_A1055DFD10954831AD06F6E614232F4A

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
    { }
};

} // namespace Epp::Nsset
} // namespace Epp

#endif
