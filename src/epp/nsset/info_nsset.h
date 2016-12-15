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

/**
 *  @file
 */

#ifndef EPP_NSSET_INFO_IMPL_d14d7d8e47e14524b67f6e31847dc375
#define EPP_NSSET_INFO_IMPL_d14d7d8e47e14524b67f6e31847dc375

#include "src/epp/nsset/impl/dns_host_output.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <boost/asio/ip/address.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

struct InfoNssetOutputData {
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    std::set<std::string> states;
    boost::posix_time::ptime crdate;
    Nullable<boost::posix_time::ptime> last_update;
    Nullable<boost::posix_time::ptime> last_transfer;
    boost::optional<std::string> auth_info_pw;
    std::vector<DnsHostOutput> dns_hosts;
    std::vector<std::string> tech_contacts;
    short tech_check_level;

    InfoNssetOutputData(
        const std::string& _handle,
        const std::string& _roid,
        const std::string& _sponsoring_registrar_handle,
        const std::string& _creating_registrar_handle,
        const Nullable<std::string>& _last_update_registrar_handle,
        const std::set<std::string>& _states,
        const boost::posix_time::ptime& _crdate,
        const Nullable<boost::posix_time::ptime>& _last_update,
        const Nullable<boost::posix_time::ptime>& _last_transfer,
        const boost::optional<std::string>& _auth_info_pw,
        const std::vector<DnsHostOutput>& _dns_hosts,
        const std::vector<std::string>& _tech_contacts,
        short _tech_check_level
    ) :
        handle(_handle),
        roid(_roid),
        sponsoring_registrar_handle(_sponsoring_registrar_handle),
        creating_registrar_handle(_creating_registrar_handle),
        last_update_registrar_handle(_last_update_registrar_handle),
        states(_states),
        crdate(_crdate),
        last_update(_last_update),
        last_transfer(_last_transfer),
        auth_info_pw(_auth_info_pw),
        dns_hosts(_dns_hosts),
        tech_contacts(_tech_contacts),
        tech_check_level(_tech_check_level)
    { }
};

/**
 * @throws ExceptionAuthErrorServerClosingConnection
 * @throws ExceptionNonexistentHandle
 */
InfoNssetOutputData info_nsset(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    SessionLang::Enum _object_state_description_lang,
    unsigned long long _session_registrar_id
);

} // namespace Epp::Nsset
} // namespace Epp

#endif
