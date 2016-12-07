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

#ifndef CHECK_CONTACT_H_19C1ED6F0967492CB555556A6BD1317D
#define CHECK_CONTACT_H_19C1ED6F0967492CB555556A6BD1317D

#include "src/epp/contact/impl/contact_handle_registration_obstruction.h"
#include "src/fredlib/opcontext.h"

#include <map>
#include <set>
#include <string>

namespace Epp {

/**
 * @returns check results for given contact handles
 */
std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > contact_check_impl(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _contact_handles
);

}

#endif
