/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef SESSION_DATA_H_983E625F4846434DBC860960DE80C9E9
#define SESSION_DATA_H_983E625F4846434DBC860960DE80C9E9

#include "src/epp/session_lang.h"

#include <string>

namespace Epp {

struct SessionData {
    unsigned long long registrar_id;
    Epp::SessionLang::Enum lang;
    const std::string server_transaction_handle;

    SessionData(
            unsigned long long _registrar_id,
            Epp::SessionLang::Enum _lang,
            const std::string& _server_transaction_handle)
        : registrar_id(_registrar_id),
          lang(_lang),
          server_transaction_handle(_server_transaction_handle)
    { }
};

} // namespace Epp

#endif
