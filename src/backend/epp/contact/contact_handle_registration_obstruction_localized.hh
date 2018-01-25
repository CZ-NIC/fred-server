/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef CONTACT_HANDLE_REGISTRATION_OBSTRUCTION_LOCALIZED_HH_32C62FA74DEF4F19BE297939B6BD5E19
#define CONTACT_HANDLE_REGISTRATION_OBSTRUCTION_LOCALIZED_HH_32C62FA74DEF4F19BE297939B6BD5E19

#include "src/backend/epp/contact/contact_handle_registration_obstruction.hh"

#include <string>

namespace Epp {
namespace Contact {

struct ContactHandleRegistrationObstructionLocalized
{
    const ContactHandleRegistrationObstruction::Enum state;
    const std::string description;


    ContactHandleRegistrationObstructionLocalized(
            const ContactHandleRegistrationObstruction::Enum _state,
            const std::string& _description)
        : state(_state),
          description(_description)
    {
    }

};


} // namespace Epp::Contact
} // namespace Epp

#endif