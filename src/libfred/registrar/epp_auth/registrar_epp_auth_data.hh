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

#ifndef REGISTRAR_EPP_AUTH_DATA_HH_6AAF7D673128405A9A77195271FC06E9
#define REGISTRAR_EPP_AUTH_DATA_HH_6AAF7D673128405A9A77195271FC06E9

#include <set>
#include <string>

namespace LibFred {
namespace Registrar {
namespace EppAuth {

struct EppAuthRecord
{
    unsigned long long id;
    std::string certificate_fingerprint;
    std::string hashed_password;

    bool operator<(const EppAuthRecord& _other) const
    {
        return id < _other.id;
    }
};

struct RegistrarEppAuthData
{
    std::string registrar_handle;
    std::set<EppAuthRecord> epp_auth_records;
};

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
