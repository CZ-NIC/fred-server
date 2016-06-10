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

#ifndef LOCALIZED_TRANSFER_H_572D2366689E3FF76A7E80D269879AD9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define LOCALIZED_TRANSFER_H_572D2366689E3FF76A7E80D269879AD9

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "util/optional_value.h"

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace KeySet {
namespace Localized {

LocalizedSuccessResponse transfer(
    const std::string& _keyset_handle,
    const std::string& _authinfopw,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const std::string &_client_transaction_handles_prefix_not_to_nofify);

}//namespace Epp::KeySet::Localized
}//namespace Epp::KeySet
}//namespace Epp

#endif//LOCALIZED_TRANSFER_H_572D2366689E3FF76A7E80D269879AD9
