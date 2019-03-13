/*
 * Copyright (C) 2013  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/registrable_object/contact/util.hh"

#include "libfred/opcontext.hh"

std::pair<std::string, unsigned long long> LibFred::ContactUtil::contact_hid_to_handle_id_pair(unsigned long long hid) {
    LibFred::OperationContextCreator ctx;

    Database::Result contact_data = ctx.get_conn().exec_params(
       "SELECT "
       "        o_r.id      AS id_, "
       "        o_r.name    AS handle_ "
       "    FROM contact_history AS c_h "
       "        JOIN object_registry AS o_r USING (id) "
       "    WHERE "
       "        c_h.historyid = $1::integer ",
       Database::query_param_list(hid) );

   if (contact_data.size() != 1) {
       throw ExceptionUnknownContactHistoryId();
   }

   return std::make_pair(
       static_cast<std::string>((*contact_data.begin())["handle_"]),
       static_cast<unsigned long long>((*contact_data.begin())["id_"])
   );
}
