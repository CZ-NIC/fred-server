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

#include "src/backend/epp/credit/client_credit.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include "libfred/registrar/get_registrar_zone_credit.hh"
#include "libfred/registrar/info_registrar.hh"

namespace Epp {
namespace Credit {

namespace {

std::string get_registrar_handle_by_id(LibFred::OperationContext& ctx, unsigned long long registrar_id)
{
    return LibFred::InfoRegistrarById(registrar_id).exec(ctx).info_registrar_data.handle;
}

ZoneCredit fred_to_epp(const LibFred::ZoneCredit& src)
{
    if (src.has_credit())
    {
        return ZoneCredit(src.get_zone_fqdn(), src.get_credit());
    }
    return ZoneCredit(src.get_zone_fqdn());
}

} // namespace Epp::Credit::{anonymous}

ClientCreditOutputData client_credit(
        LibFred::OperationContext& _ctx,
        unsigned long long _registrar_id)
{
    const bool registrar_is_authenticated = _registrar_id != 0;
    if (!registrar_is_authenticated)
    {
        throw EppResponseFailure(EppResultFailure(
                EppResultCode::authentication_error_server_closing_connection));
    }

    const std::string registrar_handle = get_registrar_handle_by_id(_ctx, _registrar_id);
    const LibFred::RegistrarZoneCredit credit_by_zones =
            LibFred::GetRegistrarZoneCredit().exec(_ctx, registrar_handle);

    ClientCreditOutputData result;
    for (LibFred::RegistrarZoneCredit::const_iterator src_ptr = credit_by_zones.begin();
         src_ptr != credit_by_zones.end(); ++src_ptr)
    {
        result.insert(fred_to_epp(*src_ptr));
    }
    return result;
}

} // namespace Epp::Credit
} // namespace Epp
