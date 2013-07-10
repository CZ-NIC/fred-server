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
 *  @file clear_object_state_request_id.h
 *  clear object state request
 */

#ifndef CLEAR_OBJECT_STATE_REQUEST_ID_H_
#define CLEAR_OBJECT_STATE_REQUEST_ID_H_

#include "fredlib/domain/create_object_state_request.h"

namespace Fred
{

/*
pozadavek na zruseni vsech stavu objektu (update object_state_request)
  M id objektu,
*/
    class ClearObjectStateRequestId
    {
    public:
        ClearObjectStateRequestId(ObjectId _object_id);
        void exec(OperationContext &_ctx);

    private:
        const ObjectId object_id_;
    };//class ClearObjectStateRequest


//exception impl
    class ClearObjectStateRequestIdException
    : public OperationExceptionImpl< ClearObjectStateRequestIdException, 2048 >
    {
    public:
        ClearObjectStateRequestIdException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< ClearObjectStateRequestIdException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"not found:object_id"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class ClearObjectStateRequestIdException

    typedef ClearObjectStateRequestIdException::OperationErrorType ClearObjectStateRequestIdError;

}//namespace Fred

#endif//CLEAR_OBJECT_STATE_REQUEST_ID_H_
