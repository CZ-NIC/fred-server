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

#ifndef AUTHENTIC_REGISTRAR_HH_9AD1BAC6EA509CCF4B1B535BBDD237ED//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define AUTHENTIC_REGISTRAR_HH_9AD1BAC6EA509CCF4B1B535BBDD237ED

#include "src/fredlib/db_settings.h"

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

#endif//AUTHENTIC_REGISTRAR_HH_9AD1BAC6EA509CCF4B1B535BBDD237ED
