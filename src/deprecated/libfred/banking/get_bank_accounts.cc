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
#include "src/deprecated/libfred/banking/get_bank_accounts.hh"
#include "src/deprecated/libfred/db_settings.hh"

#include "libfred/db_settings.hh"

#include <utility>

namespace LibFred {
namespace Banking {

EnumList getBankAccounts()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result dbresult =
            // clang-format off
            conn.exec("SELECT id, "
                             "account_name || ' ' || account_number || '/' || bank_code "
                        "FROM bank_account "
                       "ORDER BY id");
            // clang-format on
    EnumList el;
    for (std::size_t row_index = 0; row_index < dbresult.size(); ++row_index)
    {
        EnumListItem eli;
        eli.id = dbresult[row_index][0];
        eli.name = static_cast<std::string>(dbresult[row_index][1]);
        el.push_back(std::move(eli));
    }
    return el;
}

} // namespace LibFred::Banking
} // namespace LibFred
