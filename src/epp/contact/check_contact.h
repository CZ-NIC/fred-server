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

#ifndef CHECK_CONTACT_H_19C1ED6F0967492CB555556A6BD1317D
#define CHECK_CONTACT_H_19C1ED6F0967492CB555556A6BD1317D

#include "src/epp/contact/impl/contact_handle_registration_obstruction.h"
#include "src/fredlib/opcontext.h"

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Contact {

/**
 * @returns check results for given contact handles
 */
std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > check_contact(
        Fred::OperationContext& _ctx,
        const std::set<std::string>& _contact_handles,
        unsigned long long _registrar_id);

} // namespace Epp::Contact
} // namespace Epp

#endif
