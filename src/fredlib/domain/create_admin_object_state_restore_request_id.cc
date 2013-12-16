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
 *  @file create_admin_object_state_restore_request_id.cc
 *  create administrative object state restore request
 */

#include "fredlib/domain/create_admin_object_state_restore_request_id.h"
#include "fredlib/domain/clear_admin_object_state_request_id.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"
#include "clear_object_state_request_id.h"

#include <boost/algorithm/string.hpp>
#include <set>

namespace Fred
{

    CreateAdminObjectStateRestoreRequestId::CreateAdminObjectStateRestoreRequestId(ObjectId _object_id)
    :   object_id_(_object_id)
    {}

    CreateAdminObjectStateRestoreRequestId::CreateAdminObjectStateRestoreRequestId(ObjectId _object_id,
        const std::string &_reason,
        const Optional<unsigned long long> _logd_request_id)
    :   object_id_(_object_id),
        reason_(_reason),
        logd_request_id_(_logd_request_id)
    {}

    CreateAdminObjectStateRestoreRequestId& CreateAdminObjectStateRestoreRequestId::set_reason(const std::string &_reason)
    {
        reason_ = _reason;
        return *this;
    }

    CreateAdminObjectStateRestoreRequestId& CreateAdminObjectStateRestoreRequestId::set_logd_request_id(unsigned long long _logd_request_id)
    {
        logd_request_id_ = _logd_request_id;
        return *this;
    }
    
    void CreateAdminObjectStateRestoreRequestId::exec(OperationContext &_ctx)
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
                BOOST_THROW_EXCEPTION(Exception().set_object_id_not_found(object_id_));
            }
            const Database::Row &row = object_type_result[0];
            object_type = static_cast< ObjectType >(row[0]);
        }

        const ObjectStateId server_blocked_id = this->check_server_blocked_status_present(_ctx);
        enum ResultColumnIndex
        {
            OBJECT_STATE_ID_IDX = 0,
            CURRENT_IDX         = 1,
            FOLLOW_UP_IDX       = 2,
        };
        Database::Result block_history = _ctx.get_conn().exec_params(
            "SELECT os.id," // STATE_ID
                   "os.valid_to IS NULL OR CURRENT_TIMESTAMP<=os.valid_to," // CURRENT
                   "COALESCE((SELECT valid_to "
                    "FROM object_state "
                    "WHERE object_id=$1::bigint AND state_id=$2::bigint AND valid_from<os.valid_from "
                    "ORDER BY valid_from DESC LIMIT 1"
                   ")=os.valid_from,false) " // FOLLOW_UP
            "FROM object_state os "
            "WHERE os.object_id=$1::bigint AND os.state_id=$2::bigint ORDER BY os.valid_from DESC",
            Database::query_param_list(object_id_)(server_blocked_id));
        Database::Result::Iterator pRow = block_history.begin();
        // seek on actual serverBlocked
        while (pRow != block_history.end()) {
            if (bool((*pRow)[CURRENT_IDX])) {
                break;
            }
            ++pRow;
        }
        // seek on begin of serverBlocked sequence
        while (pRow != block_history.end()) {
            if (!bool((*pRow)[FOLLOW_UP_IDX])) {
                break;
            }
            ++pRow;
        }
        StatusList previous_status_list;
        if (pRow != block_history.end()) {
            Database::Result status_result = _ctx.get_conn().exec_params(
                "SELECT id "
                "FROM enum_object_states "
                "WHERE $1::integer=ANY(types)",
                Database::query_param_list(object_type));
            MultipleObjectStateId status_all;
            for (Database::Result::Iterator pStatusRow = status_result.begin(); pStatusRow != status_result.end(); ++pStatusRow) {
                status_all.insert((*pStatusRow)[0]);
            }
            LockMultipleObjectStateRequestLock(status_all, object_id_).exec(_ctx);

            const TID start_object_state_id = (*pRow)[OBJECT_STATE_ID_IDX];
            Database::Result previous_status_list_result = _ctx.get_conn().exec_params(
                "SELECT eos.name "
                "FROM (SELECT object_id,valid_from FROM object_state WHERE id=$1::integer) oss "
                "JOIN object_state os ON (os.object_id=oss.object_id AND "
                                         "os.valid_from<oss.valid_from AND "
                                         "(os.valid_to IS NULL OR oss.valid_from<=os.valid_to)) "
                "JOIN enum_object_states eos ON eos.id=os.state_id "
                "WHERE eos.manual AND name LIKE 'server%'",
                Database::query_param_list(start_object_state_id));
            for (Database::Result::Iterator pName = previous_status_list_result.begin();
                 pName != previous_status_list_result.end(); ++pName) {
                previous_status_list.insert((*pName)[0]);
            }
        }
        try {
            ClearAdminObjectStateRequestId(object_id_, reason_).exec(_ctx);
        }
        catch (const ClearAdminObjectStateRequestId::Exception &ex) {
            if (ex.is_set_server_blocked_absent()) {
                BOOST_THROW_EXCEPTION(Exception().set_server_blocked_absent(ex.get_server_blocked_absent()));
            }
            throw;
        }
        if (!previous_status_list.empty()) {
            CreateObjectStateRequestId(object_id_, previous_status_list).exec(_ctx);
        }
    }

    ObjectStateId CreateAdminObjectStateRestoreRequestId::check_server_blocked_status_present(OperationContext &_ctx) const
    {
        static ObjectStateId server_blocked_id = 0;
        if (server_blocked_id == 0) {
            Database::Result obj_state_res = _ctx.get_conn().exec(
                "SELECT id "
                "FROM enum_object_states "
                "WHERE name='serverBlocked'");

            if (obj_state_res.size() != 1) {
                BOOST_THROW_EXCEPTION(Exception().set_state_not_found("serverBlocked"));
            }
            server_blocked_id = obj_state_res[0][0];
            _ctx.get_log().debug("serverBlockedId = " + boost::lexical_cast< std::string >(server_blocked_id));
        }
        _ctx.get_log().debug("LockObjectStateRequestLock call");
        LockObjectStateRequestLock(server_blocked_id, object_id_).exec(_ctx);
        _ctx.get_log().debug("LockObjectStateRequestLock success");
        Database::Result rcheck = _ctx.get_conn().exec_params(
            "SELECT 1 "
            "FROM object_state "
            "WHERE object_id=$1::integer AND "
                  "state_id=$2::integer AND "
                  "valid_from<=CURRENT_TIMESTAMP AND "
                  "(valid_to IS NULL OR "
                   "CURRENT_TIMESTAMP<valid_to) "
                  "LIMIT 1",
            Database::query_param_list
                (object_id_)
                (server_blocked_id));
        if (0 < rcheck.size()) {
            return server_blocked_id;
        }
        BOOST_THROW_EXCEPTION(Exception().set_server_blocked_absent(object_id_));
    }

}//namespace Fred
