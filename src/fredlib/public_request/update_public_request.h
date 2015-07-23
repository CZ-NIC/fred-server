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
 *  declaration of UpdatePublicRequest class
 */

#ifndef UPDATE_PUBLIC_REQUEST_H_9F964452619CB937F93C2B144C8A204D//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define UPDATE_PUBLIC_REQUEST_H_9F964452619CB937F93C2B144C8A204D

#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Fred {

/**
 * Operation for public request update.
 */
class UpdatePublicRequest
{
public:
    typedef boost::posix_time::ptime Time;///< class for time representation
    typedef ObjectId EmailId;///< email database identification
    typedef ObjectId RequestId;///< some request identification
    DECLARE_EXCEPTION_DATA(nothing_to_do, PublicRequestId);///< exception members in case of all items empty
    DECLARE_EXCEPTION_DATA(public_request_doesnt_exist, PublicRequestId);///< exception members in case of bad public_request_id
    DECLARE_EXCEPTION_DATA(unknown_email_id, EmailId);///< exception members in case of bad answer_email_id
    DECLARE_EXCEPTION_DATA(unknown_registrar_id, RegistrarId);///< exception members in case of bad registrar_id
    DECLARE_EXCEPTION_DATA(bad_public_request_status, PublicRequest::Status::Value);///< exception members in case of invalid value of status
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_nothing_to_do< Exception >,
        ExceptionData_public_request_doesnt_exist< Exception >,
        ExceptionData_unknown_email_id< Exception >,
        ExceptionData_unknown_registrar_id< Exception >,
        ExceptionData_bad_public_request_status< Exception >
    {};

    /**
     * Constructor without parameters.
     */
    UpdatePublicRequest();

    /**
     * Constructor with all parameters.
     * @param _status can set public request status
     * @param _resolve_time can set time of public request resolving
     * @param _reason can set reason of public request creation
     * @param _email_to_answer can set the answer recipient's email address
     * @param _answer_email_id can set the email id
     * @param _registrar_id can set but I don't know relationship between this registrar and public request!
     * @param _create_request_id can set create_request_id whatever it means
     * @param _resolve_request_id can set resolve_request_id whatever it means
     */
    UpdatePublicRequest(const Optional< PublicRequest::Status::Value > &_status,
                        const Optional< Nullable< Time > > &_resolve_time,
                        const Optional< Nullable< std::string > > &_reason,
                        const Optional< Nullable< std::string > > &_email_to_answer,
                        const Optional< Nullable< EmailId > > &_answer_email_id,
                        const Optional< Nullable< RegistrarId > > &_registrar_id,
                        const Optional< Nullable< RequestId > > &_create_request_id,
                        const Optional< Nullable< RequestId > > &_resolve_request_id);
    ~UpdatePublicRequest() { }

    /**
     * Sets status of public request.
     * @param _status sets status of public request
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_status(PublicRequest::Status::Value _status);

    /**
     * Sets time of public request resolving.
     * @param _time time of public request resolving
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_resolve_time(const Nullable< Time > &_time);

    /**
     * Sets time of public request resolving to current time.
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_resolve_time_to_now();

    /**
     * Sets reason of last public request operation.
     * @param _reason reason of last public request operation
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_reason(const Nullable< std::string > &_reason);

    /**
     * Sets email address of answer recipient's.
     * @param _email email address of answer recipient's
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_email_to_answer(const Nullable< std::string > &_email);

    /**
     * Sets email id of answer.
     * @param _id email id of answer
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_answer_email_id(const Nullable< EmailId > &_id);

    /**
     * Sets id of registrar.
     * @param _id registrar id
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_registrar_id(const Nullable< RegistrarId > _id);

    /**
     * Sets create_request_id.
     * @param _id create_request_id
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_create_request_id(const Nullable< RequestId > &_id);

    /**
     * Sets resolve_request_id.
     * @param _id resolve_request_id
     * @return operation instance reference to allow method chaining
     */
    UpdatePublicRequest& set_resolve_request_id(const Nullable< RequestId > &_id);

    /**
     * Result of update operation.
     */
    struct Result
    {
        Result() { }
        Result(const Result &_src);
        Result& operator=(const Result &_src);
        PublicRequestId public_request_id;///< unique numeric identification of this public request
        std::string public_request_type;///< string representation of public request type
        ObjectId object_id;///< id of object associated with this public request
    };

    /**
     * Executes update.
     * @param _ctx contains reference to database and logging interface
     * @param _locked_public_request guarantees exclusive access to public request data
     * @return @ref Result object corresponding with performed operation
     * @throw Exception if something wrong happened
     */
    Result exec(OperationContext &_ctx, const PublicRequestLockGuard &_locked_public_request)const;
private:
    Optional< PublicRequest::Status::Value > status_;
    Optional< Nullable< Time > > resolve_time_;
    bool is_resolve_time_set_to_now_;
    Optional< Nullable< std::string > > reason_;
    Optional< Nullable< std::string > > email_to_answer_;
    Optional< Nullable< EmailId > > answer_email_id_;
    Optional< Nullable< RegistrarId > > registrar_id_;
    Optional< Nullable< RequestId > > create_request_id_;
    Optional< Nullable< RequestId > > resolve_request_id_;
};

}//namespace Fred

#endif//UPDATE_PUBLIC_REQUEST_H_9F964452619CB937F93C2B144C8A204D
