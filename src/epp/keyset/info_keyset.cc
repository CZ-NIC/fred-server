/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/keyset/info_keyset.h"
#include "src/epp/keyset/impl/keyset_output.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/fredlib/keyset/info_keyset.h"

#include <string>

namespace Epp {
namespace Keyset {

InfoKeysetOutputData info_keyset(
        Fred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const InfoKeysetConfigData& _info_keyset_config_data,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const Fred::InfoKeysetData info_keyset_data =
            Fred::InfoKeysetByHandle(_keyset_handle).exec(_ctx, "UTC").info_keyset_data;

        return get_info_keyset_output(_ctx, info_keyset_data, _session_data.registrar_id);
    }
    catch (const Fred::InfoKeysetByHandle::Exception& e)
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
