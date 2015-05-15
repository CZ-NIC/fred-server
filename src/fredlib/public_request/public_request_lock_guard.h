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
 *  declaration of PublicRequestLockGuard class family
 */

#ifndef PUBLIC_REQUEST_LOCK_GUARD_H_F77FFFF66AD58705219B2B82AA1969FA//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_LOCK_GUARD_H_F77FFFF66AD58705219B2B82AA1969FA

#include "src/fredlib/object_state/typedefs.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"

namespace Fred {

/**
 * Common class guaranteeing exclusive access to public request data.
 * @note It would be suitable to remember reference to operation context object and make this reference
 *       accessible too because this guard joins locked entity with particular database connection.
 */
class PublicRequestLockGuard
{
public:
    /**
     * Return numeric id of guarded public request.
     * @return id of guarded public request
     */
    PublicRequestId get_public_request_id()const { return public_request_id_; }
protected:
    /**
     * Derived class can store public_request_id.
     * @param _public_request_id value to store
     */
    PublicRequestLockGuard(PublicRequestId _public_request_id):public_request_id_(_public_request_id) { }
    /**
     * It should unlock public request in derived class but ...
     */
    ~PublicRequestLockGuard() { }
private:
    const PublicRequestId public_request_id_;
};

/**
 * Obtain exclusive access to public request data identified by string identification.
 */
class PublicRequestLockGuardByIdentification:public PublicRequestLockGuard
{
public:
    DECLARE_EXCEPTION_DATA(public_request_doesnt_exist, std::string);///< exception members for unknown identification
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_public_request_doesnt_exist< Exception >
    {};
    /**
     * Obtain exclusive access to public request identified by _identification. Operation context _ctx
     * can manipulate public request data from now until this transaction will finish.
     * @param _ctx use database connection from this operation context
     * @param _identification unique string identification of public request with authentication
     */
    PublicRequestLockGuardByIdentification(OperationContext &_ctx, const std::string &_identification);
    /**
     * @warning It doesn't release lock contrary to expectations. The one will release by finishing of
     *          transaction wherein it was created.
     */
    ~PublicRequestLockGuardByIdentification() { }
};

/**
 * Obtain exclusive access to public request data identified by string identification.
 */
class PublicRequestLockGuardById:public PublicRequestLockGuard
{
public:
    DECLARE_EXCEPTION_DATA(public_request_doesnt_exist, PublicRequestId);///< exception members for unknown id
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_public_request_doesnt_exist< Exception >
    {};
    /**
     * Obtain exclusive access to public request identified by _id. Operation context _ctx
     * can manipulate public request data from now until this transaction will finish.
     * @param _ctx use database connection from this operation context
     * @param _id unique numeric identification of public request
     */
    PublicRequestLockGuardById(OperationContext &_ctx, PublicRequestId _id);
    /**
     * @warning It doesn't release lock contrary to expectations. The one will release by finishing of
     *          transaction wherein it was created.
     */
    ~PublicRequestLockGuardById() { }
};

}//namespace Fred

#endif//PUBLIC_REQUEST_LOCK_GUARD_H_F77FFFF66AD58705219B2B82AA1969FA
