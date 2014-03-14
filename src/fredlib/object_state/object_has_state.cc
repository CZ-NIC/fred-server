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

#include <string>

#include "src/fredlib/opcontext.h"
#include "lock_object_state_request_lock.h"
#include "object_has_state.h"

namespace Fred
{
    ObjectHasState::ObjectHasState(unsigned long long object_id, const std::string& state_name)
        : object_id_(object_id),
        state_name_(state_name)
    {}

    bool ObjectHasState::exec(OperationContext &ctx)
    {
        LockObjectStateRequestLock(state_name_, object_id_).exec(ctx);

        Database::Result rcheck = ctx.get_conn().exec_params(
            "SELECT count(*) FROM object_state os"
            " JOIN enum_object_states eos ON eos.id = os.state_id"
            " WHERE os.object_id = $1::integer AND eos.name = $2::text"
            " AND valid_to IS NULL",
            Database::query_param_list
                (object_id_)
                (state_name_));
        return static_cast<int>(rcheck[0][0]);
    }

}//namespace Fred
