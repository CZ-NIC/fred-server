/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_CONTACT_LOCALIZED_H_3265A4F7B3D6435FBB17D88760C1AAED
#define UPDATE_CONTACT_LOCALIZED_H_3265A4F7B3D6435FBB17D88760C1AAED

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/contact/impl/change_contact.h"

#include "util/optional_value.h"

#include <string>

namespace Epp {

LocalizedSuccessResponse contact_update(
    const std::string &_contact_handle,
    const ContactChange &_data,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    const bool _epp_update_contact_enqueue_check,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string &_client_transaction_handles_prefix_not_to_notify);

}

#endif
