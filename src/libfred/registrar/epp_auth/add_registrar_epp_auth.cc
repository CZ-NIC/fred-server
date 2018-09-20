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
#include "src/libfred/registrar/epp_auth/add_registrar_epp_auth.hh"
#include "src/libfred/registrar/epp_auth/exceptions.hh"
#include "src/util/password_storage.hh"

namespace LibFred {
namespace Registrar {
namespace EppAuth {

AddRegistrarEppAuth::AddRegistrarEppAuth(
            const std::string& _registrar_handle,
            const std::string& _certificate_fingerprint,
            const std::string& _plain_password)
        : registrar_handle_(_registrar_handle),
          certificate_fingerprint_(_certificate_fingerprint),
          plain_password_(_plain_password)
{
}

unsigned long long AddRegistrarEppAuth::exec(OperationContext& _ctx) const
{
    const auto encrypted_password = PasswordStorage::encrypt_password_by_preferred_method(plain_password_);

    try
    {
        const Database::Result db_result = _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO registraracl (registrarid, cert, password) "
                "SELECT r.id, $1::text, $2::text FROM registrar AS r "
                "WHERE r.handle = UPPER($3::text) "
                "RETURNING id",
                // clang-format on
                Database::query_param_list(certificate_fingerprint_)
                                          (encrypted_password.get_value())
                                          (registrar_handle_));

        if (db_result.size() == 1)
        {
            const auto id = static_cast<unsigned long long>(db_result[0][0]);
            return id;
        }
        if (db_result.size() == 0)
        {
            throw NonexistentRegistrar();
        }
    }
    catch (const NonexistentRegistrar&)
    {
        throw;
    }
    catch (const std::exception&)
    {
        throw AddRegistrarEppAuthException();
    }
    throw AddRegistrarEppAuthException();
}

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred
