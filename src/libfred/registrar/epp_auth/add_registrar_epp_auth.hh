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

#ifndef ADD_REGISTRAR_EPP_AUTH_HH_3704104C25B849B6B67818302E72B174
#define ADD_REGISTRAR_EPP_AUTH_HH_3704104C25B849B6B67818302E72B174

#include "src/libfred/opcontext.hh"

#include <string>

namespace LibFred {
namespace Registrar {
namespace EppAuth {

class AddRegistrarEppAuth
{
public:
    AddRegistrarEppAuth(
            const std::string& _registrar_handle,
            const std::string& _certificate_fingerprint,
            const std::string& _plain_password);

    unsigned long long exec(OperationContext& _ctx) const;

private:
    std::string registrar_handle_;
    std::string certificate_fingerprint_;
    std::string plain_password_;
};

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
