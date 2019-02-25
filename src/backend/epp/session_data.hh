/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef SESSION_DATA_HH_0C21D160C76F49568109EACA36F39D26
#define SESSION_DATA_HH_0C21D160C76F49568109EACA36F39D26

#include "src/backend/epp/session_lang.hh"

#include "util/optional_value.hh"

#include <string>

namespace Epp {

struct SessionData
{
    unsigned long long registrar_id;
    Epp::SessionLang::Enum lang;
    std::string server_transaction_handle;
    Optional<unsigned long long> logd_request_id;


    SessionData(
            unsigned long long _registrar_id,
            Epp::SessionLang::Enum _lang,
            const std::string& _server_transaction_handle,
            const boost::optional<unsigned long long>& _logd_request_id)
        : registrar_id(_registrar_id),
          lang(_lang),
          server_transaction_handle(_server_transaction_handle),
          logd_request_id(_logd_request_id
                                  ? Optional<unsigned long long>(*_logd_request_id)
                                  : Optional<unsigned long long>())
    {
    }


};

bool is_session_registrar_valid(const SessionData& _session_data);

} // namespace Epp

#endif
