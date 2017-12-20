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
 *  object state check
 */

#ifndef OBJECT_HAS_STATE_H_
#define OBJECT_HAS_STATE_H_

#include "src/libfred/opexception.hh"

namespace LibFred
{

    /**
    * Check if given object has set given state.
    * Locks object using object_state_request_lock table. State is not found if object was deleted.
    * Object database id and name of the state is set via constructor.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * In case of insuperable failures and inconsistencies, the instance of @ref InternalError or other exception is thrown.
    */
    class ObjectHasState
    {
    public:
        /**
        * Check object for the state constructor with mandatory parameters.
        * @param object_id sets database id of the object @ref object_id_ attribute
        * @param state_name sets name of the state into @ref state_name_ attribute
        */
        ObjectHasState(unsigned long long object_id, const std::string& state_name);
        /**
        * Executes check of the object for the state.
        * @param ctx contains reference to database and logging interface
        * @return true if object exists and have the state set, false if not
        */
        bool exec(OperationContext& ctx);
    private:
        const unsigned long long object_id_;/**< database id of the object */
        const std::string state_name_;/**< name of the state */
    };

} // namespace LibFred

#endif // OBJECT_HAS_STATE_H_
