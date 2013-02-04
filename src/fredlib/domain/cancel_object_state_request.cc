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
 *  @file cancel_object_state_request.cc
 *  cancel object state request
 */

#include "fredlib/domain/cancel_object_state_request.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
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

#define MY_EXCEPTION_CLASS(DATA) CancelObjectStateRequestException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA) CancelObjectStateRequestError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

namespace Fred
{
    CancelObjectStateRequest::CancelObjectStateRequest(const std::string &_object_handle,
        ObjectType _object_type,
        const StatusList &_status_list)
    :   object_handle_(_object_handle),
        object_type_(_object_type),
        status_list_(_status_list)
    {}

    ObjectId CancelObjectStateRequest::exec(OperationContext &_ctx)
    {
        std::string object_state_names;

        for (StatusList::const_iterator pState = status_list_.begin();
             pState != status_list_.end(); ++pState) {
            object_state_names += (*pState) + " ";
        }

        _ctx.get_log().debug(std::string(
            "CancelObjectStateRequest::exec object name: ") + object_handle_
            + " object type: " + boost::lexical_cast< std::string >(object_type_)
            + " object state name: " + object_state_names);

        //get object
        const ObjectId object_id = GetObjectId(object_handle_, object_type_).exec(_ctx);

        GetObjectStateIdMap get_object_state_id_map(status_list_, object_type_);
        typedef GetObjectStateIdMap::StateIdMap StateIdMap;
        const StateIdMap &state_id_map = get_object_state_id_map.exec(_ctx);
        {
            MultipleObjectStateId state_id;
            for (StateIdMap::const_iterator pStateId = state_id_map.begin();
                 pStateId != state_id_map.end(); ++pStateId) {
                state_id.push_back(pStateId->second);
            }
            
            LockMultipleObjectStateRequestLock(state_id, object_id).exec(_ctx);
        }

        Database::query_param_list param(object_id);
        std::ostringstream cmd;
        cmd <<
            "UPDATE object_state_request "
            "SET canceled=CURRENT_TIMESTAMP "
            "WHERE canceled is NULL AND "
                  "object_id=$1::integer AND "
                  "valid_from<=CURRENT_TIMESTAMP AND "
                  "(valid_to IS NULL OR "
                   "CURRENT_TIMESTAMP<valid_to) AND "
                  "state_id IN (";
        for (StateIdMap::const_iterator pStateId = state_id_map.begin();
             pStateId != state_id_map.end(); ++pStateId) {
            const ObjectStateId object_state_id = pStateId->second;
            if (pStateId != state_id_map.begin()) {
                cmd << ",";
            }
            param(object_state_id);
            cmd << "$" << param.size() << "::integer";
        }//for object_state
        cmd << ") RETURNING id";
        Database::Result cmd_result = _ctx.get_conn().exec_params(cmd.str(), param);
        if (cmd_result.size() == state_id_map.size()) {
            std::string rid = "CancelObjectStateRequest::exec canceled request id:";
            for (Database::Result::Iterator pRow = cmd_result.begin(); pRow != cmd_result.end(); ++pRow) {
                rid += " " + std::string((*pRow)[0]);
            }
            _ctx.get_log().debug(rid);
            return object_id;
        }
        throw MY_EXCEPTION_CLASS("xxx");
    }//CancelObjectStateRequest::exec

}//namespace Fred
