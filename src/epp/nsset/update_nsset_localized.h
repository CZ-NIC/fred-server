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

#ifndef UPDATE_NSSET_LOCALIZED_H_EF8D30BE9BAF43968D5E49305821ECA2
#define UPDATE_NSSET_LOCALIZED_H_EF8D30BE9BAF43968D5E49305821ECA2

#include <string>
#include <boost/optional.hpp>

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/nsset/impl/nsset_dns_host_input.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"


namespace Epp {


struct NssetUpdateInputData {
    std::string handle;
    Optional<std::string> authinfo;
    std::vector<Epp::DNShostInput> dns_hosts_add;
    std::vector<Epp::DNShostInput> dns_hosts_rem;
    std::vector<std::string> tech_contacts_add;
    std::vector<std::string> tech_contacts_rem;
    boost::optional<short> tech_check_level;
    unsigned int config_nsset_min_hosts;
    unsigned int config_nsset_max_hosts;

    NssetUpdateInputData(
        const std::string& _handle,
        const Optional<std::string>& _authinfo,
        const std::vector<Epp::DNShostInput>& _dns_hosts_add,
        const std::vector<Epp::DNShostInput>& _dns_hosts_rem,
        const std::vector<std::string>& _tech_contacts_add,
        const std::vector<std::string>& _tech_contacts_rem,
        const boost::optional<short>& _tech_check_level,
        const unsigned int _config_nsset_min_hosts,
        const unsigned int _config_nsset_max_hosts
    ) :
        handle(_handle),
        authinfo(_authinfo),
        dns_hosts_add(_dns_hosts_add),
        dns_hosts_rem(_dns_hosts_rem),
        tech_contacts_add(_tech_contacts_add),
        tech_contacts_rem(_tech_contacts_rem),
        tech_check_level(_tech_check_level),
        config_nsset_min_hosts(_config_nsset_min_hosts),
        config_nsset_max_hosts(_config_nsset_max_hosts)
    { }
};



LocalizedSuccessResponse nsset_update(
    const NssetUpdateInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    bool _epp_update_nsset_enqueue_check,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string& _client_transaction_handles_prefix_not_to_nofify
);

}

#endif
