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

#include "src/backend/epp/nsset/check_nsset.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/nsset/nsset_handle_state_to_check_result.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"

#include <boost/foreach.hpp>

namespace Epp {
namespace Nsset {

std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > check_nsset(
        LibFred::OperationContext& _ctx,
        const std::set<std::string>& _nsset_handles,
        const CheckNssetConfigData& _check_nsset_config_data,
        const SessionData& _session_data)
{

    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > result;

    BOOST_FOREACH(const std::string & handle, _nsset_handles) {
        result[handle] = nsset_handle_state_to_check_result(
                LibFred::Nsset::get_handle_syntax_validity(_ctx, handle),
                LibFred::Nsset::get_handle_registrability(_ctx, handle)
        );
    }

    return result;
}


} // namespace Epp::Nsset
} // namespace Epp
