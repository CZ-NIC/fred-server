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
 *  @file cancel_object_state_request.h
 *  cancel object state request
 */

#ifndef CANCEL_OBJECT_STATE_REQUEST_ID_H_
#define CANCEL_OBJECT_STATE_REQUEST_ID_H_

#include "fredlib/domain/create_object_state_request.h"

namespace Fred
{

/*
pozadavek na zruseni stavu objektu (update object_state_request)
  M id objektu,
  M seznam stavu (jmena)
*/
    class CancelObjectStateRequestId
    {
    public:
        typedef boost::posix_time::ptime Time;
        CancelObjectStateRequestId(ObjectId _object_id,
            const StatusList &_status_list);
        void exec(OperationContext &_ctx);

    private:
        const ObjectId object_id_;
        const StatusList status_list_; //list of status names to be canceled
    };//class CancelObjectStateRequestId


//exception impl
    class CancelObjectStateRequestIdException
    : public OperationExceptionImpl<CancelObjectStateRequestIdException, 2048>
    {
    public:
        CancelObjectStateRequestIdException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< CancelObjectStateRequestIdException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"not found:object_id", "not found:state"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class CancelObjectStateRequestException

    typedef CancelObjectStateRequestIdException::OperationErrorType CancelObjectStateRequestIdError;

}//namespace Fred

#endif//CANCEL_OBJECT_STATE_REQUEST_ID_H_
