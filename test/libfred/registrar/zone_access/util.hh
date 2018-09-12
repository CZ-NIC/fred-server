/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef UTIL_HH_DD3EDFB0F2864035BAA88C1F84FFA378
#define UTIL_HH_DD3EDFB0F2864035BAA88C1F84FFA378

#include "src/libfred/opcontext.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <string>

namespace Test {

unsigned long long get_zone_access_id(::LibFred::OperationContext& _ctx,
        const std::string _registrar,
        const std::string _zone,
        const boost::gregorian::date _from_date,
        const boost::gregorian::date _to_date);

} // namespace Test

#endif
