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
 *  @file create_object_state_request.h
 *  create object state request
 */

#ifndef CREATE_OBJECT_STATE_REQUEST_ID_H_
#define CREATE_OBJECT_STATE_REQUEST_ID_H_

#include "fredlib/domain/create_object_state_request.h"

namespace Fred
{

/*
pozadavek na nastaveni stavu objektu (insert do object_state_request)
  M id objektu,
  M seznam stavu (jmena)
  od
  do
*/
    class CreateObjectStateRequestId
    {
    public:
        typedef boost::posix_time::ptime Time;
        CreateObjectStateRequestId(ObjectId _object_id,
            const StatusList &_status_list);
        CreateObjectStateRequestId(ObjectId _object_id,
            const StatusList &_status_list,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to
            );
        CreateObjectStateRequestId& set_valid_from(const Time &_valid_from);
        CreateObjectStateRequestId& set_valid_to(const Time &_valid_to);
        void exec(OperationContext &_ctx);

    private:
        const ObjectId object_id_;
        const StatusList status_list_; //list of status names to be set
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
    };//class CreateObjectStateRequest

//exception impl
    class CreateObjectStateRequestIdException
    : public OperationExceptionImpl<CreateObjectStateRequestIdException, 2048>
    {
    public:
        CreateObjectStateRequestIdException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< CreateObjectStateRequestIdException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"not found:object_id", "not found:state", "out of turn:valid_from-to",
                                         "overlayed validity time intervals:object"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class CreateObjectStateRequestIdException

    typedef CreateObjectStateRequestIdException::OperationErrorType CreateObjectStateRequestIdError;

}//namespace Fred

#endif//CREATE_OBJECT_STATE_REQUEST_ID_H_
