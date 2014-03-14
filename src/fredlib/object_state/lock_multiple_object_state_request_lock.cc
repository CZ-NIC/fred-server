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
 *  object multiple state request locking
 */

#include <string>
#include <set>
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "lock_multiple_object_state_request_lock.h"


namespace Fred
{
    LockMultipleObjectStateRequestLock::LockMultipleObjectStateRequestLock(
        const std::set<unsigned long long> &_state_id, unsigned long long _object_id)
    :   state_id_(_state_id),
        object_id_(_object_id)
    {}

    void LockMultipleObjectStateRequestLock::exec(OperationContext &_ctx)
    {
        typedef std::auto_ptr< Database::StandaloneConnection > StandaloneConnectionPtr;
        Database::StandaloneManager sm = Database::StandaloneManager(
            new Database::StandaloneConnectionFactory(Database::Manager::getConnectionString()));
        StandaloneConnectionPtr conn_standalone(sm.acquire());
        for (std::set<unsigned long long>::const_iterator pStateId = state_id_.begin(); pStateId != state_id_.end(); ++pStateId) {
            Database::query_param_list param(*pStateId);
            param(object_id_);
            conn_standalone->exec_params("INSERT INTO object_state_request_lock (state_id,object_id) "
                                         "VALUES ($1::bigint,$2::bigint)", param);
            _ctx.get_conn().exec_params("SELECT lock_object_state_request_lock($1::bigint,$2::bigint)", param);
        }
    }

}//namespace Fred
