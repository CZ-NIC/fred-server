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
    LockMultipleObjectStateRequestLock::LockMultipleObjectStateRequestLock(unsigned long long _object_id)
    :   object_id_(_object_id)
    {}

    void LockMultipleObjectStateRequestLock::exec(OperationContext &_ctx)
    {
        _ctx.get_conn().exec_params("SELECT lock_object_state_request_lock($1::bigint)", Database::query_param_list(object_id_));
    }

}//namespace Fred
