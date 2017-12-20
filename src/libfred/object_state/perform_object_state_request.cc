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
 *  perform object state request
 */

#include <string>

#include "src/libfred/opcontext.hh"
#include "src/libfred/object_state/lock_object_state_request_lock.hh"
#include "src/libfred/object_state/perform_object_state_request.hh"

namespace LibFred
{
    PerformObjectStateRequest::PerformObjectStateRequest()
    {}

    PerformObjectStateRequest::PerformObjectStateRequest(const Optional< unsigned long long > &_object_id)
    :   object_id_(_object_id)
    {}

    PerformObjectStateRequest& PerformObjectStateRequest::set_object_id(unsigned long long _object_id)
    {
        object_id_ = _object_id;
        return *this;
    }

    void PerformObjectStateRequest::exec(OperationContext &_ctx)
    {
        _ctx.get_conn().exec_params(
            "SELECT update_object_states($1::integer)",
            Database::query_param_list
                (object_id_));
    }

} // namespace LibFred
