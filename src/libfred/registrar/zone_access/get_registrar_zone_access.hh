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

#ifndef GET_REGISTRAR_ZONE_ACCESS_HH_D244602CE4E742B08CA40AE73028E1EF
#define GET_REGISTRAR_ZONE_ACCESS_HH_D244602CE4E742B08CA40AE73028E1EF

#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/zone_access/registrar_zone_access_type.hh"

#include <string>

namespace LibFred {
namespace Registrar {
namespace ZoneAccess {

class GetZoneAccess {
public:
    explicit GetZoneAccess(const std::string& _registrar_handle);

    RegistrarZoneAccesses exec(OperationContext& _ctx) const;

private:
    std::string registrar_handle_;
};

} // namespace LibFred::Registrar::ZoneAccess
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
