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
 *  declaration of GetOpenedPublicRequest class
 */

#ifndef GET_OPENED_PUBLIC_REQUEST_HH_DC9C890ACA6A4D5B81372F7FCFA8C864
#define GET_OPENED_PUBLIC_REQUEST_HH_DC9C890ACA6A4D5B81372F7FCFA8C864

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/public_request_object_lock_guard.hh"
#include "src/util/optional_value.hh"

namespace LibFred {

/**
 * Operation for public request creation.
 */
class GetOpenedPublicRequest
{
public:
    typedef ::uint64_t LogRequestId;///< logging request identification
    DECLARE_EXCEPTION_DATA(unknown_type, std::string);///< exception members for bad public request type
    DECLARE_EXCEPTION_DATA(no_request_found, std::string);///< exception members for no request of given type
    struct Exception /// Something wrong happened
    :   virtual LibFred::OperationException,
        ExceptionData_unknown_type< Exception >,
        ExceptionData_no_request_found< Exception >
    {};

    /**
     * Constructor with mandatory parameter.
     * @param _type type of public request
     */
    GetOpenedPublicRequest(const PublicRequestTypeIface &_type);

    ~GetOpenedPublicRequest() { }

    /**
     * Executes searching of given public request.
     * @param _ctx contains reference to database and logging interface
     * @param _locked_object guarantees exclusive access to all public requests of given object
     * @param _log_request_id associated request id in logger
     * @return unique numeric identification of opened public request given type of given object
     * @throw Exception if something wrong happened
     */
    PublicRequestId exec(OperationContext &_ctx,
                         const LockedPublicRequestsOfObject &_locked_object,
                         const Optional< LogRequestId > &_log_request_id = Optional< LogRequestId >())const;
private:
    const std::string type_;
};

} // namespace LibFred

#endif
