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
 * Common class guaranteeing exclusive read access to the data of given public request.
 */
class LockedPublicRequest
{
public:
    /**
     * Returns unique numeric id of locked public request.
     * @return id of locked public request
     */
    PublicRequestId get_id()const { return public_request_id_; }
protected:
    LockedPublicRequest(PublicRequestId _public_request_id):public_request_id_(_public_request_id) { }
    ~LockedPublicRequest() { }
private:
    LockedPublicRequest(const LockedPublicRequest&);
    LockedPublicRequest& operator=(const LockedPublicRequest&);
    const PublicRequestId public_request_id_;
};

/**
 * Common class guaranteeing exclusive r/w access to the data of given public request.
 */
class LockedPublicRequestForUpdate
{
public:
    /**
     * Converts to the read only accessor.
     * @return reference to the read only accessor
     */
    operator const LockedPublicRequest&()const { return locked_public_request_; }
    /**
     * Returns unique numeric id of locked public request.
     * @return id of locked public request
     */
    PublicRequestId get_id()const { return locked_public_request_.get_id(); }
    /**
     * Returns operation context which has been used for the public request locking.
     * @return reference to the operation context
     */
    OperationContext& get_ctx()const { return ctx_; }
protected:
    LockedPublicRequestForUpdate(OperationContext &_ctx, PublicRequestId _public_request_id)
    :   locked_public_request_(_public_request_id),
        ctx_(_ctx)
    { }
    ~LockedPublicRequestForUpdate() { }
private:
    LockedPublicRequestForUpdate(const LockedPublicRequestForUpdate&);
    LockedPublicRequestForUpdate& operator=(const LockedPublicRequestForUpdate&);
    class MyLockedPublicRequest:public LockedPublicRequest
    {
    public:
        MyLockedPublicRequest(PublicRequestId _public_request_id):LockedPublicRequest(_public_request_id) { }
    };
    const MyLockedPublicRequest locked_public_request_;
    OperationContext &ctx_;
};

/**
 * Obtain exclusive access to public request data identified by string identification.
 */
class PublicRequestLockGuardByIdentification
{
public:
    DECLARE_EXCEPTION_DATA(public_request_doesnt_exist, std::string);///< exception members for unknown identification
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_public_request_doesnt_exist< Exception >
    {};
    /**
     * Obtain exclusive access to the public request identified by _identification. Operation context _ctx
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
    /**
     * Converts to the read only accessor.
     * @return reference to the read only accessor
     */
    operator const LockedPublicRequest&()const { return locked_public_request_for_update_; }
    /**
     * Converts to the r/w accessor.
     * @return reference to the r/w accessor
     */
    operator const LockedPublicRequestForUpdate&()const { return locked_public_request_for_update_; }
private:
    class MyLockedPublicRequestForUpdate:public LockedPublicRequestForUpdate
    {
    public:
        MyLockedPublicRequestForUpdate(OperationContext &_ctx, PublicRequestId _public_request_id)
        :   LockedPublicRequestForUpdate(_ctx, _public_request_id) { }
    };
    const MyLockedPublicRequestForUpdate locked_public_request_for_update_;
};

/**
 * Obtain exclusive access to public request data identified by numeric identification.
 */
class PublicRequestLockGuardById
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
    /**
     * Converts to the read only accessor.
     * @return reference to the read only accessor
     */
    operator const LockedPublicRequest&()const { return locked_public_request_for_update_; }
    /**
     * Converts to the r/w accessor.
     * @return reference to the r/w accessor
     */
    operator const LockedPublicRequestForUpdate&()const { return locked_public_request_for_update_; }
private:
    class MyLockedPublicRequestForUpdate:public LockedPublicRequestForUpdate
    {
    public:
        MyLockedPublicRequestForUpdate(OperationContext &_ctx, PublicRequestId _public_request_id)
        :   LockedPublicRequestForUpdate(_ctx, _public_request_id) { }
    };
    const MyLockedPublicRequestForUpdate locked_public_request_for_update_;
};

}//namespace Fred

#endif//PUBLIC_REQUEST_LOCK_GUARD_H_F77FFFF66AD58705219B2B82AA1969FA
