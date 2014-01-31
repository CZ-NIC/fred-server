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
 *  @file
 *  create object state request
 */

#ifndef CREATE_OBJECT_STATE_REQUEST_ID_H_
#define CREATE_OBJECT_STATE_REQUEST_ID_H_

#include "src/fredlib/object_state/create_object_state_request.h"

namespace Fred
{

    /**
    * Create request for setting state of object. Use integer id as domain identification.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the update.
    * Creation is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref Exception is thrown
    * with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class CreateObjectStateRequestId
    {
    public:
        typedef boost::posix_time::ptime Time; /**< POSIX time functionality encapsulation */

        /**
        * Constructor with mandatory parameters.
        * @param _object_id sets domain id into @ref object_id_ attribute
        * @param _status_list sets states into @ref status_list_ attribute
        */
        CreateObjectStateRequestId(ObjectId _object_id,
            const StatusList &_status_list);

        /**
        * Constructor with all parameters.
        * @param _object_id sets domain id into @ref object_id_ attribute
        * @param _status_list sets states into @ref status_list_ attribute
        * @param _valid_from sets time into @ref valid_from_ attribute
        * @param _valid_to sets time into @ref valid_to_ attribute
        */
        CreateObjectStateRequestId(ObjectId _object_id,
            const StatusList &_status_list,
            const Optional< Time > &_valid_from,
            const Optional< Time > &_valid_to
            );

        /**
        * Sets time of validity starts
        * @param _valid_from sets time into @ref valid_from_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateObjectStateRequestId& set_valid_from(const Time &_valid_from);

        /**
        * Sets time of validity finishes
        * @param _valid_to sets time into @ref valid_to_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateObjectStateRequestId& set_valid_to(const Time &_valid_to);

        /**
        * Executes creation
        * @param _ctx contains reference to database and logging interface
        * @return domain handle (fqdn)
        */
        std::string exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(object_id_not_found, ObjectId); /**< exception members for object id no found generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_VECTOR_OF_EXCEPTION_DATA(state_not_found, std::string); /**< exception members for unknown state name generated by macro @ref DECLARE_VECTOR_OF_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(out_of_turn, std::string); /**< exception members for times out of turn generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(overlayed_time_intervals, std::string); /**< exception members for time interval overlayed generated by macro @ref DECLARE_EXCEPTION_DATA*/

        /**
        * This exception is thrown in case of wrong input data or other predictable and superable failure.
        */
        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_object_id_not_found<Exception>,
            ExceptionData_vector_of_state_not_found<Exception>,
            ExceptionData_out_of_turn<Exception>,
            ExceptionData_overlayed_time_intervals<Exception>
        {};

    private:
        const ObjectId object_id_; /**< domain integer identificator */
        const StatusList status_list_; /**< list of status names to be set */
        Optional< Time > valid_from_; /**< status validity starts from */
        Optional< Time > valid_to_; /**< status is valid until this time */
    };//class CreateObjectStateRequest

}//namespace Fred

#endif//CREATE_OBJECT_STATE_REQUEST_ID_H_
