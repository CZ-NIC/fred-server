/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/common_diff.hh"
#include "src/deprecated/libfred/db_settings.hh"

namespace LibFred {


void _diff_object(ChangesMap &_changes,
                  const LibFred::Object *_prev,
                  const LibFred::Object *_act)
{
    compare_and_fill(_changes, "object.authinfo", _prev->getAuthPw(), _act->getAuthPw());
}



std::pair<unsigned long long, unsigned long long>
    get_last_history(const unsigned long long &_id, const unsigned int &_type)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result history_data = conn.exec_params(
            "SELECT h.id AS prev, h.next AS act FROM history h"
            " WHERE h.next = (SELECT historyid FROM object_registry"
            " WHERE id = $1::integer AND type = $2::integer)",
            Database::query_param_list(_id)(_type));

    if (history_data.size() != 1) {
        throw std::runtime_error("no history found");
    }

    return std::make_pair(static_cast<unsigned long long>(history_data[0][0]),
                          static_cast<unsigned long long>(history_data[0][1]));
}


}

