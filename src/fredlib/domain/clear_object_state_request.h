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
 *  @file clear_object_state_request.h
 *  clear object state request
 */

#ifndef CLEAR_OBJECT_STATE_REQUEST_H_
#define CLEAR_OBJECT_STATE_REQUEST_H_

#include "fredlib/domain/create_object_state_request.h"

namespace Fred
{

/*
pozadavek na zruseni vsech stavu objektu (update object_state_request)
  M handle objektu,
  M typ objektu,
*/
    class ClearObjectStateRequest
    {
    public:
        ClearObjectStateRequest(const std::string &_object_handle,
            ObjectType _object_type);
        ObjectId exec(OperationContext &_ctx);

    private:
        const std::string object_handle_;
        const ObjectType object_type_;
    };//class ClearObjectStateRequest


//exception impl
    class ClearObjectStateRequestException
    : public OperationExceptionImpl< ClearObjectStateRequestException, 2048 >
    {
    public:
        ClearObjectStateRequestException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< ClearObjectStateRequestException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"not found:handle"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class ClearObjectStateRequestException

    typedef ClearObjectStateRequestException::OperationErrorType ClearObjectStateRequestError;

}//namespace Fred

#endif//CLEAR_OBJECT_STATE_REQUEST_H_
