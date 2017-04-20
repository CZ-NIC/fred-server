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

#include "src/epp/domain/info_domain.h"
#include "src/epp/domain/impl/domain_output.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/info_domain_data.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <string>

namespace Epp {
namespace Domain {

InfoDomainOutputData info_domain(
        Fred::OperationContext& _ctx,
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
        const Fred::InfoDomainData info_domain_data = Fred::InfoDomainByHandle(
                Fred::Zone::rem_trailing_dot(_fqdn)).exec(_ctx, "UTC").info_domain_data;

        const std::string session_registrar_handle =
            Fred::InfoRegistrarById(_session_data.registrar_id).exec(_ctx).info_registrar_data.handle;
        const bool info_is_for_sponsored_registrar =
            info_domain_data.sponsoring_registrar_handle == session_registrar_handle;

        const std::vector<Fred::ObjectStateData> object_state_data =
            Fred::GetObjectStates(info_domain_data.id).exec(_ctx);

        return get_info_domain_output(info_domain_data, object_state_data, info_is_for_sponsored_registrar);
    }
    catch (const Fred::InfoDomainByHandle::Exception& e)
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
