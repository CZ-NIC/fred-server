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
 *  @file create_object_state_request.cc
 *  create object state request
 */

#include "fredlib/domain/create_object_state_request.h"
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

#define MY_EXCEPTION_CLASS(DATA) CreateObjectStateRequestException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA) CreateObjectStateRequestError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

namespace Fred
{
    CreateObjectStateRequest::CreateObjectStateRequest(const std::string &_object_handle,
        ObjectType _object_type,
        const StatusList &_status_list)
    :   object_handle_(_object_handle),
        object_type_(_object_type),
        status_list_(_status_list)
    {}

    CreateObjectStateRequest::CreateObjectStateRequest(const std::string &_object_handle,
        ObjectType _object_type,
        const StatusList &_status_list,
        const Optional< Time > &_valid_from,
        const Optional< Time > &_valid_to)
    :   object_handle_(_object_handle),
        object_type_(_object_type),
        status_list_(_status_list),
        valid_from_(_valid_from),
        valid_to_(_valid_to)
    {}

    CreateObjectStateRequest& CreateObjectStateRequest::set_valid_from(const Time &_valid_from)
    {
        valid_from_ = _valid_from;
        return *this;
    }

    CreateObjectStateRequest& CreateObjectStateRequest::set_valid_to(const Time &_valid_to)
    {
        valid_to_ = _valid_to;
        return *this;
    }

    ObjectId CreateObjectStateRequest::exec(OperationContext &_ctx)
    {
        std::string object_state_names;

        for (StatusList::const_iterator pState = status_list_.begin();
             pState != status_list_.end(); ++pState) {
            object_state_names += (*pState) + " ";
        }

        _ctx.get_log().debug(std::string(
            "CreateObjectStateRequest::exec object name: ") + object_handle_
            + " object type: " + boost::lexical_cast< std::string >(object_type_)
            + " object state name: " + object_state_names
            + " valid from: " + boost::posix_time::to_iso_string(valid_from_)
            + " valid to: " + boost::posix_time::to_iso_string(valid_to_));

        //check time
        const boost::posix_time::ptime new_valid_from
            = valid_from_.isset() ? valid_from_.get_value()
                : boost::posix_time::second_clock::universal_time();

        const boost::posix_time::ptime new_valid_to
            = valid_to_.isset() ? valid_to_.get_value()
                : boost::posix_time::pos_infin;

        if (new_valid_to < new_valid_from) {
            std::string errmsg("|| out of turn:valid_from-to: ");
            errmsg += boost::posix_time::to_iso_string(new_valid_from) + " - " +
                      boost::posix_time::to_iso_string(new_valid_to);
            errmsg += " |";
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
        }

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

        for (StateIdMap::const_iterator pStateId = state_id_map.begin();
             pStateId != state_id_map.end(); ++pStateId) {

            const ObjectStateId object_state_id = pStateId->second;

            //get existing state requests for object and state
            //assuming requests for different states of the same object may overlay
            Database::Result requests_result = _ctx.get_conn().exec_params(
                "SELECT valid_from,valid_to,canceled "
                "FROM object_state_request "
                "WHERE object_id=$1::bigint AND state_id=$2::bigint",
                Database::query_param_list(object_id)(object_state_id));

            for (std::size_t idx = 0; idx < requests_result.size(); ++idx) {
                const boost::posix_time::ptime obj_valid_from = requests_result[idx][0];

                boost::posix_time::ptime obj_valid_to = requests_result[idx][1];

                //if obj_canceled is not null
                if (!requests_result[idx][2].isnull())
                {
                    boost::posix_time::ptime obj_canceled = requests_result[idx][2];

                    if (obj_canceled < obj_valid_to) {
                        obj_valid_to = obj_canceled;
                    }
                }//if obj_canceled is not null

                if (obj_valid_to < obj_valid_from ) {
                    std::string errmsg("|| out of turn:valid_from-to: ");
                    errmsg += boost::posix_time::to_iso_string(obj_valid_from) + " - " +
                              boost::posix_time::to_iso_string(obj_valid_to);
                    errmsg += " |";
                    throw MY_EXCEPTION_CLASS(errmsg.c_str());
                }

                if (obj_valid_to.is_special()) {
                    obj_valid_to = boost::posix_time::pos_infin;
                }

                _ctx.get_log().debug(std::string(
                    "CreateObjectStateRequest::exec new_valid_from: ")
                    + boost::posix_time::to_iso_extended_string(new_valid_from)
                    + " new_valid_to: " + boost::posix_time::to_iso_extended_string(new_valid_to)
                    + " obj_valid_from: " + boost::posix_time::to_iso_extended_string(obj_valid_from)
                    + " obj_valid_to: " + boost::posix_time::to_iso_extended_string(obj_valid_to)
                );

                //check overlay
                if (((obj_valid_from <= new_valid_from) && (new_valid_from < obj_valid_to))
                  || ((obj_valid_from < new_valid_to) && (new_valid_to <= obj_valid_to))) {
                    std::string errmsg("|| overlayed validity time intervals:object: ");
                    errmsg += "<" + boost::posix_time::to_iso_string(obj_valid_from) + ", " +
                              boost::posix_time::to_iso_string(obj_valid_to) + ") - "
                              "<" + boost::posix_time::to_iso_string(new_valid_from) + ", " +
                              boost::posix_time::to_iso_string(new_valid_to) + ")";
                    errmsg += " |";
                    throw MY_EXCEPTION_CLASS(errmsg.c_str());
                }
            }//for check with existing object state requests

            _ctx.get_conn().exec_params(
                "INSERT INTO object_state_request "
                "(object_id,state_id,crdate,valid_from,valid_to) VALUES "
                "($1::bigint,$2::bigint,"
                 "CURRENT_TIMESTAMP,$3::timestamp,"
                 "$4::timestamp)",
                Database::query_param_list
                    (object_id)(object_state_id)
                    (new_valid_from)(new_valid_to.is_special()
                            ? Database::QPNull
                            : Database::QueryParam(new_valid_to))
            );

        }//for object_state
        return object_id;
    }//CreateObjectStateRequest::exec

