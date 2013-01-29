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
 *  @file create_administrative_object_block_request.h
 *  create administrative object block request
 */

/*
administrativni nastaveni stavu blokovani objektu, (insert do object_state_request) CreateAdministrativeObjectBlockRequest
  M handle objektu,
  M typ objektu,
  M seznam stavu (jmena)
  od
  do
  poznamka
*/

#ifndef CREATE_ADMINISTRATIVE_OBJECT_BLOCK_REQUEST_H_
#define CREATE_ADMINISTRATIVE_OBJECT_BLOCK_REQUEST_H_

#include "fredlib/domain/create_object_state_request.h"

namespace Fred
{

    class CreateAdministrativeObjectBlockRequest
    {
    public:
        typedef CreateObjectStateRequest::StatusList StatusList;
        typedef CreateObjectStateRequest::Time Time;
        CreateAdministrativeObjectBlockRequest(const std::string &_object_handle,
            ObjectType _object_type,
            const StatusList &_status_list);
        CreateAdministrativeObjectBlockRequest(const std::string &_object_handle,
            ObjectType _object_type,
            const StatusList &_status_list,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to,
            const std::string &_notice
            );
        CreateAdministrativeObjectBlockRequest& set_valid_from(const Time &_valid_from);
        CreateAdministrativeObjectBlockRequest& set_valid_to(const Time &_valid_to);
        CreateAdministrativeObjectBlockRequest& set_notice(const std::string &_notice);
        void exec(OperationContext &_ctx);

    private:
        void check_administrative_block_status_only(OperationContext &_ctx) const;
        void check_server_blocked_status_absent(OperationContext &_ctx) const;
        const std::string object_handle_;
        const ObjectType object_type_;
        const StatusList status_list_; //list of status names to be set
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
        Optional< std::string > notice_;
    };//class CreateAdministrativeObjectBlockRequest

//exception impl
    class CreateAdministrativeObjectBlockRequestException
    : public OperationExceptionImpl<CreateAdministrativeObjectBlockRequestException, 2048>
    {
    public:
        CreateAdministrativeObjectBlockRequestException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< CreateAdministrativeObjectBlockRequestException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"invalid argument:state", "not found:state", "serverBlocked:present"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class CreateAdministrativeObjectBlockRequestException

typedef CreateAdministrativeObjectBlockRequestException::OperationErrorType CreateAdministrativeObjectBlockRequestError;

}//namespace Fred

#endif//CREATE_ADMINISTRATIVE_OBJECT_BLOCK_REQUEST_H_
