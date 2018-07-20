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

#ifndef UTIL_HH_768620FFC83747FE8C6479F25B1F4031
#define UTIL_HH_768620FFC83747FE8C6479F25B1F4031

#include "src/libfred/opcontext.hh"
#include "src/libfred/zone/info_zone_data.hh"
#include "test/setup/fixtures.hh"

namespace Test {

struct ContextHolder
    : virtual instantiate_db_template
{
    LibFred::OperationContextCreator ctx;
};

template <class T>
struct SupplyFixtureCtx : ContextHolder, T
{
    SupplyFixtureCtx()
        : ContextHolder(),
          T(ctx)
    {
    }
};

bool operator==(const ::LibFred::Zone::EnumZone& _lhs, const ::LibFred::Zone::EnumZone& _rhs);

bool operator==(const ::LibFred::Zone::NonEnumZone& _lhs, const ::LibFred::Zone::NonEnumZone& _rhs);

} // namespace Test
#endif
