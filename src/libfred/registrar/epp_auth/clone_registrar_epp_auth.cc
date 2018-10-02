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
#include "src/libfred/registrar/epp_auth/clone_registrar_epp_auth.hh"
#include "src/libfred/registrar/epp_auth/exceptions.hh"
#include "src/util/db/db_exceptions.hh"

namespace LibFred {
namespace Registrar {
namespace EppAuth {

CloneRegistrarEppAuth::CloneRegistrarEppAuth(const unsigned long long _id,
        const std::string& _certificate_fingerprint)
        : id_(_id),
          certificate_fingerprint_(_certificate_fingerprint)
{
}

unsigned long long CloneRegistrarEppAuth::exec(OperationContext& _ctx) const
{
    try
    {
        const Database::Result db_result = _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO registraracl (registrarid, cert, password) "
                "SELECT ra.registrarid, $1::text, ra.password FROM registraracl AS ra "
                "WHERE ra.id = $2::bigint "
                "RETURNING id ",
                // clang-format on
                Database::query_param_list(certificate_fingerprint_)(id_));

        if (db_result.size() == 1)
        {
            const auto id = static_cast<unsigned long long>(db_result[0][0]);
            return id;
        }
        if (db_result.size() == 0)
        {
            throw NonexistentRegistrarEppAuth();
        }
    }
    catch (const NonexistentRegistrarEppAuth&)
    {
        throw;
    }
    catch (const Database::ResultFailed&)
    {
        throw DuplicateCertificate();
    }
    catch (const std::exception&)
    {
        throw CloneRegistrarEppAuthException();
    }
    throw CloneRegistrarEppAuthException();
}

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred
