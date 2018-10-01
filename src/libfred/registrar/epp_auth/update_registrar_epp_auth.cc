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
#include "src/libfred/registrar/epp_auth/update_registrar_epp_auth.hh"
#include "src/util/db/query_param.hh"
#include "src/util/password_storage.hh"
#include "src/util/util.hh"

#include <sstream>
#include <stdexcept>

namespace LibFred {
namespace Registrar {
namespace EppAuth {

namespace {
constexpr const char * psql_type(const std::string&)
{
    return "::text";
}

constexpr const char * psql_type(const unsigned long long)
{
    return "::bigint";
}

} // namespace LibFred::Registrar::EppAuth::{anonymous}

UpdateRegistrarEppAuth::UpdateRegistrarEppAuth(const unsigned long long _id)
        : id_(_id)
{
}

UpdateRegistrarEppAuth& UpdateRegistrarEppAuth::set_certificate_fingerprint(
        const boost::optional<std::string>& _certificate_fingerprint)
{
    certificate_fingerprint_ = _certificate_fingerprint;
    return *this;
}

UpdateRegistrarEppAuth& UpdateRegistrarEppAuth::set_plain_password(
        const boost::optional<std::string>& _plain_password)
{
    plain_password_ = _plain_password;
    return *this;
}

void UpdateRegistrarEppAuth::exec(OperationContext& _ctx) const
{
    const bool values_for_update_are_set = (certificate_fingerprint_ != boost::none ||
                                            plain_password_ != boost::none);

    if (!values_for_update_are_set)
    {
        throw NoUpdateData();
    }

    Database::QueryParams params;
    std::ostringstream object_sql;
    Util::HeadSeparator set_separator(" SET ", ", ");

    object_sql << "UPDATE registraracl";
    if (certificate_fingerprint_ != boost::none)
    {
        params.push_back(*certificate_fingerprint_);
        object_sql << set_separator.get();
        object_sql <<  "cert = $" << params.size() << psql_type(certificate_fingerprint_.get());
    }
    if (plain_password_ != boost::none)
    {
        const auto encrypted_password =
                PasswordStorage::encrypt_password_by_preferred_method(plain_password_.get());
        params.push_back(encrypted_password.get_value());
        object_sql << set_separator.get();
        object_sql << "password = $" << params.size() << psql_type(encrypted_password.get_value());
    }
    params.push_back(id_);
    object_sql << " WHERE id = $" << params.size() << psql_type(id_) <<  " RETURNING 1";

    try
    {
        const Database::Result update_result = _ctx.get_conn().exec_params(
                object_sql.str(),
                params);
        if (update_result.size() == 0)
        {
            throw NonexistentRegistrarEppAuth();
        }
        if (update_result.size() > 1)
        {
            throw std::runtime_error("Duplicate in database.");
        }
    }
    catch (const NonexistentRegistrarEppAuth&)
    {
        throw;
    }
    catch (const std::exception&)
    {
        throw UpdateRegistrarEppAuthException();
    }
}

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred
