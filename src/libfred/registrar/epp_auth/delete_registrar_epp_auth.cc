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
#include "src/libfred/registrar/epp_auth/delete_registrar_epp_auth.hh"
#include "src/libfred/registrar/epp_auth/exceptions.hh"

#include <stdexcept>

namespace LibFred {
namespace Registrar {
namespace EppAuth {

DeleteRegistrarEppAuth::DeleteRegistrarEppAuth(const unsigned long long _id)
        : id_(_id)
{
}

void DeleteRegistrarEppAuth::exec(OperationContext& _ctx) const
{
    try
    {
        const Database::Result db_result = _ctx.get_conn().exec_params(
                // clang-format off
                "DELETE FROM registraracl "
                "WHERE id = $1::bigint "
                "RETURNING 1",
                // clang-format on
                Database::query_param_list(id_));

        if (db_result.size() == 0)
        {
            throw NonexistentRegistrarEppAuth();
        }
        if (db_result.size() > 1)
        {
            throw std::runtime_error("Duplicity in database");
        }
    }
    catch (const NonexistentRegistrarEppAuth&)
    {
        throw;
    }
    catch (const std::exception&)
    {
        throw DeleteRegistrarEppAuthException();
    }
}

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred
