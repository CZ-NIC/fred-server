/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef UPDATE_EPP_AUTH_HH_85C783403B7C41BD9C888F632A41942F
#define UPDATE_EPP_AUTH_HH_85C783403B7C41BD9C888F632A41942F

#include <exception>
#include <string>
#include <vector>

namespace Admin {
namespace Registrar {

struct EppAuthRecord
{
    unsigned long long id;
    std::string certificate_fingerprint;
    std::string plain_password;
    std::string new_certificate_fingerprint;

    EppAuthRecord();

    bool operator==(const EppAuthRecord& _other) const;
};

struct EppAuthData
{
    std::string registrar_handle;
    std::vector<EppAuthRecord> epp_auth_records;
};

unsigned long long add_epp_auth(const std::string& _registrar_handle,
        const std::string& _certificate_fingerprint,
        const std::string& _plain_password);

void update_epp_auth(const EppAuthData& _auth_data);

struct AddEppAuthException : std::exception
{
    const char* what() const noexcept override;
};

struct UpdateEppAuthException : std::exception
{
    const char* what() const noexcept override;
};

struct EppAuthMissingParameters : std::exception
{
    const char* what() const noexcept override;
};

struct EppAuthNoUpdateData : std::exception
{
    const char* what() const noexcept override;
};

struct EppAuthNonexistentRegistrar : std::exception
{
    const char* what() const noexcept override;
};

struct NonexistentEppAuth : std::exception
{
    const char* what() const noexcept override;
};

struct DuplicateCertificate : std::exception
{
    const char* what() const noexcept override;
};

} // namespace Admin::Registrar
} // namespace Admin
#endif
