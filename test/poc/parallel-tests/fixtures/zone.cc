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
#include "test/poc/parallel-tests/fixtures/zone.hh"

#include "libfred/zone/create_zone.hh"
#include "libfred/zone/exceptions.hh"
#include "libfred/zone/info_zone.hh"

#include "libfred/registrable_object/domain/domain_name.hh"

#include "util/log/logger.hh"

#include <cstring>
#include <stdexcept>


namespace Test {

namespace {

struct GetNonEnumZone : boost::static_visitor<LibFred::Zone::NonEnumZone>
{
    LibFred::Zone::NonEnumZone operator()(const LibFred::Zone::NonEnumZone& value) const
    {
        return value;
    }
    template <typename T>
    LibFred::Zone::NonEnumZone operator()(const T&) const
    {
        throw std::runtime_error{"not a non-enum zone"};
    }
};

struct GetEnumZone : boost::static_visitor<LibFred::Zone::EnumZone>
{
    LibFred::Zone::EnumZone operator()(const LibFred::Zone::EnumZone& value) const
    {
        return value;
    }
    template <typename T>
    LibFred::Zone::EnumZone operator()(const T&) const
    {
        throw std::runtime_error{"not an enum zone"};
    }
};

auto get_non_enum_zone(LibFred::OperationContext& ctx, const char* zone, bool idn_enabled)
{
    try
    {
        return boost::apply_visitor(GetNonEnumZone{}, LibFred::Zone::InfoZone{zone}.exec(ctx));
    }
    catch (const LibFred::Zone::NonExistentZone&) { }
    catch (const LibFred::Zone::InfoZoneException& e)
    {
        ::LOGGER.info(boost::format{"InfoZone failed: %1%"} % e.what());
    }
    catch (const std::exception& e)
    {
        ::LOGGER.error(boost::format{"InfoZone failed: %1%"} % e.what());
        throw;
    }
    catch (...)
    {
        ::LOGGER.error("InfoZone failed by an unexpected exception");
        throw;
    }
    static constexpr int expiration_period_min_in_months = 12;
    static constexpr int expiration_period_max_in_months = 120;
    LibFred::Zone::CreateZone{zone, expiration_period_min_in_months, expiration_period_max_in_months}
            .exec(ctx);
    if (!idn_enabled)
    {
        LibFred::Domain::set_domain_name_validation_config_into_database(ctx, zone, { "dncheck_no_consecutive_hyphens" });
    }
    return boost::apply_visitor(GetNonEnumZone{}, LibFred::Zone::InfoZone{zone}.exec(ctx));
}

auto get_enum_zone(LibFred::OperationContext& ctx, const char* zone, int enum_validation_period_in_months)
{
    try
    {
        return boost::apply_visitor(GetEnumZone{}, LibFred::Zone::InfoZone{zone}.exec(ctx));
    }
    catch (const LibFred::Zone::NonExistentZone&) { }
    catch (const LibFred::Zone::InfoZoneException& e)
    {
        ::LOGGER.info(boost::format{"InfoZone failed: %1%"} % e.what());
    }
    catch (const std::exception& e)
    {
        ::LOGGER.error(boost::format{"InfoZone failed: %1%"} % e.what());
        throw;
    }
    catch (...)
    {
        ::LOGGER.error("InfoZone failed by an unexpected exception");
        throw;
    }
    static constexpr int expiration_period_min_in_months = 12;
    static constexpr int expiration_period_max_in_months = 120;
    LibFred::Zone::CreateZone{zone, expiration_period_min_in_months, expiration_period_max_in_months}
            .set_enum_validation_period_in_months(enum_validation_period_in_months)
            .exec(ctx);
    LibFred::Domain::set_domain_name_validation_config_into_database(ctx, zone, { "dncheck_single_digit_labels_only" });
    return boost::apply_visitor(GetEnumZone{}, LibFred::Zone::InfoZone{zone}.exec(ctx));
}

std::string make_fqdn(const char* subdomain, std::size_t subdomain_length, const char* zone)
{
    auto fqdn = std::string{};
    if (subdomain_length == 0)
    {
        fqdn = zone;
        return fqdn;
    }
    const auto zone_length = std::strlen(zone);
    if (subdomain[subdomain_length - 1] == '.')
    {
        fqdn.reserve(subdomain_length + zone_length);
        fqdn.append(subdomain, subdomain_length);
    }
    else
    {
        fqdn.reserve(subdomain_length + 1 + zone_length);
        fqdn.append(subdomain, subdomain_length);
        fqdn.append(1, '.');
    }
    fqdn.append(zone, zone_length);
    return fqdn;
}

std::string make_fqdn(const char* subdomain, const char* zone)
{
    return make_fqdn(subdomain, std::strlen(subdomain), zone);
}

std::string make_fqdn(const std::string& subdomain, const char* zone)
{
    return make_fqdn(subdomain.c_str(), subdomain.length(), zone);
}

std::string make_fqdn(unsigned long long subdomain, const char* zone)
{
    auto fqdn = std::string{};
    if (subdomain == 0)
    {
        fqdn = zone;
        return fqdn;
    }
    const auto number_of_digits = [](unsigned long long value)
    {
        unsigned result = 1;
        while (10 <= value)
        {
            ++result;
            value /= 10;
        }
        return result;
    }(subdomain);
    const auto zone_length = std::strlen(zone);
    fqdn.reserve(2 * number_of_digits + zone_length);
    while (0 < subdomain)
    {
        const char level[2] = {char('0' + (subdomain % 10)), '.'};
        fqdn.append(level, 2);
        subdomain /= 10;
    }
    fqdn.append(zone, zone_length);
    return fqdn;
}

}//namespace Test::{anonymous}

Zone::Zone(LibFred::OperationContext& ctx, const char* zone, bool idn_enabled)
    : data{get_non_enum_zone(ctx, zone, idn_enabled)}
{ }

EnumZone::EnumZone(LibFred::OperationContext& ctx, const char* zone, int enum_validation_period_in_months)
    : data{get_enum_zone(ctx, zone, enum_validation_period_in_months)}
{ }

CzZone::CzZone(LibFred::OperationContext& ctx)
    : Zone{ctx, fqdn()}
{ }

const char* CzZone::fqdn() noexcept { return "cz"; }

std::string CzZone::fqdn(const char* subdomain)
{
    return make_fqdn(subdomain, fqdn());
}

std::string CzZone::fqdn(const std::string& subdomain)
{
    return make_fqdn(subdomain, fqdn());
}

constexpr int default_enum_validation_period_in_months = 6;

CzEnumZone::CzEnumZone(LibFred::OperationContext& ctx)
    : EnumZone{ctx, fqdn(), default_enum_validation_period_in_months}
{ }

const char* CzEnumZone::fqdn() noexcept { return "0.2.4.e164.arpa"; }

std::string CzEnumZone::fqdn(unsigned long long subdomain)
{
    return make_fqdn(subdomain, fqdn());
}

InitDomainNameCheckers::InitDomainNameCheckers(LibFred::OperationContext& ctx)
{
    LibFred::Domain::insert_domain_name_checker_name_into_database(ctx, "dncheck_no_consecutive_hyphens", "forbid consecutive hyphens");
    LibFred::Domain::insert_domain_name_checker_name_into_database(ctx, "dncheck_single_digit_labels_only", "enforces single digit labels (for enum domains)");
}

}//namespace Test
