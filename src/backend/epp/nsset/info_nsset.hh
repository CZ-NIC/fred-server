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
#ifndef INFO_NSSET_HH_9C05C3D3135F4A3F98BD3132C4441E61
#define INFO_NSSET_HH_9C05C3D3135F4A3F98BD3132C4441E61

#include "src/backend/epp/nsset/dns_host_output.hh"
#include "src/backend/epp/nsset/info_nsset_config_data.hh"
#include "src/backend/epp/nsset/status_value.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/session_lang.hh"
#include "libfred/opcontext.hh"
#include "util/db/nullable.hh"

#include <boost/asio/ip/address.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>
#include <vector>
namespace Epp {
namespace Nsset {

struct InfoNssetOutputData
{
    InfoNssetOutputData();
    InfoNssetOutputData(
            const std::string& _handle,
            const std::string& _roid,
            const std::string& _sponsoring_registrar_handle,
            const std::string& _creating_registrar_handle,
            const Nullable<std::string>& _last_update_registrar_handle,
            const std::set<StatusValue::Enum>& _states,
            const boost::posix_time::ptime& _crdate,
            const Nullable<boost::posix_time::ptime>& _last_update,
            const Nullable<boost::posix_time::ptime>& _last_transfer,
            const boost::optional<std::string>& _authinfopw,
            const std::vector<DnsHostOutput>& _dns_hosts,
            const std::vector<std::string>& _tech_contacts,
            short _tech_check_level);
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    std::set<Epp::Nsset::StatusValue::Enum> states;
    boost::posix_time::ptime crdate;
    Nullable<boost::posix_time::ptime> last_update;
    Nullable<boost::posix_time::ptime> last_transfer;
    boost::optional<std::string> authinfopw;
    std::vector<DnsHostOutput> dns_hosts;
    std::vector<std::string> tech_contacts;
    short tech_check_level;
};

InfoNssetOutputData info_nsset(
        LibFred::OperationContext& _ctx,
        const std::string& _nsset_handle,
        const InfoNssetConfigData& _info_nsset_config_data,
        const SessionData& _session_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
