/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/keyset/info_keyset.hh"
#include "src/backend/epp/keyset/impl/keyset_output.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"

#include <string>

namespace Epp {
namespace Keyset {

InfoKeysetOutputData info_keyset(
        LibFred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const InfoKeysetConfigData&,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const LibFred::InfoKeysetData info_keyset_data =
            LibFred::InfoKeysetByHandle(_keyset_handle).exec(_ctx, "UTC").info_keyset_data;

        const std::vector<LibFred::ObjectStateData> keyset_states_data = LibFred::GetObjectStates(info_keyset_data.id).exec(_ctx);

        return get_info_keyset_output(info_keyset_data, keyset_states_data);
    }
    catch (const LibFred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        throw;
    }
}

} // namespace Epp::Keyset
} // namespace Epp
