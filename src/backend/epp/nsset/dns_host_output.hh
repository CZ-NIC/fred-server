/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef DNS_HOST_OUTPUT_HH_D8C841CCF7484919BA3F63D63CB76D06
#define DNS_HOST_OUTPUT_HH_D8C841CCF7484919BA3F63D63CB76D06

#include <boost/asio/ip/address.hpp>
#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

/**
 * DNS host output data for nsset info.
 */
struct DnsHostOutput
{
    std::string fqdn; /**< nameserver host name*/
    std::vector<boost::asio::ip::address> inet_addr; /**< list of IPv4 or IPv6 addresses of the nameserver host*/


    /**
     * Constructor initializing all attributes.
     * @param _fqdn nameserver name
     * @param _inet_addr addresses of the nameserver
     */
    DnsHostOutput(
            const std::string& _fqdn,
            const std::vector<boost::asio::ip::address>& _inet_addr)
        : fqdn(_fqdn),
          inet_addr(_inet_addr)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
