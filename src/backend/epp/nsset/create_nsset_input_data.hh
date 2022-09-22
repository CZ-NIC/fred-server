/*
 * Copyright (C) 2017-2022  CZ.NIC, z. s. p. o.
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

#ifndef CREATE_NSSET_INPUT_DATA_HH_5460288F04D649A9A702EE75C65597DC
#define CREATE_NSSET_INPUT_DATA_HH_5460288F04D649A9A702EE75C65597DC

#include "src/backend/epp/nsset/dns_host_input.hh"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

struct CreateNssetInputData
{
    std::string handle;
    std::vector<std::string> tech_contacts;
    std::vector<DnsHostInput> dns_hosts;
    boost::optional<short> tech_check_level;

    CreateNssetInputData(
            const std::string& _handle,
            const std::vector<std::string>& _tech_contacts,
            const std::vector<DnsHostInput>& _dns_hosts,
            const boost::optional<short>& _tech_check_level)
        : handle(_handle),
          tech_contacts(_tech_contacts),
          dns_hosts(_dns_hosts),
          tech_check_level(_tech_check_level)
    {
    }
};

} // namespace Epp::Nsset
} // namespace Epp

#endif
