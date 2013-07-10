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
 *  @file create_administrative_object_block_request_id.h
 *  create administrative object block request
 */

/*
administrativni nastaveni stavu blokovani objektu, (insert do object_state_request) CreateAdministrativeObjectBlockRequestId
  M id objektu,
  M seznam stavu (jmena)
  od
  do
  duvod blokace
*/

#ifndef CREATE_ADMINISTRATIVE_OBJECT_BLOCK_REQUEST_ID_H_
#define CREATE_ADMINISTRATIVE_OBJECT_BLOCK_REQUEST_ID_H_

#include "fredlib/domain/create_object_state_request_id.h"

namespace Fred
{

    class CreateAdministrativeObjectBlockRequestId
    {
    public:
        typedef CreateObjectStateRequest::Time Time;
        CreateAdministrativeObjectBlockRequestId(ObjectId _object_id,
            const StatusList &_status_list);
        CreateAdministrativeObjectBlockRequestId(ObjectId _object_id,
            const StatusList &_status_list,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to,
            const std::string &_reason
            );
        CreateAdministrativeObjectBlockRequestId& set_valid_from(const Time &_valid_from);
        CreateAdministrativeObjectBlockRequestId& set_valid_to(const Time &_valid_to);
        CreateAdministrativeObjectBlockRequestId& set_reason(const std::string &_reason);
        void exec(OperationContext &_ctx);

    //exception impl
        enum { EXCEPTION_DATASIZE = 2048 };
        class Exception
        : public OperationExceptionImpl< Exception, EXCEPTION_DATASIZE >
        {
        public:
            Exception(const char* file,
                const int line,
                const char* function,
                const char* data)
            :   OperationExceptionImpl< Exception, EXCEPTION_DATASIZE >(file, line, function, data)
            {}

            ConstArr get_fail_param_impl() throw()
            {
                static const char* list[] = {"invalid argument:state", "not found:state", "serverBlocked:present"};
                return ConstArr(list, sizeof(list) / sizeof(char*));
            }
        };//class CreateAdministrativeObjectBlockRequest::Exception

        typedef Exception::OperationErrorType Error;
    private:
        void check_administrative_block_status_only(OperationContext &_ctx) const;
        void check_server_blocked_status_absent(OperationContext &_ctx) const;
        const ObjectId object_id_;
        const StatusList status_list_; //list of status names to be set
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
        Optional< std::string > reason_;
    };//class CreateAdministrativeObjectBlockRequestId


}//namespace Fred

#endif//CREATE_ADMINISTRATIVE_OBJECT_BLOCK_REQUEST_ID_H_
