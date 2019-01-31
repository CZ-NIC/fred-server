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

#ifndef INFO_NSSET_LOCALIZED_OUTPUT_DATA_HH_E09825B0CC864FB69CE407DA737A40BC
#define INFO_NSSET_LOCALIZED_OUTPUT_DATA_HH_E09825B0CC864FB69CE407DA737A40BC

#include "src/backend/epp/nsset/dns_host_output.hh"
#include "src/backend/epp/object_states_localized.hh"
#include "src/backend/epp/nsset/status_value.hh"
#include "util/db/nullable.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

struct InfoNssetLocalizedOutputData
{
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    ObjectStatesLocalized<Epp::Nsset::StatusValue> localized_external_states;
    boost::posix_time::ptime crdate;
    Nullable<boost::posix_time::ptime> last_update;
    Nullable<boost::posix_time::ptime> last_transfer;
    boost::optional<std::string> authinfopw;
    std::vector<DnsHostOutput> dns_host;
    std::vector<std::string> tech_contacts;
    short tech_check_level;


    InfoNssetLocalizedOutputData(
            const std::string& _handle,
            const std::string& _roid,
            const std::string& _sponsoring_registrar_handle,
            const std::string& _creating_registrar_handle,
            const Nullable<std::string>& _last_update_registrar_handle,
            const ObjectStatesLocalized<StatusValue> _localized_external_states,
            const boost::posix_time::ptime& _crdate,
            const Nullable<boost::posix_time::ptime>& _last_update,
            const Nullable<boost::posix_time::ptime>& _last_transfer,
            const boost::optional<std::string>& _authinfopw,
            const std::vector<DnsHostOutput>& _dns_host,
            const std::vector<std::string>& _tech_contacts,
            short _tech_check_level)
        : handle(_handle),
          roid(_roid),
          sponsoring_registrar_handle(_sponsoring_registrar_handle),
          creating_registrar_handle(_creating_registrar_handle),
          last_update_registrar_handle(_last_update_registrar_handle),
          localized_external_states(_localized_external_states),
          crdate(_crdate),
          last_update(_last_update),
          last_transfer(_last_transfer),
          authinfopw(_authinfopw),
          dns_host(_dns_host),
          tech_contacts(_tech_contacts),
          tech_check_level(_tech_check_level)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
