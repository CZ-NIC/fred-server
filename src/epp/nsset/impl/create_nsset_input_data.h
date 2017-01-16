/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_NSSET_INPUT_DATA_H_330441543C5E4F19818C057877F54560
#define CREATE_NSSET_INPUT_DATA_H_330441543C5E4F19818C057877F54560

#include "src/epp/nsset/impl/dns_host_input.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

struct CreateNssetInputData
{
    std::string handle;
    boost::optional<std::string> authinfopw;
    std::vector<DnsHostInput> dns_hosts;
    std::vector<std::string> tech_contacts;
    boost::optional<short> input_tech_check_level;
    unsigned int config_default_nsset_tech_check_level;
    unsigned int config_nsset_min_hosts;
    unsigned int config_nsset_max_hosts;

    CreateNssetInputData(
            const std::string& _handle,
            const boost::optional<std::string>& _authinfopw,
            const std::vector<DnsHostInput>& _dns_hosts,
            const std::vector<std::string>& _tech_contacts,
            const boost::optional<short>& _input_tech_check_level,
            const unsigned int _config_default_nsset_tech_check_level,
            const unsigned int _config_nsset_min_hosts,
            const unsigned int _config_nsset_max_hosts)
        : handle(_handle),
          authinfopw(_authinfopw),
          dns_hosts(_dns_hosts),
          tech_contacts(_tech_contacts),
          input_tech_check_level(_input_tech_check_level),
          config_default_nsset_tech_check_level(_config_default_nsset_tech_check_level),
          config_nsset_min_hosts(_config_nsset_min_hosts),
          config_nsset_max_hosts(_config_nsset_max_hosts)
    { }

    short get_nsset_tech_check_level() const
    {
        return input_tech_check_level
            ? input_tech_check_level.value()
            : boost::numeric_cast<short>(config_default_nsset_tech_check_level);
    }
};

} // namespace Epp::Nsset
} // namespace Epp

#endif