    PerformObjectStateRequest::PerformObjectStateRequest()
    {}

    PerformObjectStateRequest::PerformObjectStateRequest(const Optional< ObjectId > &_object_id)
    :   object_id_(_object_id)
    {}

    PerformObjectStateRequest& PerformObjectStateRequest::set_object_id(ObjectId _object_id)
    {
        object_id_ = _object_id;
        return *this;
    }

    void PerformObjectStateRequest::exec(OperationContext &_ctx)
    {
        _ctx.get_conn().exec_params(
            "SELECT update_object_states($1::integer)",
            Database::query_param_list
                (object_id_));
    }

    LockObjectStateRequestLock::LockObjectStateRequestLock(ObjectStateId _state_id, ObjectId _object_id)
    :   state_id_(_state_id),
        object_id_(_object_id)
    {}

    void LockObjectStateRequestLock::exec(OperationContext &_ctx)
    {
        {//insert separately
            typedef std::auto_ptr< Database::StandaloneConnection > StandaloneConnectionPtr;
            Database::StandaloneManager sm = Database::StandaloneManager(
                new Database::StandaloneConnectionFactory(/*Database::Manager::getConnectionString()*/"host=/data/fred/fred/scripts/root/nofred/pg_sockets port=22345 dbname=fred user=fred connect_timeout=2"));
            StandaloneConnectionPtr conn_standalone(sm.acquire());
            conn_standalone->exec_params(
                "INSERT INTO object_state_request_lock (id,state_id,object_id) "
                "VALUES (DEFAULT, $1::bigint, $2::bigint)",
                Database::query_param_list(state_id_)(object_id_));
        }

        _ctx.get_conn().exec_params("SELECT lock_object_state_request_lock($1::bigint, $2::bigint)",
            Database::query_param_list(state_id_)(object_id_));
    }

    LockMultipleObjectStateRequestLock::LockMultipleObjectStateRequestLock(
        const MultipleObjectStateId &_state_id, ObjectId _object_id)
    :   state_id_(_state_id),
        object_id_(_object_id)
    {}

    void LockMultipleObjectStateRequestLock::exec(OperationContext &_ctx)
    {
        {//insert separately
            typedef std::auto_ptr< Database::StandaloneConnection > StandaloneConnectionPtr;
            Database::StandaloneManager sm = Database::StandaloneManager(
                new Database::StandaloneConnectionFactory(/*Database::Manager::getConnectionString()*/"host=/data/fred/fred/scripts/root/nofred/pg_sockets port=22345 dbname=fred user=fred connect_timeout=2"));
            StandaloneConnectionPtr conn_standalone(sm.acquire());
            Database::query_param_list param(object_id_);
            std::ostringstream cmd;
            cmd << "INSERT INTO object_state_request_lock (object_id,state_id) VALUES ";
            for (MultipleObjectStateId::const_iterator pId = state_id_.begin(); pId != state_id_.end(); ++pId) {
                if (1 < param.size()) {
                    cmd << ",";
                }
                param(*pId);
                cmd << "($1::bigint,$" << param.size() << "::bigint)";
            }
            conn_standalone->exec_params(cmd.str(), param);
        }

        for (MultipleObjectStateId::const_iterator pStateId = state_id_.begin(); pStateId != state_id_.end(); ++pStateId) {
            _ctx.get_conn().exec_params("SELECT lock_object_state_request_lock($1::bigint,$2::bigint)",
                Database::query_param_list(*pStateId)(object_id_));
        }
    }

    GetObjectId::GetObjectId(const std::string &_object_handle, ObjectType _object_type)
    :   object_handle_(_object_handle),
        object_type_(_object_type)
    {}

    ObjectId GetObjectId::exec(OperationContext &_ctx)
    {
        Database::Result obj_id_res = _ctx.get_conn().exec_params(
            "SELECT id FROM object_registry "
            "WHERE type=$1::integer AND name=$2::text AND erdate IS NULL",
            Database::query_param_list
                (object_type_)(object_handle_));

        if (obj_id_res.size() != 1) {
            std::string errmsg("|| not found:handle: ");
            errmsg += boost::replace_all_copy(object_handle_,"|", "[pipe]");//quote pipes
            errmsg += " of type " + boost::lexical_cast< std::string >(object_type_);
            errmsg += " |";
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
        }

        return obj_id_res[0][0];
    }

}//namespace Fred
