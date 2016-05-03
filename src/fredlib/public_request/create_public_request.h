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
 *  declaration of CreatePublicRequest class
 */

#ifndef CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9

#include "src/fredlib/public_request/public_request_type_iface.h"
#include "src/fredlib/public_request/public_request_object_lock_guard.h"
#include "util/optional_value.h"

namespace Fred {

/**
 * Operation for public request creation.
 */
class CreatePublicRequest
{
public:
    typedef ::uint64_t LogRequestId;///< logging request identification
    DECLARE_EXCEPTION_DATA(unknown_type, std::string);///< exception members for bad public request type
    DECLARE_EXCEPTION_DATA(unknown_registrar_id, RegistrarId);///< exception members for bad registrar id
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_unknown_type< Exception >,
        ExceptionData_unknown_registrar_id< Exception >
    {};

    /**
     * Constructor with mandatory parameter.
     * @param _type type of public request
     */
    CreatePublicRequest(const PublicRequestTypeIface &_type);

    /**
     * Constructor with all parameters.
     * @param _type type of public request
     * @param _reason reason of public request creation
     * @param _email_to_answer the answer recipient's email address
     * @param _registrar_id I don't know relationship between this registrar and public request!
     */
    CreatePublicRequest(const PublicRequestTypeIface &_type,
                        const Optional< std::string > &_reason,
                        const Optional< std::string > &_email_to_answer,
                        const Optional< RegistrarId > &_registrar_id);
    ~CreatePublicRequest() { }

    /**
     * Sets reason of public request creation.
     * @param _reason sets reason of public request creation
     * @return operation instance reference to allow method chaining
     */
    CreatePublicRequest& set_reason(const std::string &_reason);

    /**
     * Sets email address of answer recipient's.
     * @param _email sets email address of answer recipient's
     * @return operation instance reference to allow method chaining
     */
    CreatePublicRequest& set_email_to_answer(const std::string &_email);

    /**
     * Sets id of registrar.
     * @param _id sets registrar id
     * @return operation instance reference to allow method chaining
     */
    CreatePublicRequest& set_registrar_id(RegistrarId _id);

    /**
     * Executes creation.
     * @param _locked_object guarantees exclusive access to all public requests of given object
     * @param _create_log_request_id associated request id in logger
     * @return unique numeric identification of just created public request
     * @throw Exception if something wrong happened
     */
    PublicRequestId exec(const LockedPublicRequestsOfObjectForUpdate &_locked_object,
                         const Optional< LogRequestId > &_create_log_request_id = Optional< LogRequestId >())const;

    /**
     * Invalidates my opened requests of the same type.
     * @param _type type of public request
     * @param _locked_object guarantees exclusive access to all public requests of given object
     * @param _registrar_id registrar which calls this function
     * @param _log_request_id associated request id in logger
     * @return the number of invalidated public requests
     * @throw Exception if something wrong happened
     */
    static ::size_t invalidate_the_same(const std::string &_type,
                                        const LockedPublicRequestsOfObjectForUpdate &_locked_object,
                                        const Optional< RegistrarId > _registrar_id = Optional< RegistrarId >(),
                                        const Optional< LogRequestId > &_log_request_id = Optional< LogRequestId >());
private:
    const std::string type_;
    Optional< std::string > reason_;
    Optional< std::string > email_to_answer_;
    Optional< RegistrarId > registrar_id_;
};

}//namespace Fred

#endif//CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9