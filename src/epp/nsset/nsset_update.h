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

#ifndef EPP_NSSET_UPDATE_H_355cb09807ee4a0c89f950e8121a512a
#define EPP_NSSET_UPDATE_H_355cb09807ee4a0c89f950e8121a512a

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/nsset/nsset_dns_host_data.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {

struct NssetUpdateInputData {
    std::string handle;
    Optional<std::string> authinfo;
    std::vector<Epp::DNShostData> dns_hosts_add;
    std::vector<Epp::DNShostData> dns_hosts_rem;
    std::vector<std::string> tech_contacts_add;
    std::vector<std::string> tech_contacts_rem;
    Optional<short> tech_check_level;

    NssetUpdateInputData(
        const std::string&            _handle,
        const Optional<std::string>&  _authinfo,
        const std::vector<Epp::DNShostData>& _dns_hosts_add,
        const std::vector<Epp::DNShostData>& _dns_hosts_rem,
        const std::vector<std::string>& _tech_contacts_add,
        const std::vector<std::string>& _tech_contacts_rem,
        const Optional<short>& _tech_check_level
    ) :
        handle(_handle),
        authinfo(_authinfo),
        dns_hosts_add(_dns_hosts_add),
        dns_hosts_rem(_dns_hosts_rem),
        tech_contacts_add(_tech_contacts_add),
        tech_contacts_rem(_tech_contacts_rem),
        tech_check_level(_tech_check_level)
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
    const std::string& _client_transaction_handles_prefix_not_to_nofify
);

}

#endif
