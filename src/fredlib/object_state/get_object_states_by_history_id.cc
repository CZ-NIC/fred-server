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

#include "src/fredlib/object_state/get_object_states_by_history_id.hh"
#include "src/fredlib/db_settings.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Fred {

GetObjectStatesByHistoryId::GetObjectStatesByHistoryId(unsigned long long _history_id)
    : history_id_(_history_id)
{ }

GetObjectStatesByHistoryId::Result GetObjectStatesByHistoryId::exec(OperationContext& _ctx)
{
    const Database::Result dbres = _ctx.get_conn().exec_params(
            "WITH h AS ("
                "SELECT oh.id AS object_id,"
                       "h.valid_from AS valid_from,"
                       "COALESCE(h.valid_to,NOW()) AS valid_to "
                "FROM history h "
                "JOIN object_history oh ON oh.historyid=h.id "
                "WHERE h.id=$1::BIGINT) "
            "SELECT eos.id,eos.name,os.valid_from,os.valid_to,os.ohid_from,os.ohid_to,"
                   "eos.external,eos.manual,eos.importance,"
                   "(os.valid_from<=h.valid_from AND (h.valid_from<os.valid_to OR os.valid_to IS NULL)) AS at_valid_from,"
                   "(os.valid_from<h.valid_to AND (h.valid_to<=os.valid_to OR os.valid_to IS NULL)) AS before_valid_to "
            "FROM h "
            "JOIN object_state os ON os.object_id=h.object_id "
            "JOIN enum_object_states eos ON eos.id=os.state_id "
            "WHERE (os.valid_from<=h.valid_from AND (h.valid_from<os.valid_to OR os.valid_to IS NULL)) OR "
                  "(os.valid_from<h.valid_to AND (h.valid_to<=os.valid_to OR os.valid_to IS NULL)) "
            "ORDER BY eos.importance",
            Database::query_param_list(history_id_));
    Result result;
    for (unsigned long long idx = 0; idx < dbres.size() ; ++idx)
    {
        ObjectStateData flag;
        flag.state_id = static_cast<unsigned long long>(dbres[idx][0]);
        flag.state_name = static_cast<std::string>(dbres[idx][1]);
        flag.valid_from_time = boost::posix_time::time_from_string(static_cast<std::string>(dbres[idx][2]));
        flag.valid_to_time = dbres[idx][3].isnull()
                ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(
                        boost::posix_time::time_from_string(static_cast<std::string>(dbres[idx][3])));
        flag.valid_from_history_id = dbres[idx][4].isnull()
                ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(dbres[idx][4]));
        flag.valid_to_history_id = dbres[idx][5].isnull()
                ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(dbres[idx][5]));
        flag.is_external = static_cast<bool>(dbres[idx][6]);
        flag.is_manual = static_cast<bool>(dbres[idx][7]);
        flag.importance = static_cast<long>(dbres[idx][8]);
        const bool flag_presents_at_valid_from = static_cast<bool>(dbres[idx][9]);
        const bool flag_presents_before_valid_to = static_cast<bool>(dbres[idx][10]);
        if (flag_presents_at_valid_from)
        {
            result.object_state_at_begin.push_back(flag);
        }
        if (flag_presents_before_valid_to)
        {
            result.object_state_at_end.push_back(flag);
        }
    }
    return result;
}

}//namespace Fred
