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
 *  @file clear_admin_object_state_request_id.cc
 *  clear all administrative object state requests
 */

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

    ClearAdminObjectStateRequestId::ClearAdminObjectStateRequestId(ObjectId _object_id)
    :   object_id_(_object_id)
    {}

    ClearAdminObjectStateRequestId::ClearAdminObjectStateRequestId(ObjectId _object_id,
        const std::string &_reason)
    :   object_id_(_object_id),
        reason_(_reason)
    {}

    ClearAdminObjectStateRequestId& ClearAdminObjectStateRequestId::set_reason(const std::string &_reason)
    {
        reason_ = _reason;
        return *this;
    }

    void ClearAdminObjectStateRequestId::exec(OperationContext &_ctx)
    {
        try {
            ClearObjectStateRequestId::Requests requests = ClearObjectStateRequestId(object_id_).exec(_ctx);
            if (requests.empty()) {
                BOOST_THROW_EXCEPTION(Exception().set_server_blocked_absent(object_id_));
            }
            ClearObjectStateRequestId::Requests::const_iterator pRequestId = requests.begin();
            Database::query_param_list param(*pRequestId);
            std::ostringstream query;
            query << "SELECT osr.id "
                     "FROM object_state_request osr "
                     "JOIN enum_object_states eos ON eos.id=osr.state_id "
                     "WHERE osr.id IN ($" << param.size() << "::integer";
            for (++pRequestId; pRequestId != requests.end(); ++pRequestId) {
                param(*pRequestId);
                query << ",$" << param.size() << "::integer";
            }
            query << ") AND eos.name='serverBlocked' LIMIT 1";
            Database::Result osr_id_res = _ctx.get_conn().exec_params(query.str(), param);
            if (osr_id_res.size() != 1) {
                BOOST_THROW_EXCEPTION(Exception().set_server_blocked_absent(object_id_));
            }
            const ObjectId osr_id = static_cast< ObjectId >(osr_id_res[0][0]);
            if (reason_.isset()) {
                _ctx.get_conn().exec_params(
                    "UPDATE object_state_request_reason "
                    "SET reason_cancellation=$1::text "
                    "WHERE object_state_request_id=$2::integer",
                    Database::query_param_list(reason_.get_value())
                                              (osr_id));
            }
            else {
                _ctx.get_conn().exec_params(
                    "UPDATE object_state_request_reason "
                    "SET reason_cancellation=NULL "
                    "WHERE object_state_request_id=$1::integer",
                    Database::query_param_list(osr_id));
            }
        }
        catch (const ClearObjectStateRequestId::Exception &e) {
            if (e.is_set_object_id_not_found()) {
                BOOST_THROW_EXCEPTION(Exception().set_object_id_not_found(e.get_object_id_not_found()));
            }
        }
    }

}//namespace Fred
