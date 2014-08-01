/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  get states of given object
 */

#include <vector>
#include <string>

#include "src/fredlib/opcontext.h"
#include "get_object_states.h"

namespace Fred
{

    GetObjectStates::GetObjectStates(unsigned long long object_id)
    : object_id_(object_id)
    {}

    std::vector<ObjectStateData> GetObjectStates::exec(OperationContext& ctx)
    {
        Database::Result domain_states_result = ctx.get_conn().exec_params(
        "SELECT eos.id, eos.name, os.valid_from, os.valid_to , os.ohid_from, os.ohid_to"
        " , eos.external, eos.manual, eos.importance "
        " FROM object_state os "
            " JOIN enum_object_states eos ON eos.id = os.state_id "
            " WHERE os.object_id = $1::bigint "
                " AND os.valid_from <= CURRENT_TIMESTAMP "
                " AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) "
        " ORDER BY eos.importance "
        , Database::query_param_list(object_id_)
        );

        std::vector<ObjectStateData> result;
        result.reserve(domain_states_result.size());
        for(unsigned long long i = 0 ; i < domain_states_result.size() ; ++i)
        {
            ObjectStateData osd;
            osd.state_id = static_cast<unsigned long long>(domain_states_result[i][0]);
            osd.state_name = static_cast<std::string>(domain_states_result[i][1]);
            osd.valid_from_time = boost::posix_time::time_from_string(static_cast<std::string>(domain_states_result[i][2]));
            osd.valid_to_time = domain_states_result[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(
                static_cast<std::string>(domain_states_result[i][3])));

            osd.valid_from_history_id = domain_states_result[i][4].isnull() ? Nullable<unsigned long long>()
                    : Nullable<unsigned long long>(static_cast<unsigned long long>(domain_states_result[i][4]));
            osd.valid_to_history_id = domain_states_result[i][5].isnull() ? Nullable<unsigned long long>()
                    : Nullable<unsigned long long>(static_cast<unsigned long long>(domain_states_result[i][5]));

            osd.is_external = static_cast<bool>(domain_states_result[i][6]);
            osd.is_manual = static_cast<bool>(domain_states_result[i][7]);
            osd.importance = static_cast<long>(domain_states_result[i][8]);

            result.push_back(osd);
        }//for i
        return result;
    }

}//namespace Fred
