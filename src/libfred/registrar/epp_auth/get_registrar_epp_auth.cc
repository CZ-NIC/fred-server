/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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
#include "src/libfred/db_settings.hh"
#include "src/libfred/registrar/epp_auth/exceptions.hh"
#include "src/libfred/registrar/epp_auth/get_registrar_epp_auth.hh"
#include "src/libfred/registrar/epp_auth/registrar_epp_auth_data.hh"

namespace LibFred {
namespace Registrar {
namespace EppAuth {

GetRegistrarEppAuth::GetRegistrarEppAuth(const std::string& _registrar_handle)
        : registrar_handle_(_registrar_handle)
{
}

RegistrarEppAuthData GetRegistrarEppAuth::exec(OperationContext& _ctx) const
{
    try
    {
        RegistrarEppAuthData registrar_epp_auth_data;
        const Database::Result db_result = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT r.handle, ra.id, ra.cert, ra.password "
                "FROM registrar AS r "
                "JOIN registraracl AS ra ON r.id=ra.registrarid "
                "WHERE r.handle=UPPER($1::text) ",
                // clang-format on
                Database::query_param_list(registrar_handle_));

        if (db_result.size() > 0)
        {
            registrar_epp_auth_data.registrar_handle = static_cast<std::string>(db_result[0]["handle"]);
            for (unsigned i = 0; i < db_result.size(); ++i)
            {
                EppAuthRecord record;
                record.id = static_cast<unsigned long long>(db_result[i]["id"]);
                record.certificate_fingerprint = static_cast<std::string>(db_result[i]["cert"]);
                record.hashed_password = static_cast<std::string>(db_result[i]["password"]);
                registrar_epp_auth_data.epp_auth_records.insert(std::move(record));
            }
        }
        return registrar_epp_auth_data;
    }
    catch (const std::exception&)
    {
        throw GetRegistrarEppAuthException();
    }
}

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred
