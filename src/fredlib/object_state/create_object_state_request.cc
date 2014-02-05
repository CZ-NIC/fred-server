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

#include "src/fredlib/object_state/create_object_state_request.h"
#include "src/fredlib/object_state/get_blocking_status_desc_list.h"
#include "src/fredlib/object_state/get_object_state_id_map.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "src/fredlib/object.h"

#include <boost/algorithm/string.hpp>

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
            + " valid from: " + boost::posix_time::to_iso_string(valid_from_.get_value_or_default())
            + " valid to: " + boost::posix_time::to_iso_string(valid_to_.get_value_or_default()));

        //check time
        if (valid_to_.isset()) {
            if (valid_from_.isset()) { // <from,to)
                if (valid_to_.get_value() < valid_from_.get_value()) {
                    std::string errmsg("valid from-to <");
                    errmsg += boost::posix_time::to_iso_string(valid_from_.get_value()) + ", " +
                              boost::posix_time::to_iso_string(valid_to_.get_value()) + ")";
                    BOOST_THROW_EXCEPTION(Exception().set_out_of_turn(errmsg));
                }
            }
            else { // <now,to)
                Database::Result out_of_turn_result = _ctx.get_conn().exec_params(
                        "SELECT $1<CURRENT_TIMESTAMP",
                        Database::query_param_list(valid_to_.get_value()));
                if (bool(out_of_turn_result[0][0])) {
                    std::string errmsg("valid from-to <CURRENT_TIMESTAMP, ");
                    errmsg += boost::posix_time::to_iso_string(valid_to_.get_value()) + ")";
                    BOOST_THROW_EXCEPTION(Exception().set_out_of_turn(errmsg));
                }
            }
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
                state_id.insert(pStateId->second);
            }
            
            LockMultipleObjectStateRequestLock(state_id, object_id).exec(_ctx);
        }

        std::string object_state_id_set;
        for (StateIdMap::const_iterator pStateId = state_id_map.begin();
             pStateId != state_id_map.end(); ++pStateId) {
            const ObjectStateId object_state_id = pStateId->second;
            if (object_state_id_set.empty()) {
                object_state_id_set = "(";
            }
            else {
                object_state_id_set += ",";
            }
            object_state_id_set += boost::lexical_cast< std::string >(object_state_id);
        }
        object_state_id_set += ")";

        std::string new_valid_column;
        Database::query_param_list param(object_id);
        if (valid_from_.isset()) {
            if (valid_to_.isset()) { // <from,to)
                new_valid_column = "$2::timestamp AS new_valid_from,$3::timestamp AS new_valid_to";
                param(valid_from_.get_value())(valid_to_.get_value());
            }
            else { // <from,oo)
                new_valid_column = "$2::timestamp AS new_valid_from,NULL::timestamp AS new_valid_to";
                param(valid_from_.get_value());
            }
        }
        else if (valid_to_.isset()) { // <now,to)
            new_valid_column = "CURRENT_TIMESTAMP::timestamp AS new_valid_from,$2::timestamp AS new_valid_to";
            param(valid_to_.get_value());
        }
        else { // <now,oo)
            new_valid_column = "CURRENT_TIMESTAMP::timestamp AS new_valid_from,NULL::timestamp AS new_valid_to";
        }
        std::string sub_query = "SELECT valid_from AS obj_valid_from,"
                                       "LEAST(canceled,valid_to) AS obj_valid_to," +
                                       new_valid_column + " "
                                "FROM object_state_request "
                                "WHERE object_id=$1::bigint AND state_id IN " + object_state_id_set;
        Database::Result invalid_state_result = _ctx.get_conn().exec_params(
            "SELECT obj_valid_from,obj_valid_to,new_valid_from,new_valid_to "
            "FROM (" + sub_query + ") AS obj_state "
            "WHERE obj_valid_to<obj_valid_from OR "
                  "(obj_valid_from<=new_valid_from AND (new_valid_from<obj_valid_to OR obj_valid_to IS NULL)) OR "
                  "((obj_valid_from<new_valid_to OR new_valid_to IS NULL) AND (new_valid_to<=obj_valid_to OR obj_valid_to IS NULL)) "
            "LIMIT 1", param);

        if (0 < invalid_state_result.size()) {
            const Database::Row &row = invalid_state_result[0];
            const boost::posix_time::ptime obj_valid_from = static_cast< const boost::posix_time::ptime& >(row[0]);
            const boost::posix_time::ptime obj_valid_to = row[1].isnull()
                                                              ? boost::posix_time::ptime(boost::posix_time::pos_infin)
                                                              : static_cast< const boost::posix_time::ptime& >(row[1]);
            if (obj_valid_to < obj_valid_from ) {
                std::string errmsg("valid from-to <");
                errmsg += boost::posix_time::to_iso_string(obj_valid_from) + ", " +
                          boost::posix_time::to_iso_string(obj_valid_to) + ")";
                BOOST_THROW_EXCEPTION(Exception().set_out_of_turn(errmsg));
            }
            const boost::posix_time::ptime new_valid_from = static_cast< const boost::posix_time::ptime& >(row[2]);
            const boost::posix_time::ptime new_valid_to = row[3].isnull()
                                                            ? boost::posix_time::ptime(boost::posix_time::pos_infin)
                                                            : static_cast< const boost::posix_time::ptime& >(row[3]);
            std::string errmsg("object:");
            errmsg += object_handle_ + " "
                      "<" + boost::posix_time::to_iso_string(obj_valid_from) + ", " +
                      boost::posix_time::to_iso_string(obj_valid_to) + ") - "
                      "<" + boost::posix_time::to_iso_string(new_valid_from) + ", " +
                      boost::posix_time::to_iso_string(new_valid_to) + ")";
            BOOST_THROW_EXCEPTION(Exception().set_overlayed_time_intervals(errmsg));
        }

        param.clear();
        param(object_id) // $1
             (valid_to_.isset() // $2
                             ? Database::QueryParam(valid_to_.get_value())
                             : Database::QPNull);
        std::ostringstream cmd;
        cmd << "INSERT INTO object_state_request "
                       "(object_id,"
                        "state_id,"
                        "crdate,"
                        "valid_from,"
                        "valid_to) VALUES ";
        if (valid_from_.isset()) {
            param(valid_from_.get_value()); // $3
            for (StateIdMap::const_iterator pStateId = state_id_map.begin();
                 pStateId != state_id_map.end(); ++pStateId) {
                if (pStateId != state_id_map.begin()) {
                    cmd << ",";
                }
                param(pStateId->second);
                cmd << "($1::bigint,"
                        "$" << param.size() << "::bigint,"
                        "CURRENT_TIMESTAMP,"
                        "$3::timestamp,"
                        "$2::timestamp)";
            }
        }
        else {
            for (StateIdMap::const_iterator pStateId = state_id_map.begin();
                 pStateId != state_id_map.end(); ++pStateId) {
                if (pStateId != state_id_map.begin()) {
                    cmd << ",";
                }
                param(pStateId->second);
                cmd << "($1::bigint," <<
                        "$" << param.size() << "::bigint,"
                        "CURRENT_TIMESTAMP,"
                        "CURRENT_TIMESTAMP,"
                        "$2::timestamp)";
            }
        }

        _ctx.get_conn().exec_params(cmd.str(), param);
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
                new Database::StandaloneConnectionFactory(Database::Manager::getConnectionString()));
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
        typedef std::auto_ptr< Database::StandaloneConnection > StandaloneConnectionPtr;
        Database::StandaloneManager sm = Database::StandaloneManager(
            new Database::StandaloneConnectionFactory(Database::Manager::getConnectionString()));
        StandaloneConnectionPtr conn_standalone(sm.acquire());
        for (MultipleObjectStateId::const_iterator pStateId = state_id_.begin(); pStateId != state_id_.end(); ++pStateId) {
            Database::query_param_list param(*pStateId);
            param(object_id_);
            conn_standalone->exec_params("INSERT INTO object_state_request_lock (state_id,object_id) "
                                         "VALUES ($1::bigint,$2::bigint)", param);
            _ctx.get_conn().exec_params("SELECT lock_object_state_request_lock($1::bigint,$2::bigint)", param);
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
            BOOST_THROW_EXCEPTION(Exception().set_handle_not_found(object_handle_));
        }

        return obj_id_res[0][0];
    }

}//namespace Fred
