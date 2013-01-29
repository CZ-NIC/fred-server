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
 *  @file create_administrative_object_block_request.cc
 *  create administrative object block request
 */

#include "fredlib/domain/create_administrative_object_block_request.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>
#include <set>

#define MY_EXCEPTION_CLASS(DATA) CreateAdministrativeObjectBlockRequestException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA) CreateAdministrativeObjectBlockRequestError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

namespace Fred
{

    CreateAdministrativeObjectBlockRequest::CreateAdministrativeObjectBlockRequest(const std::string &_object_handle,
        ObjectType _object_type,
        const StatusList &_status_list)
    :   object_handle_(_object_handle),
        object_type_(_object_type),
        status_list_(_status_list)
    {}

    CreateAdministrativeObjectBlockRequest::CreateAdministrativeObjectBlockRequest(const std::string &_object_handle,
        ObjectType _object_type,
        const StatusList &_status_list,
        const Optional< Time > &_valid_from,
        const Optional< Time > &_valid_to,
        const std::string &_notice)
    :   object_handle_(_object_handle),
        object_type_(_object_type),
        status_list_(_status_list),
        valid_from_(_valid_from),
        valid_to_(_valid_to),
        notice_(_notice)
    {}

    CreateAdministrativeObjectBlockRequest& CreateAdministrativeObjectBlockRequest::set_valid_from(const Time &_valid_from)
    {
        valid_from_ = _valid_from;
        return *this;
    }

    CreateAdministrativeObjectBlockRequest& CreateAdministrativeObjectBlockRequest::set_valid_to(const Time &_valid_to)
    {
        valid_to_ = _valid_to;
        return *this;
    }

    CreateAdministrativeObjectBlockRequest& CreateAdministrativeObjectBlockRequest::set_notice(const std::string &_notice)
    {
        notice_ = _notice;
        return *this;
    }

    void CreateAdministrativeObjectBlockRequest::exec(OperationContext &_ctx)
    {
        this->check_administrative_block_status_only(_ctx);
    }//CreateAdministrativeObjectBlockRequest::exec

    void CreateAdministrativeObjectBlockRequest::check_administrative_block_status_only(OperationContext &_ctx) const
    {
        if (status_list_.empty()) {
            std::string errmsg("|| invalid argument:state: status list empty |");
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
        }
        typedef std::set< std::string > StatusSet;
        static StatusSet administrativeBlockStatusSet; // set of administrative block status
        if (administrativeBlockStatusSet.empty()) {
            Database::Result statusResult = _ctx.get_conn().exec(
              "SELECT name "
              "FROM enum_object_states "
              "WHERE manual AND "
                    "name LIKE 'server%' AND "
                    "name!='serverBlocked'");
            for (::size_t rowIdx = 0; rowIdx < statusResult.size(); ++rowIdx) {
                const std::string state = statusResult[rowIdx][0];
                administrativeBlockStatusSet.insert(state);
            }
        }
        std::string invalidStatus;
        for (StatusList::const_iterator pState = status_list_.begin(); pState != status_list_.end(); ++pState) {
            if (0 < administrativeBlockStatusSet.count(*pState)) {
                invalidStatus += " " + *pState;
            }
        }
        if (!invalidStatus.empty()) {
            std::string errmsg("|| invalid argument:state: unable set" + invalidStatus + " status |");
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
        }
    }

}//namespace Fred
