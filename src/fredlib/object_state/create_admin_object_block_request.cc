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
 *  @file create_admin_object_block_request.cc
 *  create administrative object block request
 */

#include "src/fredlib/object_state/create_admin_object_block_request.h"
#include "src/fredlib/object_state/get_blocking_status_desc_list.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "src/fredlib/object.h"

#include <boost/algorithm/string.hpp>
#include <set>

namespace Fred
{

    CreateAdminObjectBlockRequest::CreateAdminObjectBlockRequest(const std::string &_object_handle,
        ObjectType _object_type,
        const StatusList &_status_list)
    :   object_handle_(_object_handle),
        object_type_(_object_type),
        status_list_(_status_list)
    {}

    CreateAdminObjectBlockRequest::CreateAdminObjectBlockRequest(const std::string &_object_handle,
        ObjectType _object_type,
        const StatusList &_status_list,
        const Optional< Time > &_valid_from,
        const Optional< Time > &_valid_to,
        const std::string &_reason)
    :   object_handle_(_object_handle),
        object_type_(_object_type),
        status_list_(_status_list),
        valid_from_(_valid_from),
        valid_to_(_valid_to),
        reason_(_reason)
    {}

    CreateAdminObjectBlockRequest& CreateAdminObjectBlockRequest::set_valid_from(const Time &_valid_from)
    {
        valid_from_ = _valid_from;
        return *this;
    }

    CreateAdminObjectBlockRequest& CreateAdminObjectBlockRequest::set_valid_to(const Time &_valid_to)
    {
        valid_to_ = _valid_to;
        return *this;
    }

//  CREATE TABLE object_state_request_reason
//  (
//    object_state_request_id INTEGER NOT NULL REFERENCES object_state_request (id),
//    state_on BOOL NOT NULL,
//    reason VARCHAR(300) NOT NULL,
//    PRIMARY KEY (object_state_request_id,state_on)
//  )
    CreateAdminObjectBlockRequest& CreateAdminObjectBlockRequest::set_reason(const std::string &_reason)
    {
        reason_ = _reason;
        return *this;
    }

    ObjectId CreateAdminObjectBlockRequest::exec(OperationContext &_ctx)
    {
        this->check_administrative_block_status_only(_ctx);
        this->check_server_blocked_status_absent(_ctx);
        StatusList status_list = status_list_;
        status_list.insert("serverBlocked");
        CreateObjectStateRequest createObjectStateRequest(object_handle_,
            object_type_,
            status_list,
            valid_from_,
            valid_to_);
        const ObjectId object_id = createObjectStateRequest.exec(_ctx);
        if (reason_.isset()) {
            _ctx.get_conn().exec_params(
                "INSERT INTO object_state_request_reason (object_state_request_id,reason_creation,reason_cancellation) "
                    "SELECT osr.id,$1::text,NULL "
                    "FROM object_state_request osr "
                    "JOIN enum_object_states eos ON eos.id=osr.state_id "
                    "WHERE osr.valid_from<=CURRENT_TIMESTAMP AND "
                          "(osr.valid_to IS NULL OR "
                           "CURRENT_TIMESTAMP<=osr.valid_to) AND "
                          "osr.canceled IS NULL AND "
                          "osr.object_id=$2::integer AND "
                          "eos.name='serverBlocked' "
                    "ORDER BY osr.id DESC LIMIT 1",
                Database::query_param_list(reason_.get_value())
                                          (object_id));
        }
        else {
            _ctx.get_conn().exec_params(
                "INSERT INTO object_state_request_reason (object_state_request_id,reason_creation,reason_cancellation) "
                    "SELECT osr.id,NULL,NULL "
                    "FROM object_state_request osr "
                    "JOIN enum_object_states eos ON eos.id=osr.state_id "
                    "WHERE osr.valid_from<=CURRENT_TIMESTAMP AND "
                          "(osr.valid_to IS NULL OR "
                           "CURRENT_TIMESTAMP<=osr.valid_to) AND "
                          "osr.canceled IS NULL AND "
                          "osr.object_id=$1::integer AND "
                          "eos.name='serverBlocked' "
                    "ORDER BY osr.id DESC LIMIT 1",
                Database::query_param_list(object_id));
        }
        return object_id;
    }//CreateAdminObjectBlockRequest::exec

    void CreateAdminObjectBlockRequest::check_administrative_block_status_only(OperationContext &_ctx) const
    {
        if (status_list_.empty()) {
            BOOST_THROW_EXCEPTION(Exception().set_invalid_argument("status list empty"));
        }
        typedef std::set< std::string > StatusSet;
        typedef GetBlockingStatusDescList::StatusDescList StatusDescList;
        static StatusSet administrativeBlockStatusSet; // set of administrative block status
        if (administrativeBlockStatusSet.empty()) {
            GetBlockingStatusDescList getBlockingStatusDescList;
            const StatusDescList &statusDescList = getBlockingStatusDescList.exec(_ctx);
            for (StatusDescList::const_iterator pItem = statusDescList.begin(); pItem != statusDescList.end(); ++pItem) {
                administrativeBlockStatusSet.insert(pItem->status);
            }
        }
        std::string invalidStatus;
        for (StatusList::const_iterator pState = status_list_.begin(); pState != status_list_.end(); ++pState) {
            if (administrativeBlockStatusSet.count(*pState) <= 0) {
                invalidStatus += " " + *pState;
            }
        }
        if (!invalidStatus.empty()) {
            BOOST_THROW_EXCEPTION(Exception().set_invalid_argument("unable to set" + invalidStatus + " status"));
        }
    }

    void CreateAdminObjectBlockRequest::check_server_blocked_status_absent(OperationContext &_ctx) const
    {
        const ObjectId object_id = GetObjectId(object_handle_, object_type_).exec(_ctx);
        static TID serverBlockedId = 0;
        if (serverBlockedId == 0) {
            Database::Result obj_state_res = _ctx.get_conn().exec(
                "SELECT id "
                "FROM enum_object_states "
                "WHERE name='serverBlocked'");

            if (obj_state_res.size() != 1) {
                BOOST_THROW_EXCEPTION(Exception().set_state_not_found("serverBlocked"));
            }
            serverBlockedId = obj_state_res[0][0];
            _ctx.get_log().debug("serverBlockedId = " + boost::lexical_cast< std::string >(serverBlockedId));
        }
        _ctx.get_log().debug("LockObjectStateRequestLock call");
        LockObjectStateRequestLock(object_id).exec(_ctx);
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
                (object_id)
                (serverBlockedId));
        if (rcheck.size() <= 0) {
            return;
        }
        BOOST_THROW_EXCEPTION(Exception().set_server_blocked_present(object_handle_));
    }

}//namespace Fred
