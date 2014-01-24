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
 *  @file create_admin_object_block_request.h
 *  create administrative object block request
 */


#ifndef CREATE_ADMIN_OBJECT_BLOCK_REQUEST_H_
#define CREATE_ADMIN_OBJECT_BLOCK_REQUEST_H_

#include "src/fredlib/object_state/create_object_state_request.h"

namespace Fred
{

    class CreateAdminObjectBlockRequest
    {
    public:
        typedef CreateObjectStateRequest::Time Time;
        CreateAdminObjectBlockRequest(const std::string &_object_handle,
            ObjectType _object_type,
            const StatusList &_status_list);
        CreateAdminObjectBlockRequest(const std::string &_object_handle,
            ObjectType _object_type,
            const StatusList &_status_list,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to,
            const std::string &_reason
            );
        CreateAdminObjectBlockRequest& set_valid_from(const Time &_valid_from);
        CreateAdminObjectBlockRequest& set_valid_to(const Time &_valid_to);
        CreateAdminObjectBlockRequest& set_reason(const std::string &_reason);
        ObjectId exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(invalid_argument, std::string);
        DECLARE_EXCEPTION_DATA(state_not_found, std::string);
        DECLARE_EXCEPTION_DATA(server_blocked_present, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_invalid_argument<Exception>,
            ExceptionData_state_not_found<Exception>,
            ExceptionData_server_blocked_present<Exception>
        {};
    private:
        void check_administrative_block_status_only(OperationContext &_ctx) const;
        void check_server_blocked_status_absent(OperationContext &_ctx) const;
        const std::string object_handle_;
        const ObjectType object_type_;
        const StatusList status_list_; //list of status names to be set
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
        Optional< std::string > reason_;
    };//class CreateAdminObjectBlockRequest


}//namespace Fred

#endif//CREATE_ADMIN_OBJECT_BLOCK_REQUEST_H_
