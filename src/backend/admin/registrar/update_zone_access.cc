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

#include "src/backend/admin/registrar/update_zone_access.hh"
#include "src/libfred/registrar/zone_access/add_registrar_zone_access.hh"
#include "src/libfred/registrar/zone_access/exceptions.hh"
#include "src/libfred/registrar/zone_access/update_registrar_zone_access.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/log/context.hh"
#include "src/util/log/logger.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>

namespace Admin {
namespace Registrar {

namespace {

class LogContext
{
public:
    explicit LogContext(const std::string& _op_name)
            : ctx_operation_(_op_name)
    {
    }

private:
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(__FUNCTION__);

} // namespace Admin::Registrar::{anonymous}

void update_zone_access(const LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses& _zones)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER(PACKAGE).debug("Registrar handle: " + _zones.registrar_handle);

    LibFred::OperationContextCreator ctx;
    for (const auto& zone : _zones.zone_accesses)
    {
        boost::optional<boost::gregorian::date> to_date = boost::none;
        if (!zone.to_date.is_special())
        {
            to_date = zone.to_date;
        }

        if (zone.id != 0)
        {
            const std::string operation_name = "LibFred::Registrar::ZoneAccess::UpdateRegistrarZoneAccess() ";
            TRACE("[CALL] " + operation_name);

            if (zone.from_date.is_special() && zone.to_date.is_special())
            {
                LOGGER(PACKAGE).info(operation_name + "No update data");
                throw ZoneAccessNoUpdateData();
            }
            boost::optional<boost::gregorian::date> from_date = boost::none;
            if (!zone.from_date.is_special())
            {
                from_date = zone.from_date;
            }
            try
            {
                LibFred::Registrar::ZoneAccess::UpdateRegistrarZoneAccess(zone.id)
                    .set_from_date(from_date)
                    .set_to_date(to_date)
                    .exec(ctx);
            }
            catch (const LibFred::Registrar::ZoneAccess::NonexistentZoneAccess& e)
            {
                LOGGER(PACKAGE).warning(operation_name + e.what());
                throw NonexistentZoneAccess();
            }
            catch (const std::exception& e)
            {
                LOGGER(PACKAGE).error(operation_name + e.what());
                throw UpdateZoneAccessException();
            }
        }
        else
        {
            const std::string operation_name = "LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess() ";
            TRACE("[CALL] " + operation_name);

            const bool missing_params = (zone.from_date.is_special()) ||
                    (zone.zone_fqdn.empty()) ||
                    _zones.registrar_handle.empty();
            if (missing_params)
            {
                LOGGER(PACKAGE).info(operation_name + ": Missing parameters - " +
                                (zone.from_date.is_special() ? "from_date " : "") +
                                (zone.zone_fqdn.empty() ? "zone_fqdn " : "")  +
                                (_zones.registrar_handle.empty() ? "registrar_handle " : ""));
                throw ZoneAccessMissingParameters();
            }
            try
            {
                LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(_zones.registrar_handle,
                        zone.zone_fqdn,
                        zone.from_date)
                    .set_to_date(to_date)
                    .exec(ctx);
            }
            catch (const LibFred::Registrar::ZoneAccess::NonexistentRegistrar& e)
            {
                LOGGER(PACKAGE).warning(operation_name + e.what());
                throw ZoneAccessNonexistentRegistrar();
            }
            catch (const LibFred::Registrar::ZoneAccess::NonexistentZone& e)
            {
                LOGGER(PACKAGE).warning(operation_name + e.what());
                throw NonexistentZone();
            }
            catch (const std::exception& e)
            {
                LOGGER(PACKAGE).error(operation_name + e.what());
                throw UpdateZoneAccessException();
            }
        }
    }
    ctx.commit_transaction();
}

const char* UpdateZoneAccessException::what() const noexcept
{
    return "Failed to update registrar zone access due to an unknown exception.";
}

const char* ZoneAccessNoUpdateData::what() const noexcept
{
    return "No data for update registrar zone access.";
}

const char* NonexistentZoneAccess::what() const noexcept
{
    return "Failed to update registrar zone access because the access doesn't exist.";
}

const char* ZoneAccessMissingParameters::what() const noexcept
{
    return "Failed to add registrar zone access due to missing mandatory parameter.";
}

const char* ZoneAccessNonexistentRegistrar::what() const noexcept
{
    return "Failed to add registrar zone access because the registrar doesn't exist.";
}

const char* NonexistentZone::what() const noexcept
{
    return "Failed to add registrar zone access because the zone doesn't exist.";
}

} // namespace Admin::Registrar
} // namespace Admin
