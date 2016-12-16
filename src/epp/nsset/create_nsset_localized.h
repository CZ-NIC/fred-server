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

#ifndef CREATE_NSSET_LOCALIZED_H_E012D98CCFF7446FB305B72D2F4B8417
#define CREATE_NSSET_LOCALIZED_H_E012D98CCFF7446FB305B72D2F4B8417

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <boost/cast.hpp>

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/nsset/impl/dns_host_input.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Epp {
namespace Nsset {

struct CreateNssetInputData
{
    std::string handle;
    boost::optional<std::string> authinfo;
    std::vector<DnsHostInput> dns_hosts;
    std::vector<std::string> tech_contacts;
    boost::optional<short> input_tech_check_level;
    unsigned int config_default_nsset_tech_check_level;
    unsigned int config_nsset_min_hosts;
    unsigned int config_nsset_max_hosts;

    CreateNssetInputData(
        const std::string & _handle,
        const boost::optional<std::string>&_authinfo,
        const std::vector<DnsHostInput>&_dns_hosts,
        const std::vector<std::string>&_tech_contacts,
        const boost::optional<short>&_input_tech_check_level,
        const unsigned int _config_default_nsset_tech_check_level)
        const unsigned int _config_nsset_min_hosts,
        const unsigned int _config_nsset_max_hosts)
    :   handle(_handle),
        authinfo(_authinfo),
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
            ? *input_tech_check_level
            : boost::numeric_cast<short>(config_default_nsset_tech_check_level);
    }
};

struct CreateNssetLocalizedResponse
{
    const LocalizedSuccessResponse ok_response;
    const boost::posix_time::ptime crdate;

    CreateNssetLocalizedResponse(
        const LocalizedSuccessResponse& _ok_response,
        const boost::posix_time::ptime& _crdate)
    :   ok_response(_ok_response),
        crdate(_crdate)
    { }
};

CreateNssetLocalizedResponse create_nsset_localized(
        const CreateNssetInputData& _data,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix);

} // namespace Epp::Nsset
} // namespace Epp

#endif
