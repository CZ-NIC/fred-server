/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_KEYSET_HH_38CC1B22FCC84934A2D28948B489EC1E
#define UPDATE_KEYSET_HH_38CC1B22FCC84934A2D28948B489EC1E

#include "src/backend/epp/keyset/update_keyset_input_data.hh"
#include "libfred/opcontext.hh"
#include "src/backend/epp/keyset/update_keyset_config_data.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Keyset {

struct UpdateKeysetResult
{
    unsigned long long id;
    unsigned long long update_history_id;

};

UpdateKeysetResult update_keyset(
        LibFred::OperationContext& _ctx,
        const UpdateKeysetInputData& _update_keyset_input_data,
        const UpdateKeysetConfigData& _update_keyset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Keyset
} // namespace Epp

#endif
