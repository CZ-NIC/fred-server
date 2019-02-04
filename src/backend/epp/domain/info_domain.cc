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

#include "src/backend/epp/domain/info_domain.hh"
#include "src/backend/epp/domain/impl/domain_output.hh"

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
        const InfoDomainConfigData& _info_domain_config_data,
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

        const std::string session_registrar_handle =
            LibFred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle;
        const bool info_is_for_sponsoring_registrar =
            info_domain_data.sponsoring_registrar_handle == session_registrar_handle;

        const std::vector<LibFred::ObjectStateData> object_state_data =
            LibFred::GetObjectStates(info_domain_data.id).exec(_ctx);

        return get_info_domain_output(info_domain_data, object_state_data, info_is_for_sponsoring_registrar);
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
