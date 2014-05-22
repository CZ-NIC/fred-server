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

#ifndef CREATE_OBJECT_STATE_REQUEST_H_
#define CREATE_OBJECT_STATE_REQUEST_H_

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/types.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "lock_object_state_request_lock.h"
#include "perform_object_state_request.h"

#include <boost/date_time/local_time/local_time.hpp>
#include <string>
#include <set>

namespace Fred
{

    typedef TID ObjectId;
    typedef TID ObjectStateId;
    typedef std::set< ObjectStateId > MultipleObjectStateId;
    typedef short int ObjectType;
    typedef std::set< std::string > StatusList;

/*
pozadavek na nastaveni stavu objektu (insert do object_state_request)
  M handle objektu,
  M typ objektu,
  M seznam stavu (jmena)
  od
  do
*/
    class CreateObjectStateRequest
    {
    public:
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
        ObjectId exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(state_not_found, std::string);
        DECLARE_EXCEPTION_DATA(out_of_turn, std::string);
        DECLARE_EXCEPTION_DATA(overlayed_time_intervals, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_state_not_found<Exception>,
            ExceptionData_out_of_turn<Exception>,
            ExceptionData_overlayed_time_intervals<Exception>
        {};
    private:
        const std::string object_handle_;
        const ObjectType object_type_;
        const StatusList status_list_; //list of status names to be set
        Optional< Time > valid_from_;
        Optional< Time > valid_to_;
    };//class CreateObjectStateRequest


    class GetObjectId
    {
    public:
        GetObjectId(const std::string &_object_handle, ObjectType _object_type);
        ObjectId exec(OperationContext &_ctx);
    //exception impl
        DECLARE_EXCEPTION_DATA(handle_not_found, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_handle_not_found<Exception>
        {};
    private:
        const std::string object_handle_;
        const ObjectType object_type_;
    };

}//namespace Fred

#endif//CREATE_OBJECT_STATE_REQUEST_H_
