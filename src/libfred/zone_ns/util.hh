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

#ifndef UTIL_HH_F13E44D632884171A2427E7F474CC8E9
#define UTIL_HH_F13E44D632884171A2427E7F474CC8E9

#include <boost/asio/ip/address.hpp>
#include <exception>
#include <string>
#include <vector>

namespace LibFred {
namespace ZoneNs {

struct CreateZoneNsException : std::exception
{
    const char* what() const noexcept override
    {
        return "Failed to create zone ns due to an unknown exception.";
    }
};

std::string ip_addresses_to_string(
        const std::vector<boost::asio::ip::address> _ip_addresses);

} // namespace LibFred::ZoneNs
} // namespace LibFred

#endif
