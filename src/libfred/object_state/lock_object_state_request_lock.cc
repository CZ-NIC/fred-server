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

#include "src/libfred/opcontext.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/object_state/lock_object_state_request_lock.hh"


namespace LibFred
{
    LockObjectStateRequestLock::LockObjectStateRequestLock(unsigned long long object_id)
    : object_id_(object_id)
    {}

    void LockObjectStateRequestLock::exec(OperationContext &ctx)
    {
        ctx.get_conn().exec_params("SELECT lock_object_state_request_lock($1::bigint)",
            Database::query_param_list(object_id_));
    }

} // namespace LibFred
