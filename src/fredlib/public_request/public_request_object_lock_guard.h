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
 *  declaration of PublicRequestObjectLockGuardByObjectId class
 */

#ifndef PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F

#include "src/fredlib/object_state/typedefs.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"

namespace Fred {

/**
 * Common class guaranteeing exclusive read access to the public requests of given object.
 */
class LockedPublicRequestsOfObject
{
public:
    /**
     * Returns unique numeric id of object which public requests are locked.
     * @return object id
     */
    ObjectId get_id()const { return object_id_; }
protected:
    LockedPublicRequestsOfObject(ObjectId _object_id):object_id_(_object_id) { }
    ~LockedPublicRequestsOfObject() { }
private:
    LockedPublicRequestsOfObject(const LockedPublicRequestsOfObject&);
    LockedPublicRequestsOfObject& operator=(const LockedPublicRequestsOfObject&);
    const ObjectId object_id_;
};

/**
 * Common class guaranteeing exclusive r/w access to the public requests of given object.
 */
class LockedPublicRequestsOfObjectForUpdate
{
public:
    /**
     * Converts to the read only accessor.
     * @return reference to the read only accessor
     */
    operator const LockedPublicRequestsOfObject&()const { return locked_public_requests_; }
    /**
     * Returns unique numeric id of object with locked public requests.
     * @return object id
     */
    ObjectId get_id()const { return locked_public_requests_.get_id(); }
    /**
     * Returns operation context which has been used for the public requests locking.
     * @return reference to the operation context
     */
    OperationContext& get_ctx()const { return ctx_; }
protected:
    LockedPublicRequestsOfObjectForUpdate(OperationContext &_ctx, ObjectId _object_id)
    :   locked_public_requests_(_object_id),
        ctx_(_ctx)
    { }
    ~LockedPublicRequestsOfObjectForUpdate() { }
private:
    LockedPublicRequestsOfObjectForUpdate(const LockedPublicRequestsOfObjectForUpdate&);
    LockedPublicRequestsOfObjectForUpdate& operator=(const LockedPublicRequestsOfObjectForUpdate&);
    class MyLockedPublicRequestsOfObject:public LockedPublicRequestsOfObject
    {
    public:
        MyLockedPublicRequestsOfObject(ObjectId _object_id):LockedPublicRequestsOfObject(_object_id) { }
    };
    const MyLockedPublicRequestsOfObject locked_public_requests_;
    OperationContext &ctx_;
};

/**
 * Obtain exclusive access to all public requests of given object.
 * @warning Destructor doesn't release lock contrary to expectations. The one will release by finishing of
 *          transaction wherein it was created.
 */
class PublicRequestsOfObjectLockGuardByObjectId
{
public:
    DECLARE_EXCEPTION_DATA(object_doesnt_exist, ObjectId);///< exception members for bad object_id
    struct Exception /// Something wrong happened
    :   virtual Fred::OperationException,
        ExceptionData_object_doesnt_exist< Exception >
    {};
    /**
     * Obtain exclusive access to all public requests on object identified by _object_id. Operation context
     * _ctx can manipulate public request data from now until this transaction will finish.
     * @param _ctx use database connection from this operation context
     * @param _object_id unique numeric identification of object
     */
    PublicRequestsOfObjectLockGuardByObjectId(OperationContext &_ctx, ObjectId _object_id);
    /**
     * Converts to the read only accessor.
     * @return reference to the read only accessor
     */
    operator const LockedPublicRequestsOfObject&()const { return locked_public_requests_for_update_; }
    /**
     * Converts to the r/w accessor.
     * @return reference to the r/w accessor
     */
    operator const LockedPublicRequestsOfObjectForUpdate&()const { return locked_public_requests_for_update_; }
private:
    class MyLockedPublicRequestsOfObjectForUpdate:public LockedPublicRequestsOfObjectForUpdate
    {
    public:
        MyLockedPublicRequestsOfObjectForUpdate(OperationContext &_ctx, ObjectId _object_id)
        :   LockedPublicRequestsOfObjectForUpdate(_ctx, _object_id) { }
    };
    const MyLockedPublicRequestsOfObjectForUpdate locked_public_requests_for_update_;
};

}//namespace Fred

#endif//PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F
