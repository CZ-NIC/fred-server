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

#include "fredlib/domain/clear_object_state_request_id.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/domain/get_object_state_id_map.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>

#ifndef __ASSERT_FUNCTION
#define __ASSERT_FUNCTION __PRETTY_FUNCTION__
#endif

#define MY_EXCEPTION_CLASS(DATA) ClearObjectStateRequestIdException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA) ClearObjectStateRequestIdError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

namespace Fred
{
    ClearObjectStateRequestId::ClearObjectStateRequestId(ObjectId _object_id)
    :   object_id_(_object_id)
    {}

    void ClearObjectStateRequestId::exec(OperationContext &_ctx)
    {
        //get object type
        ObjectType object_type = 0;
        Database::query_param_list param(object_id_);
        {
            Database::Result object_type_result = _ctx.get_conn().exec_params(
                "SELECT type "
                "FROM object_registry "
                "WHERE id=$1::bigint", param);
            if (object_type_result.size() <= 0) {
                std::string errmsg("|| not found:object_id: ");
                errmsg += boost::lexical_cast< std::string >(object_id_);
                errmsg += " |";
                throw MY_EXCEPTION_CLASS(errmsg.c_str());
            }
            const Database::Row &row = object_type_result[0];
            object_type = static_cast< ObjectType >(row[0]);
        }

        Database::Result status_result = _ctx.get_conn().exec_params(
            "SELECT id "
            "FROM enum_object_states "
            "WHERE $1::integer=ANY(types)",
            Database::query_param_list(object_type));
        MultipleObjectStateId status_all;
        status_all.reserve(status_result.size());
        for (Database::Result::Iterator pRow = status_result.begin(); pRow != status_result.end(); ++pRow) {
            status_all.push_back((*pRow)[0]);
        }
        LockMultipleObjectStateRequestLock(status_all, object_id_).exec(_ctx);

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
    }//ClearObjectStateRequestId::exec

}//namespace Fred
