/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  object state request locking
 */

#ifndef LOCK_OBJECT_STATE_REQUEST_LOCK_H_
#define LOCK_OBJECT_STATE_REQUEST_LOCK_H_

#include <string>

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Fred
{
    /**
    * Locks object state using object_state_request_lock table.
    * State database id or name and database id of the object is set via constructors.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class LockObjectStateRequestLock
    {
    public:
        /**
        * Locks object state using object_state_request_lock table constructor with mandatory parameters.
        * @param state_id sets database id of the state @ref state_id_ attribute
        * @param object_id sets database id of the object @ref object_id_ attribute
        */
        LockObjectStateRequestLock(unsigned long long state_id, unsigned long long object_id);
        /**
        * Locks object state using object_state_request_lock table constructor with mandatory parameters.
        * @param state_name sets name of the state into @ref state_name_ attribute
        * @param object_id sets database id of the object @ref object_id_ attribute
        */
        LockObjectStateRequestLock(const std::string& state_name, unsigned long long object_id);
        /**
        * Executes object state lock using object_state_request_lock table constructor.
        * @param ctx contains reference to database and logging interface
        * @return true if object exists and have the state set, false if not
        */
        void exec(OperationContext &_ctx);
    private:
        unsigned long long state_id_;/**< database id of the state */
        Optional<std::string> state_name_;/**< name of the state */
        const unsigned long long object_id_;/**< database id of the object */
    };

}//namespace Fred

#endif // LOCK_OBJECT_STATE_REQUEST_LOCK_H_
