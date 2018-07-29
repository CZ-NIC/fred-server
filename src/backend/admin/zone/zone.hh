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

#ifndef ZONE_HH_B9DCA511FCD149BB9BEA44F320E7196F
#define ZONE_HH_B9DCA511FCD149BB9BEA44F320E7196F

#include <string>

namespace Admin {
namespace Zone {

void add_zone(
        const std::string& _fqdn,
        int _expiration_period_min,
        int _expiration_period_max,
        int _ttl,
        const std::string& _hostmaster,
        int _refresh,
        int _update_retr,
        int _expiry,
        int _minimum,
        const std::string& _ns_fqdn);

void add_zone_ns(
        const std::string& _zone_fqdn_,
        const std::string& _nameserver_fqdn_,
        const std::string& _nameserver_ip_addresses);

} // namespace Admin::Zone
} // namespace Zone

#endif
