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

#include "test/libfred/zone/zone_soa/util.hh"

namespace Test {

bool operator==(const ::LibFred::Zone::InfoZoneSoaData& _lhs, const ::LibFred::Zone::InfoZoneSoaData& _rhs)
{
    return (_rhs.zone == _lhs.zone
            && _rhs.ttl == _lhs.ttl
            && _rhs.hostmaster == _lhs.hostmaster
            && _rhs.refresh == _lhs.refresh
            && _rhs.update_retr == _lhs.update_retr
            && _rhs.expiry == _lhs.expiry
            && _rhs.minimum == _lhs.minimum
            && _rhs.ns_fqdn == _lhs.ns_fqdn);
}

} // namespace Test
