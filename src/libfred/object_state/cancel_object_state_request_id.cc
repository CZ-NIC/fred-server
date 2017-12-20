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
 *  @file cancel_object_state_request_id.cc
 *  cancel object state request
 */

#include "src/libfred/object_state/cancel_object_state_request_id.hh"
#include "src/libfred/object_state/get_blocking_status_desc_list.hh"
#include "src/libfred/object_state/get_object_state_id_map.hh"
#include "src/libfred/object_state/lock_object_state_request_lock.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/db_settings.hh"
#include "src/util/optional_value.hh"
#include "src/util/db/nullable.hh"
#include "src/util/util.hh"
#include "src/libfred/object.hh"

#include <boost/algorithm/string.hpp>

namespace LibFred
{
    CancelObjectStateRequestId::CancelObjectStateRequestId(ObjectId _object_id,
        const StatusList &_status_list)
    :   object_id_(_object_id),
        status_list_(_status_list)
    {}

    void CancelObjectStateRequestId::exec(OperationContext &_ctx)
    {
        std::string object_state_names;

        for (StatusList::const_iterator pState = status_list_.begin();
             pState != status_list_.end(); ++pState) {
            object_state_names += (*pState) + " ";
        }

        _ctx.get_log().debug(std::string(
            "CancelObjectStateRequest::exec object id: ") + boost::lexical_cast< std::string >(object_id_)
            + " object state name: " + object_state_names);

        //get object type
        ObjectType object_type = 0;
        Database::query_param_list param(object_id_);
        {
            Database::Result object_type_result = _ctx.get_conn().exec_params(
                "SELECT type "
                "FROM object_registry "
                "WHERE id=$1::bigint", param);
            if (object_type_result.size() <= 0) {
                BOOST_THROW_EXCEPTION(Exception().set_object_id_not_found(object_id_));
            }
            const Database::Row &row = object_type_result[0];
            object_type = static_cast< ObjectType >(row[0]);
        }

        GetObjectStateIdMap get_object_state_id_map(status_list_, object_type);
        typedef GetObjectStateIdMap::StateIdMap StateIdMap;
        StateIdMap state_id_map;
        try {
            state_id_map = get_object_state_id_map.exec(_ctx);
            LockObjectStateRequestLock(object_id_).exec(_ctx);
        }
        catch (const GetObjectStateIdMap::Exception &ex) {
            if (ex.is_set_state_not_found()) {
                BOOST_THROW_EXCEPTION(Exception().set_state_not_found(ex.get_state_not_found()));
            }

            throw;
        }

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
        cmd << ") RETURNING id,state_id";
        enum
        {
            REQUEST_ID_IDX = 0,
            STATE_ID_IDX   = 1,
        };
        Database::Result cmd_result = _ctx.get_conn().exec_params(cmd.str(), param);

        std::set<unsigned long long> result_state_id;
        for(unsigned long long i = 0; i < cmd_result.size(); ++i){
            result_state_id.insert(cmd_result[i][1]);
        }

        std::set<unsigned long long> required_state_id;
        for (StateIdMap::const_iterator pStateId = state_id_map.begin();
             pStateId != state_id_map.end(); ++pStateId) {
            const ObjectStateId object_state_id = pStateId->second;
            required_state_id.insert(object_state_id);
        }

        if (cmd_result.size() >= state_id_map.size() && result_state_id == required_state_id) {
            std::string rid = "CancelObjectStateRequest::exec canceled request id:";
            for (Database::Result::Iterator pRow = cmd_result.begin(); pRow != cmd_result.end(); ++pRow) {
                rid += " " + std::string((*pRow)[REQUEST_ID_IDX]);
            }
            _ctx.get_log().debug(rid);
            return;
        }
        for (Database::Result::Iterator pRow = cmd_result.begin(); pRow != cmd_result.end(); ++pRow) {
            const ObjectStateId state_id = (*pRow)[STATE_ID_IDX];
            for (StateIdMap::iterator pStateId = state_id_map.begin();
                 pStateId != state_id_map.end(); ++pStateId) {
                if (pStateId->second == state_id) {
                    state_id_map.erase(pStateId);
                    break;
                }
            }
        }
        std::string errmsg;
        for (StateIdMap::const_iterator pStateId = state_id_map.begin();
             pStateId != state_id_map.end(); ++pStateId) {
            if (!errmsg.empty()) {
                errmsg += " ";
            }
            errmsg += pStateId->first;
        }
        errmsg += " for object_id " + boost::lexical_cast< std::string >(object_id_);
        BOOST_THROW_EXCEPTION(Exception().set_state_not_found(errmsg));
    }//CancelObjectStateRequestId::exec

} // namespace LibFred
