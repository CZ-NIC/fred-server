/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_NSSET_INPUT_DATA_H_3F2F31DCCD7B40A69412F2CC09D32AC7
#define UPDATE_NSSET_INPUT_DATA_H_3F2F31DCCD7B40A69412F2CC09D32AC7

#include "src/epp/nsset/impl/dns_host_input.h"
#include "util/optional_value.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

struct UpdateNssetInputData
{
    std::string handle;
    Optional<std::string> authinfopw;
    std::vector<DnsHostInput> dns_hosts_add;
    std::vector<DnsHostInput> dns_hosts_rem;
    std::vector<std::string> tech_contacts_add;
    std::vector<std::string> tech_contacts_rem;
    boost::optional<short> tech_check_level;
    unsigned int config_nsset_min_hosts;
    unsigned int config_nsset_max_hosts;

    UpdateNssetInputData(
            const std::string& _handle,
            const Optional<std::string>& _authinfopw,
            const std::vector<DnsHostInput>& _dns_hosts_add,
            const std::vector<DnsHostInput>& _dns_hosts_rem,
            const std::vector<std::string>& _tech_contacts_add,
            const std::vector<std::string>& _tech_contacts_rem,
            const boost::optional<short>& _tech_check_level,
            const unsigned int _config_nsset_min_hosts,
            const unsigned int _config_nsset_max_hosts)
        : handle(_handle),
          authinfopw(_authinfopw),
          dns_hosts_add(_dns_hosts_add),
          dns_hosts_rem(_dns_hosts_rem),
          tech_contacts_add(_tech_contacts_add),
          tech_contacts_rem(_tech_contacts_rem),
          tech_check_level(_tech_check_level),
          config_nsset_min_hosts(_config_nsset_min_hosts),
          config_nsset_max_hosts(_config_nsset_max_hosts)
    { }
};

} // namespace Epp::Nsset
} // namespace Epp

#endif
