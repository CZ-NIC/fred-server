/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "test/libfred/zone/zone_ns/util.hh"

#include <iostream>
#include <algorithm>
#include <boost/asio/ip/address.hpp>

namespace Test {

bool operator==(const ::LibFred::Zone::InfoZoneNsData& _lhs,
        const ::LibFred::Zone::InfoZoneNsData& _rhs)
{
        return (_lhs.id == _rhs.id
            && _lhs.zone_id == _rhs.zone_id
            && _lhs.nameserver_fqdn == _rhs.nameserver_fqdn
            && _lhs.nameserver_ip_addresses == _rhs.nameserver_ip_addresses);
}

} // namespace Test
