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
#include "src/libfred/registrar/zone_access/exceptions.hh"
#include "src/libfred/registrar/zone_access/get_registrar_zone_access.hh"

namespace LibFred {
namespace Registrar {
namespace ZoneAccess {

GetZoneAccess::GetZoneAccess(const std::string& _registrar_handle)
        : registrar_handle_(_registrar_handle)
{
}

RegistrarZoneAccesses GetZoneAccess::exec(OperationContext& _ctx) const
{
    try
    {
        RegistrarZoneAccesses zone_accesses;
        zone_accesses.registrar_handle = registrar_handle_;

        const Database::Result db_result = _ctx.get_conn().exec_params(
                "SELECT ri.id, z.fqdn AS zone, ri.fromdate, ri.todate "
                "FROM registrar AS r "
                "JOIN registrarinvoice AS ri ON r.id=ri.registrarid "
                "JOIN zone AS z ON z.id=ri.zone "
                "WHERE r.handle=UPPER($1::text) ",
                Database::query_param_list(registrar_handle_));
        if (db_result.size() > 0)
        {
            for (unsigned i = 0; i < db_result.size(); ++i)
            {
                ZoneAccess access;
                access.id = static_cast<unsigned long long>(db_result[i]["id"]);
                access.zone_fqdn = static_cast<std::string>(db_result[i]["zone"]);
                if (!db_result[i]["fromdate"].isnull())
                {
                    access.from_date = boost::gregorian::from_string(
                            static_cast<std::string>(db_result[i]["fromdate"]));
                }
                if (!db_result[i]["todate"].isnull())
                {
                    access.to_date = boost::gregorian::from_string(
                            static_cast<std::string>(db_result[i]["todate"]));
                }
                zone_accesses.zone_accesses.push_back(std::move(access));
            }
        }
        return zone_accesses;
    }
    catch (const std::exception& e)
    {
        throw GetRegistrarZoneAccessException();
    }
}

} // namespace LibFred::Registrar::ZoneAccess
} // namespace LibFred::Registrar
} // namespace LibFred
