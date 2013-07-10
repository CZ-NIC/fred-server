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
 *  @file create_administrative_object_block_request_id.cc
 *  create administrative object block request
 */

#include "fredlib/domain/create_administrative_object_block_request_id.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>
#include <set>

#ifndef __ASSERT_FUNCTION
#define __ASSERT_FUNCTION __PRETTY_FUNCTION__
#endif

#define MY_EXCEPTION_CLASS(DATA) CreateAdministrativeObjectBlockRequestId::Exception(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA) CreateAdministrativeObjectBlockRequestId::Error(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

namespace Fred
{

    CreateAdministrativeObjectBlockRequestId::CreateAdministrativeObjectBlockRequestId(ObjectId _object_id,
        const StatusList &_status_list)
    :   object_id_(_object_id),
        status_list_(_status_list)
    {}

    CreateAdministrativeObjectBlockRequestId::CreateAdministrativeObjectBlockRequestId(ObjectId _object_id,
        const StatusList &_status_list,
        const Optional< Time > &_valid_from,
        const Optional< Time > &_valid_to,
        const std::string &_reason)
    :   object_id_(_object_id),
        status_list_(_status_list),
        valid_from_(_valid_from),
        valid_to_(_valid_to),
        reason_(_reason)
    {}

    CreateAdministrativeObjectBlockRequestId& CreateAdministrativeObjectBlockRequestId::set_valid_from(const Time &_valid_from)
    {
        valid_from_ = _valid_from;
        return *this;
    }

    CreateAdministrativeObjectBlockRequestId& CreateAdministrativeObjectBlockRequestId::set_valid_to(const Time &_valid_to)
    {
        valid_to_ = _valid_to;
        return *this;
    }

    CreateAdministrativeObjectBlockRequestId& CreateAdministrativeObjectBlockRequestId::set_reason(const std::string &_reason)
    {
        reason_ = _reason;
        return *this;
    }

    void CreateAdministrativeObjectBlockRequestId::exec(OperationContext &_ctx)
    {
        this->check_administrative_block_status_only(_ctx);
        this->check_server_blocked_status_absent(_ctx);
        StatusList status_list = status_list_;
        status_list.push_back("serverBlocked");
        CreateObjectStateRequestId createObjectStateRequestId(object_id_,
            status_list,
            valid_from_,
            valid_to_);
        createObjectStateRequestId.exec(_ctx);
        if (reason_.isset()) {
            Database::Result request_id_res = _ctx.get_conn().exec_params(
                "INSERT INTO object_blocked (object_state_request_id,state_on,reason) "
                    "SELECT osr.id,true,$1 "
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
                                          (object_id_));
        }
    }//CreateAdministrativeObjectBlockRequestId::exec

    void CreateAdministrativeObjectBlockRequestId::check_administrative_block_status_only(OperationContext &_ctx) const
    {
        if (status_list_.empty()) {
            std::string errmsg("|| invalid argument:state: status list empty |");
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
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
            std::string errmsg("|| invalid argument:state: unable to set" + invalidStatus + " status |");
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
        }
    }

    void CreateAdministrativeObjectBlockRequestId::check_server_blocked_status_absent(OperationContext &_ctx) const
    {
        static TID serverBlockedId = 0;
        if (serverBlockedId == 0) {
            Database::Result obj_state_res = _ctx.get_conn().exec(
                "SELECT id "
                "FROM enum_object_states "
                "WHERE name='serverBlocked'");

            if (obj_state_res.size() != 1) {
                throw MY_EXCEPTION_CLASS("|| not found:state: serverBlocked |");
            }
            serverBlockedId = obj_state_res[0][0];
            _ctx.get_log().debug("serverBlockedId = " + boost::lexical_cast< std::string >(serverBlockedId));
        }
        _ctx.get_log().debug("LockObjectStateRequestLock call");
        LockObjectStateRequestLock(serverBlockedId, object_id_).exec(_ctx);
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
                (serverBlockedId));
        if (rcheck.size() <= 0) {
            return;
        }
        std::string errmsg("|| serverBlocked:present: object_id ");
        errmsg += boost::lexical_cast< std::string >(object_id_);
        errmsg += " |";
        throw MY_EXCEPTION_CLASS(errmsg.c_str());
    }

}//namespace Fred
