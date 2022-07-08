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

#include "src/backend/epp/domain/info_domain.hh"
#include "src/backend/epp/domain/impl/domain_output.hh"

#include "src/backend/epp/authorization_required.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/exception.hh"

#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_data.hh"
#include "libfred/zone/zone.hh"
#include "libfred/registrar/info_registrar.hh"

#include <string>

namespace Epp {
namespace Domain {

InfoDomainOutputData info_domain(
        LibFred::OperationContext& _ctx,
        const std::string& _fqdn,
        const InfoDomainConfigData&,
        const Password& _authinfopw,
        const SessionData& _session_data)
{
    if (!is_session_registrar_valid(_session_data))
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    try
    {
        const LibFred::InfoDomainData info_domain_data =
                LibFred::InfoDomainByFqdn(_fqdn)
                        .exec(_ctx, "UTC")
                        .info_domain_data;

        const std::vector<LibFred::ObjectStateData> object_state_data =
            LibFred::GetObjectStates(info_domain_data.id).exec(_ctx);

        if (!_authinfopw->empty())
        {
            authorization_required(
                    _ctx,
                    LibFred::Object::ObjectId{info_domain_data.id},
                    _authinfopw);
        }
        return get_info_domain_output(info_domain_data, object_state_data);
    }
    catch (const LibFred::InfoDomainByFqdn::Exception& e)
    {
        if (e.is_set_unknown_fqdn())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }

        // in the improbable case that exception is incorrectly set
        throw;
    }
}

} // namesapce Epp::Domain
} // namespace Epp
