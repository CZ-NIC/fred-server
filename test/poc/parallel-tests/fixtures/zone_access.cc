/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#include "test/poc/parallel-tests/fixtures/zone_access.hh"

#include "libfred/registrar/zone_access/add_registrar_zone_access.hh"
#include "libfred/registrar/credit/create_registrar_credit_transaction.hh"

#include "util/log/logger.hh"

#include <exception>


namespace Test {

ZoneAccess::ZoneAccess(LibFred::OperationContext& ctx, const Zone& zone, const Registrar& registrar)
    : ZoneAccess{ctx, zone.data.fqdn, registrar}
{ }

ZoneAccess::ZoneAccess(LibFred::OperationContext& ctx, const EnumZone& zone, const Registrar& registrar)
    : ZoneAccess{ctx, zone.data.fqdn, registrar}
{ }

ZoneAccess::ZoneAccess(LibFred::OperationContext& ctx, const Zone& zone, const Registrar& registrar, long long credit)
    : ZoneAccess{ctx, zone.data.fqdn, registrar, credit}
{ }

ZoneAccess::ZoneAccess(LibFred::OperationContext& ctx, const EnumZone& zone, const Registrar& registrar, long long credit)
    : ZoneAccess{ctx, zone.data.fqdn, registrar, credit}
{ }

ZoneAccess::ZoneAccess(LibFred::OperationContext& ctx, const std::string& zone, const Registrar& registrar)
{
    try
    {
        LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess{
                registrar.data.handle,
                zone,
                boost::gregorian::date{boost::gregorian::day_clock::local_day()}}.exec(ctx);
    }
    catch (const std::exception& e)
    {
        ::LOGGER.error(boost::format{"unable to add an access to \"%1%\" zone for \"%2%\" registrar: %3%"}
                       % zone % registrar.data.handle % e.what());
        throw;
    }
    catch (...)
    {
        ::LOGGER.error(boost::format{"unable to add an access to \"%1%\" zone for \"%2%\" registrar"}
                       % zone % registrar.data.handle);
        throw;
    }
}

ZoneAccess::ZoneAccess(LibFred::OperationContext& ctx, const std::string& zone, const Registrar& registrar, long long credit)
    : ZoneAccess{ctx, zone, registrar}
{
    try
    {
        LibFred::Registrar::Credit::CreateRegistrarCreditTransaction{
                registrar.data.handle,
                zone,
                std::to_string(credit)}.exec(ctx);
    }
    catch (const std::exception& e)
    {
        ::LOGGER.error(boost::format{"unable to add credit %1% to \"%2%\" zone for \"%3%\" registrar: %4%"}
                       % credit % zone % registrar.data.handle % e.what());
        throw;
    }
    catch (...)
    {
        ::LOGGER.error(boost::format{"unable to add credit %1% to \"%2%\" zone for \"%3%\" registrar"}
                       % credit % zone % registrar.data.handle);
        throw;
    }
}

}//namespace Test
