/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file clear_object_state_request_id.cc
 *  clear object state request
 */

#include "src/fredlib/object_state/clear_object_state_request_id.h"
#include "src/fredlib/object_state/get_blocking_status_desc_list.h"
#include "src/fredlib/object_state/get_object_state_id_map.h"
#include "lock_object_state_request_lock.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "src/fredlib/object.h"

#include <boost/algorithm/string.hpp>

namespace Fred
{
    ClearObjectStateRequestId::ClearObjectStateRequestId(ObjectId _object_id)
    :   object_id_(_object_id)
    {}

    ClearObjectStateRequestId::Requests ClearObjectStateRequestId::exec(OperationContext &_ctx)
    {
        LockObjectStateRequestLock(object_id_).exec(_ctx);

        enum ResultColumnIndex
        {
            REQUEST_ID_IDX = 0,
        };
        Database::Result cmd_result = _ctx.get_conn().exec_params(
            "UPDATE object_state_request "
            "SET canceled=CURRENT_TIMESTAMP "
            "WHERE canceled is NULL AND "
                  "object_id=$1::bigint AND "
                  "valid_from<=CURRENT_TIMESTAMP AND "
                  "(valid_to IS NULL OR "
                   "CURRENT_TIMESTAMP<valid_to) AND "
                  "(SELECT 0 "
                   "FROM enum_object_states eos "
                   "WHERE eos.id=state_id AND "
                         "eos.manual AND "
                         "eos.name LIKE 'server%' "
                   "LIMIT 1) IS NOT NULL "
            "RETURNING id", Database::query_param_list(object_id_));
        Requests result;
        if (0 < cmd_result.size()) {
            std::string rid = "ClearObjectStateRequest::exec canceled request id:";
            for (Database::Result::Iterator pRow = cmd_result.begin(); pRow != cmd_result.end(); ++pRow) {
                rid += " " + std::string((*pRow)[REQUEST_ID_IDX]);
                result.push_back(static_cast< Fred::ObjectId >((*pRow)[REQUEST_ID_IDX]));
            }
            _ctx.get_log().debug(rid);
        }
        return result;
    }//ClearObjectStateRequestId::exec

}//namespace Fred
