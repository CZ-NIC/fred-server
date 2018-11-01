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

#ifndef CREATE_ZONE_NS_HH_2D413215C56D4D7F9D5233D84B5915B8
#define CREATE_ZONE_NS_HH_2D413215C56D4D7F9D5233D84B5915B8

#include "src/libfred/opcontext.hh"

#include <boost/asio/ip/address.hpp>
#include <string>
#include <vector>

namespace LibFred {
namespace Zone {

class CreateZoneNs
{
public:
    CreateZoneNs(const std::string& _zone_fqdn, const std::string& _nameserver_fqdn);

    CreateZoneNs& set_nameserver_ip_addresses(
            const std::vector<boost::asio::ip::address>& _nameserver_ip_addresses);

    unsigned long long exec(OperationContext& _ctx) const;
private:
    std::string zone_fqdn_;
    std::string nameserver_fqdn_;
    std::vector<boost::asio::ip::address> nameserver_ip_addresses_;
};

} // namespace LibFred::Zone
} // namespace LibFred

#endif
