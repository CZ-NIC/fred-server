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
 *  @file domain_update.h
 *  <++>
 */

#ifndef DOMAIN_UPDATE_H_E54DDFE4EB2C45D6972306DF00AD05C7
#define DOMAIN_UPDATE_H_E54DDFE4EB2C45D6972306DF00AD05C7

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

#include <string>

namespace Epp {

namespace Domain {

LocalizedSuccessResponse domain_update(
    const std::string& _domain_fqdn,
    const Optional<std::string>& _registrant_chg,
    const Optional<std::string>& _auth_info_pw_chg,
    const Optional<Nullable<std::string> >& _nsset_chg,
    const Optional<Nullable<std::string> >& _keyset_chg,
    const std::vector<std::string>& _admin_contacts_add,
    const std::vector<std::string>& _admin_contacts_rem,
    const std::vector<std::string>& _tmpcontacts_rem,
    const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    const bool _epp_update_domain_enqueue_check,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string& _client_transaction_handles_prefix_not_to_notify,
    bool _rifd_epp_update_domain_keyset_clear);
}

}

#endif