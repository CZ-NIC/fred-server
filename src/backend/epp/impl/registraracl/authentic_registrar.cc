/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/impl/registraracl/authentic_registrar.hh"

#include "util/password_storage.hh"

#include <stdexcept>

namespace Epp {
namespace RegistrarAcl {

AuthenticRegistrar::AuthenticRegistrar(
        Database::Connection& _conn,
        unsigned long long _registrar_id,
        const std::string& _certificate_fingerprint,
        const std::string& _plaintext_password)
    : conn_(_conn),
      registrar_id_(_registrar_id),
      certificate_fingerprint_(_certificate_fingerprint),
      registrar_acl_id_(0)
{
    const Database::Result dbres = conn_.exec_params(
            "SELECT id,password "
            "FROM registraracl "
            "WHERE registrarid=$1::BIGINT AND "
                  "cert=$2::TEXT "
            "FOR UPDATE",
            Database::query_param_list(registrar_id_)
                                      (certificate_fingerprint_));
    if (dbres.size() <= 0)
    {
        throw AuthenticationFailed();
    }
    if (1 < dbres.size())
    {
        throw std::runtime_error("too many rows selected from registraracl");
    }
    registrar_acl_id_ = static_cast<unsigned long long>(dbres[0][0]);
    const auto encrypted_password = PasswordStorage::PasswordData::construct_from(static_cast<std::string>(dbres[0][1]));
    try
    {
        PasswordStorage::check_password(_plaintext_password, encrypted_password);
    }
    catch (const PasswordStorage::IncorrectPassword&)
    {
        throw AuthenticationFailed();
    }
    if (!is_encrypted_by_preferred_method(encrypted_password))
    {
        this->set_password(_plaintext_password);
    }
}

void AuthenticRegistrar::set_password(
        const std::string& _plaintext_password)const
{
    const auto encrypted_password = PasswordStorage::encrypt_password_by_preferred_method(_plaintext_password);
    const auto dbres = conn_.exec_params(
            "UPDATE registraracl "
            "SET password=$4::TEXT "
            "WHERE id=$1::BIGINT AND "
                  "registrarid=$2::BIGINT AND "
                  "cert=$3::TEXT",
            Database::query_param_list(registrar_acl_id_)
                                      (registrar_id_)
                                      (certificate_fingerprint_)
                                      (encrypted_password.get_value()));
    if (dbres.rows_affected() != 1)
    {
        throw std::runtime_error("unexpected number of updated records in registraracl table");
    }
}

} // namespace Epp::RegistrarAcl
} // namespace Epp
