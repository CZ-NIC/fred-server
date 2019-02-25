/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef AUTHENTIC_REGISTRAR_HH_6357347039744A8094FD4FEEDD63D8CE
#define AUTHENTIC_REGISTRAR_HH_6357347039744A8094FD4FEEDD63D8CE

#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/db_settings.hh"

#include <exception>
#include <string>

namespace Epp {
namespace RegistrarAcl {

class AuthenticRegistrar
{
public:
    struct AuthenticationFailed:std::exception
    {
        const char* what()const throw() { return "authentication of registrar failed"; }
    };
    AuthenticRegistrar(
            Database::Connection& _conn,
            unsigned long long _registrar_id,
            const std::string& _certificate_fingerprint,
            const std::string& _plaintext_password);
    void set_password(const std::string& _plaintext_password)const;
private:
    Database::Connection& conn_;
    const unsigned long long registrar_id_;
    const std::string certificate_fingerprint_;
    unsigned long long registrar_acl_id_;
};

}//namespace Epp::RegistrarAcl
}//namespace Epp

#endif//AUTHENTIC_REGISTRAR_HH_6357347039744A8094FD4FEEDD63D8CE
