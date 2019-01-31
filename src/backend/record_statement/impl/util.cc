/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "src/backend/record_statement/impl/util.hh"

#include "libfred/object/object_state.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/opcontext.hh"

namespace Fred {
namespace Backend {
namespace RecordStatement {
namespace Impl {

bool is_delete_candidate(LibFred::OperationContext& _ctx, unsigned long long _object_id)
{
    const LibFred::ObjectStatesInfo object_states(LibFred::GetObjectStates(_object_id).exec(_ctx));
    return object_states.presents(LibFred::Object_State::delete_candidate);
}

} // namespace Fred::Backend::RecordStatement::Impl
} // namespace Fred::Backend::RecordStatement
} // namespace Fred::Backend
} // namespace Fred
