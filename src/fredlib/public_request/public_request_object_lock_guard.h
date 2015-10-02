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
 *  declaration of PublicRequestObjectLockGuard class
 */

#ifndef PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F

#include "src/fredlib/object_state/typedefs.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"

namespace Fred {

/**
 * Common class guaranteeing exclusive access to all public requests of given object.
 * @warning Destructor doesn't release lock contrary to expectations. The one will release by finishing of
 *          transaction wherein it was created.
 * @note It would be suitable to remember reference to operation context object and make this reference
 *       accessible too because this guard joins locked entity with particular database connection.
 */
class PublicRequestObjectLockGuard
{
public:
    /**
     * Return object's numeric id whose public requests guards.
     * @return object's numeric id whose public requests guards
     */
    ObjectId get_object_id()const { return object_id_; }
protected:
    /**
     * Derived class can store object_id.
     * @param _object_id unique numeric identification of object
     */
    PublicRequestObjectLockGuard(ObjectId _object_id):object_id_(_object_id) { }
private:
    const ObjectId object_id_;
};

/**
 * Obtain exclusive access to all public requests of given object.
 * @warning Destructor doesn't release lock contrary to expectations. The one will release by finishing of
 *          transaction wherein it was created.
 * @note It would be suitable to remember reference to operation context object and make this reference
 *       accessible too because this guard joins locked entity with particular database connection.
 */
class PublicRequestObjectLockGuardByObjectId:public PublicRequestObjectLockGuard
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
    PublicRequestObjectLockGuardByObjectId(OperationContext &_ctx, ObjectId _object_id);
};

}//namespace Fred

#endif//PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F
