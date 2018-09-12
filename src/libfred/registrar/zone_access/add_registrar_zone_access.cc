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

#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/zone_access/add_registrar_zone_access.hh"
#include "src/libfred/registrar/zone_access/exceptions.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/optional.hpp>
#include <string>

namespace LibFred {
namespace Registrar {
namespace ZoneAccess {

AddRegistrarZoneAccess::AddRegistrarZoneAccess(
        const std::string& _registrar_handle,
        const std::string& _zone_fqdn,
        const boost::gregorian::date& _from_date)
        : registrar_handle_(_registrar_handle),
          zone_fqdn_(_zone_fqdn),
          from_date_(_from_date),
          to_date_(not_a_date_time)
{
}

AddRegistrarZoneAccess& AddRegistrarZoneAccess::set_to_date(
        const boost::optional<boost::gregorian::date>& _to_date)
{
    to_date_ = _to_date;
    return *this;
}

unsigned long long AddRegistrarZoneAccess::exec(OperationContext& _ctx) const
{
    try
    {
        const Database::Result insert_result = _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO registrarinvoice (registrarid, zone, fromdate, todate) "
                "SELECT r.id, (SELECT z.id FROM zone AS z WHERE z.fqdn = LOWER($1::text)), $2::date, $3::date "
                "FROM registrar AS r WHERE r.handle = UPPER($4::text) "
                "RETURNING id",
                // clang-format on
                Database::query_param_list(zone_fqdn_)
                                          (from_date_)
                                          (to_date_->is_special() ?
                                                Database::QPNull : Database::QueryParam(to_date_.get()))
                                          (registrar_handle_));

        if (insert_result.size() == 1)
        {
            const unsigned long long id = static_cast<unsigned long long>(insert_result[0][0]);
            return id;
        }
        else if (insert_result.size() == 0)
        {
            throw NonexistentRegistrar();
        }
        else
        {
            throw std::runtime_error("Duplicity in database");
        }
    }
    catch (const NonexistentRegistrar&)
    {
        throw;
    }
    catch (const Database::ResultFailed&)
    {
        throw NonexistentZone();
    }
    catch (const std::exception&)
    {
        throw AddRegistrarZoneAccessException();
    }
}

} // namespace LibFred::Registrar::ZoneAccess
} // namespace LibFred::Registrar
} // namespace LibFred

