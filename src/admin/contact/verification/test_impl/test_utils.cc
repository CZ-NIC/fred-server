/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

#include "admin/contact/verification/test_impl/test_utils.h"
#include "fredlib/contact/info_contact_history.h"
#include "fredlib/opcontext.h"

namespace Admin
{
namespace Utils
{
    /* TODO this is only temporary hack before new version of InfoContactHistory is available
     * see ticket #9544
     */
    Fred::InfoContactData get_contact_data(long _history_id) {
        Fred::OperationContext ctx;

        // get roid, registrar
        Database::Result res = ctx.get_conn().exec_params(
            "SELECT o_r.roid AS roid_, reg.handle AS reg_handle_ "
            "   FROM contact_history AS c_h "
            "       JOIN object_registry AS o_r USING(id) "
            "       JOIN registrar AS reg ON o_r.crid = reg.id "
            "   WHERE c_h.historyid = $1::int "
            "   FOR SHARE; ",
            Database::query_param_list(_history_id) );

        if(res.size() != 1) {
            throw Fred::InternalError("no data found");
        }

        std::vector<Fred::InfoContactHistoryOutput> history_data (
            Fred::InfoContactHistory(
                static_cast<std::string>(res[0]["roid_"]),
                static_cast<std::string>(res[0]["reg_handle_"])
            ).exec(ctx)
        );

        // get right historical state
        for(std::vector<Fred::InfoContactHistoryOutput>::iterator it = history_data.begin();
            it != history_data.end();
            ++it
        ) {
            if(it->info_contact_data.historyid == _history_id) {
                return it->info_contact_data;
            }
        }

        throw Fred::InternalError("no data found");
    }
}
}
