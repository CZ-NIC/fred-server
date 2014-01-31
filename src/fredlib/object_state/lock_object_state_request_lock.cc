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

#include <string>
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "lock_object_state_request_lock.h"


namespace Fred
{
    LockObjectStateRequestLock::LockObjectStateRequestLock(unsigned long long state_id, unsigned long long object_id)
    :   state_id_(state_id),
        object_id_(object_id)
    {}

    LockObjectStateRequestLock::LockObjectStateRequestLock(const std::string& state_name, unsigned long long object_id)
    :   state_name_(state_name),
        object_id_(object_id)
    {}

    void LockObjectStateRequestLock::exec(OperationContext &ctx)
    {
        if(state_name_.isset())
        {
            Database::Result res_state_id = ctx.get_conn().exec_params("SELECT id FROM enum_object_states "
                " WHERE name=$1::text ", Database::query_param_list (state_name_.get_value()));

            if(res_state_id.size() == 1)
            {
                state_id_ = static_cast<unsigned long long>(res_state_id[0][0]);
            }
            else
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("unknown object state name"));
            }
        }

        {//insert separately
            typedef std::auto_ptr< Database::StandaloneConnection > StandaloneConnectionPtr;
            Database::StandaloneManager sm = Database::StandaloneManager(
                new Database::StandaloneConnectionFactory(Database::Manager::getConnectionString()));
            StandaloneConnectionPtr conn_standalone(sm.acquire());
            conn_standalone->exec_params(
                "INSERT INTO object_state_request_lock (id,state_id,object_id) "
                "VALUES (DEFAULT, $1::bigint, $2::bigint)",
                Database::query_param_list(state_id_)(object_id_));
        }

        ctx.get_conn().exec_params("SELECT lock_object_state_request_lock($1::bigint, $2::bigint)",
            Database::query_param_list(state_id_)(object_id_));
    }

}//namespace Fred
