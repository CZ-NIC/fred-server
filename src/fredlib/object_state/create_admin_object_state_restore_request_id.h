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
 *  create administrative object state restore request
 */

#ifndef CREATE_ADMIN_OBJECT_STATE_RESTORE_REQUEST_ID_H_
#define CREATE_ADMIN_OBJECT_STATE_RESTORE_REQUEST_ID_H_

#include <string>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object_state/typedefs.h"
#include "util/optional_value.h"

namespace Fred
{

    /**
    * Create request for restoring state before administrative blocking of object. Use integer id as domain identification.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the update.
    * Creation is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref Exception is thrown
    * with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class CreateAdminObjectStateRestoreRequestId
    {
    public:

        /**
        * Constructor with mandatory parameters.
        * @param _object_id sets domain id into @ref object_id_ attribute
        */
        CreateAdminObjectStateRestoreRequestId(ObjectId _object_id);

        /**
        * Constructor with all parameters.
        * @param _object_id sets domain id into @ref object_id_ attribute
        * @param _reason sets reason of restoring into @ref reason_ attribute
        * @param _logd_request_id sets sets logger request id into @ref logd_request_id_ attribute
        */
        CreateAdminObjectStateRestoreRequestId(ObjectId _object_id,
            const std::string &_reason,
            const Optional<unsigned long long> _logd_request_id
            );

        /**
        * Sets reason of restoring
        * @param _reason sets reason of restoring into @ref reason_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateAdminObjectStateRestoreRequestId& set_reason(const std::string &_reason);

        /**
        * Sets logger request id
        * @param _logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateAdminObjectStateRestoreRequestId& set_logd_request_id(unsigned long long _logd_request_id);

        /**
        * Executes creation
        * @param _ctx contains reference to database and logging interface
        */
        void exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(object_id_not_found, ObjectId); /**< exception members in case domain id is not found generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(state_not_found, std::string); /**< exception members for unknown state name generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(server_blocked_absent, ObjectId); /**< exception members in case domain is not blocked generated by macro @ref DECLARE_EXCEPTION_DATA*/

        /**
        * This exception is thrown in case of wrong input data or other predictable and superable failure.
        */
        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_object_id_not_found<Exception>,
            ExceptionData_state_not_found<Exception>,
            ExceptionData_server_blocked_absent<Exception>
        {};

    private:
        ObjectStateId check_server_blocked_status_present(OperationContext &_ctx) const;
        const ObjectId object_id_; /**< domain integer identificator */
        Optional< std::string > reason_; /**< reason of administrative restoring */
        Nullable< unsigned long long > logd_request_id_; /**< id of the record in logger database, id is used in other calls to logging within current request */
    };//class CreateAdminObjectStateRestoreRequestId

}//namespace Fred

#endif//CREATE_ADMIN_OBJECT_STATE_RESTORE_REQUEST_ID_H_
