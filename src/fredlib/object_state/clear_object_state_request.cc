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
 *  @file clear_object_state_request.cc
 *  clear object state request
 */

#include "src/fredlib/object_state/clear_object_state_request.h"
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
    ClearObjectStateRequest::ClearObjectStateRequest(const std::string &_object_handle,
        ObjectType _object_type)
    :   object_handle_(_object_handle),
        object_type_(_object_type)
    {}

    ObjectId ClearObjectStateRequest::exec(OperationContext &_ctx)
    {
        //get object
        const ObjectId object_id = GetObjectId(object_handle_, object_type_).exec(_ctx);

        LockObjectStateRequestLock(object_id).exec(_ctx);

        Database::query_param_list param(object_id);
        std::ostringstream cmd;
        cmd <<
            "UPDATE object_state_request "
            "SET canceled=CURRENT_TIMESTAMP "
            "WHERE canceled is NULL AND "
                  "object_id=$1::integer AND "
                  "valid_from<=CURRENT_TIMESTAMP AND "
                  "(valid_to IS NULL OR "
                   "CURRENT_TIMESTAMP<valid_to) "
            "RETURNING id";
        enum ResultColumnIndex
        {
            REQUEST_ID_IDX = 0,
        };
        Database::Result cmd_result = _ctx.get_conn().exec_params(cmd.str(), param);
        if (0 < cmd_result.size()) {
            std::string rid = "ClearObjectStateRequest::exec canceled request id:";
            for (Database::Result::Iterator pRow = cmd_result.begin(); pRow != cmd_result.end(); ++pRow) {
                rid += " " + std::string((*pRow)[REQUEST_ID_IDX]);
            }
            _ctx.get_log().debug(rid);
        }
        return object_id;
    }//ClearObjectStateRequest::exec

}//namespace Fred
