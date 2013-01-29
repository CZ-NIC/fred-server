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

/*
nastaveni stavu objektu, (insert do object_state_request) CreateObjectStateRequest
  M handle objektu,
  M typ objektu,
  M seznam stavu (jmena)
  od
  do
*/

#ifndef CREATE_OBJECT_STATE_REQUEST_H_
#define CREATE_OBJECT_STATE_REQUEST_H_

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "fredlib/types.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

#include <boost/date_time/local_time/local_time.hpp>
#include <string>
#include <vector>

namespace Fred
{

    typedef TID ObjectId;
    typedef TID ObjectStateId;
    typedef short int ObjectType;

    void lock_object_state_request_lock(OperationContext &_ctx,
        ObjectStateId _state_id,
        ObjectId _object_id);

    ObjectId get_object_id(OperationContext &_ctx,
        const std::string &_object_handle,
        ObjectType _object_type);

    class CreateObjectStateRequest
    {
    public:
        typedef std::vector< std::string > StatusList;
        typedef boost::posix_time::ptime Time;
        CreateObjectStateRequest(const std::string &_object_handle,
            ObjectType _object_type,
            const StatusList &_status_list);
        CreateObjectStateRequest(const std::string &_object_handle,
            ObjectType _object_type,
            const StatusList &_status_list,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to
            );
        CreateObjectStateRequest& set_valid_from(const Time &_valid_from);
        CreateObjectStateRequest& set_valid_to(const Time &_valid_to);
        void exec(OperationContext &_ctx);

    private:
        const std::string object_handle_;
        const ObjectType object_type_;
        const StatusList status_list_; //list of status names to be set
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
    };//class CreateObjectStateRequest

//exception impl
    class CreateObjectStateRequestException
    : public OperationExceptionImpl<CreateObjectStateRequestException, 2048>
    {
    public:
        CreateObjectStateRequestException(const char* file,
            const int line,
            const char* function,
            const char* data)
        :   OperationExceptionImpl< CreateObjectStateRequestException, 2048 >(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[] = {"not found:handle", "not found:state", "out of turn:valid_from-to",
                                         "overlayed validity time intervals:object"};
            return ConstArr(list, sizeof(list) / sizeof(char*));
        }
    };//class CreateObjectStateRequestException

typedef CreateObjectStateRequestException::OperationErrorType CreateObjectStateRequestError;

}//namespace Fred

#endif//CREATE_OBJECT_STATE_REQUEST_H_
