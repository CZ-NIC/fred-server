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

#include "fredlib/object_state/create_object_state_request.h"
#include <vector>

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
        typedef std::vector< ObjectId > Requests;
        Requests exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(object_id_not_found, ObjectId);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_object_id_not_found<Exception>
        {};
    private:
        const ObjectId object_id_;
    };//class ClearObjectStateRequest


}//namespace Fred

#endif//CLEAR_OBJECT_STATE_REQUEST_ID_H_
