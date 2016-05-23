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

#ifndef EPP_NSSET_CHECK_IMPL_H_ca494567ef5445e09d94c094c9b2c7e2
#define EPP_NSSET_CHECK_IMPL_H_ca494567ef5445e09d94c094c9b2c7e2

#include "src/epp/nsset/nsset_handle_registration_obstruction.h"
#include "src/fredlib/opcontext.h"

#include <map>
#include <set>
#include <string>

namespace Epp {

/**
 * @returns check results for given nsset handles
 */
std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > nsset_check_impl(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _nsset_handles
);

}

#endif
