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

#ifndef REGISTRAR_ZONE_ACCESS_TYPE_HH_584CBFFFE37842FCB727553E142592A1
#define REGISTRAR_ZONE_ACCESS_TYPE_HH_584CBFFFE37842FCB727553E142592A1

#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <vector>

namespace LibFred {
namespace Registrar {
namespace ZoneAccess {

struct ZoneAccess
{
    unsigned long long id = 0;
    std::string zone_fqdn;
    boost::gregorian::date from_date;
    boost::gregorian::date to_date;
};

struct RegistrarZoneAccesses
{
    std::string registrar_handle;
    std::vector<ZoneAccess> zone_accesses;
};

} // namespace LibFred::Registrar::ZoneAccess
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
