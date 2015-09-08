/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  declaration of GetActivePublicRequest class
 */

#ifndef GET_ACTIVE_PUBLIC_REQUEST_H_28ECBCA5E6C221EBD07D31636B2F3AEF//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define GET_ACTIVE_PUBLIC_REQUEST_H_28ECBCA5E6C221EBD07D31636B2F3AEF

#include "src/fredlib/public_request/public_request_type_iface.h"
#include "src/fredlib/public_request/public_request_object_lock_guard.h"
#include "util/optional_value.h"

namespace Fred {

/**
 * Operation for public request creation.
 */
class GetActivePublicRequest
{
public:
    typedef ::uint64_t LogRequestId;///< logging request identification
    DECLARE_EXCEPTION_DATA(unknown_type, std::string);///< exception members for bad public request type
    DECLARE_EXCEPTION_DATA(no_request_found, std::string);///< exception members for no request of given type
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_unknown_type< Exception >,
        ExceptionData_no_request_found< Exception >
    {};

    /**
     * Constructor with mandatory parameter.
     * @param _type type of public request
     */
    GetActivePublicRequest(const PublicRequestTypeIface &_type);

    ~GetActivePublicRequest() { }

    /**
     * Executes searching of given public request.
     * @param _ctx contains reference to database and logging interface
     * @param _locked_object guarantees exclusive access to all public requests of given object
     * @param _log_request_id associated request id in logger
     * @return unique numeric identification of active public request given type of given object
     * @throw Exception if something wrong happened
     */
    PublicRequestId exec(OperationContext &_ctx,
                         const PublicRequestObjectLockGuard &_locked_object,
                         const Optional< LogRequestId > &_log_request_id = Optional< LogRequestId >())const;
private:
    const std::string type_;
};

}//namespace Fred

#endif//GET_ACTIVE_PUBLIC_REQUEST_H_28ECBCA5E6C221EBD07D31636B2F3AEF
