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
 *  declaration of CreatePublicRequestAuth class
 */

#ifndef CREATE_PUBLIC_REQUEST_AUTH_H_D034201C95EC2E9FF787F4CBF17E7802//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_PUBLIC_REQUEST_AUTH_H_D034201C95EC2E9FF787F4CBF17E7802

#include "src/fredlib/public_request/public_request_auth_type_iface.h"
#include "src/fredlib/public_request/public_request_object_lock_guard.h"
#include "util/optional_value.h"

namespace Fred {

enum { PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH = 32 };///< length of string for identification of public request with authentication

/**
 * Operation for creation of public request with authentication.
 */
class CreatePublicRequestAuth
{
public:
    DECLARE_EXCEPTION_DATA(unknown_type, std::string);///< exception members for bad public request type
    DECLARE_EXCEPTION_DATA(unknown_registrar_id, RegistrarId);///< exception members for bad registrar id
    DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);///< exception members for bad registrar handle
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_unknown_type< Exception >,
        ExceptionData_unknown_registrar_id< Exception >,
        ExceptionData_unknown_registrar_handle< Exception >
    {};

    /**
     * Constructor with mandatory parameter.
     * @param _type type of public request
     */
    CreatePublicRequestAuth(const PublicRequestAuthTypeIface &_type);

    /**
     * Constructor with all parameters.
     * @param _type type of public request
     * @param _reason reason of public request creation
     * @param _email_to_answer the answer recipient's email address
     * @param _registrar_id I don't know relationship between this registrar and public request!
     */
    CreatePublicRequestAuth(const PublicRequestAuthTypeIface &_type,
                            const Optional< std::string > &_reason,
                            const Optional< std::string > &_email_to_answer,
                            const Optional< RegistrarId > &_registrar_id);
    ~CreatePublicRequestAuth() { }

    /**
     * Sets reason of public request creation.
     * @param _reason sets reason of public request creation
     * @return operation instance reference to allow method chaining
     */
    CreatePublicRequestAuth& set_reason(const std::string &_reason);

    /**
     * Sets email address for sending answers of public request creation.
     * @param _email sets email address of answer recipient's
     * @return operation instance reference to allow method chaining
     */
    CreatePublicRequestAuth& set_email_to_answer(const std::string &_email);

    /**
     * Sets registrar id of public request creation.
     * @param _id sets registrar id
     * @return operation instance reference to allow method chaining
     */
    CreatePublicRequestAuth& set_registrar_id(RegistrarId _id);

    /**
     * Sets registrar id of public request creation.
     * @param _ctx contains reference to database and logging interface
     * @param _registrar_handle sets registrar id of registrar with this handle
     * @return operation instance reference to allow method chaining
     */
    CreatePublicRequestAuth& set_registrar_id(OperationContext &_ctx, const std::string &_registrar_handle);

    /**
     * Result of creation operation.
     */
    struct Result
    {
        Result() { }
        Result(const Result &_src);
        Result& operator=(const Result &_src);
        PublicRequestId public_request_id;///< unique numeric identification of just created public request
        std::string identification;///< unique string identification of just created public request
        std::string password;///< authentication string of just created public request
    };

    /**
     * Executes creation.
     * @param _ctx contains reference to database and logging interface
     * @param _locked_object guarantees exclusive access to all public requests of given object
     * @return @ref Result object corresponding with performed operation
     * @throw Exception if something wrong happened
     */
    Result exec(OperationContext &_ctx, const PublicRequestObjectLockGuard &_locked_object)const;
private:
    const std::string type_;
    const std::string password_;
    Optional< std::string > reason_;
    Optional< std::string > email_to_answer_;
    Optional< RegistrarId > registrar_id_;
};

}//namespace Fred

#endif//CREATE_PUBLIC_REQUEST_AUTH_H_D034201C95EC2E9FF787F4CBF17E7802
