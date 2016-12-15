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

#ifndef INFO_NSSET_LOCALIZED_H_47356271F650459CBC0999B992C5C6DD
#define INFO_NSSET_LOCALIZED_H_47356271F650459CBC0999B992C5C6DD

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/ip/address.hpp>
#include "src/epp/nsset/impl/dns_host_output.h"

namespace Epp {
namespace Nsset {

struct InfoNssetLocalizedOutputData
{
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    std::map<std::string, std::string> localized_external_states;
    boost::posix_time::ptime crdate;
    Nullable<boost::posix_time::ptime> last_update;
    Nullable<boost::posix_time::ptime> last_transfer;
    Nullable<std::string> auth_info_pw;
    std::vector<DnsHostOutput> dns_host;
    std::vector<std::string> tech_contacts;
    short tech_check_level;

    InfoNssetLocalizedOutputData(
        const std::string& _handle,
        const std::string& _roid,
        const std::string& _sponsoring_registrar_handle,
        const std::string& _creating_registrar_handle,
        const Nullable<std::string>& _last_update_registrar_handle,
        const std::map<std::string, std::string>& _localized_external_states,
        const boost::posix_time::ptime& _crdate,
        const Nullable<boost::posix_time::ptime>& _last_update,
        const Nullable<boost::posix_time::ptime>& _last_transfer,
        const Nullable<std::string>& _auth_info_pw,
        const std::vector<DnsHostOutput>& _dns_host,
        const std::vector<std::string>& _tech_contacts,
        short _tech_check_level)
    :
        handle(_handle),
        roid(_roid),
        sponsoring_registrar_handle(_sponsoring_registrar_handle),
        creating_registrar_handle(_creating_registrar_handle),
        last_update_registrar_handle(_last_update_registrar_handle),
        localized_external_states(_localized_external_states),
        crdate(_crdate),
        last_update(_last_update),
        last_transfer(_last_transfer),
        auth_info_pw(_auth_info_pw),
        dns_host(_dns_host),
        tech_contacts(_tech_contacts),
        tech_check_level(_tech_check_level)
    { }
};

struct InfoNssetLocalizedResponse
{
    const LocalizedSuccessResponse ok_response;
    const InfoNssetLocalizedOutputData data;

    InfoNssetLocalizedResponse(
        const LocalizedSuccessResponse& _ok_response,
        const InfoNssetLocalizedOutputData& _data)
    :
        ok_response(_ok_response),
        data(_data)
    { }
};

InfoNssetLocalizedResponse info_nsset_localized(
        const std::string& _handle,
        unsigned long long _registrar_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle);

} // namespace Epp::Nsset
} // namespace Epp

#endif
