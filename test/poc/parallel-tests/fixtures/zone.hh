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
#ifndef ZONE_HH_D8D996DA0D05D9258DCA58598AFE4CB3//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define ZONE_HH_D8D996DA0D05D9258DCA58598AFE4CB3

#include "libfred/opcontext.hh"
#include "libfred/zone/info_zone_data.hh"


namespace Test {

struct Zone
{
    explicit Zone(
            LibFred::OperationContext& ctx,
            const char* zone,
            bool idn_enabled = false);
    LibFred::Zone::NonEnumZone data;
};

struct EnumZone
{
    explicit EnumZone(
            LibFred::OperationContext& ctx,
            const char* zone,
            int enum_validation_period_in_months);
    LibFred::Zone::EnumZone data;
};

struct CzZone : Zone
{
    explicit CzZone(LibFred::OperationContext& ctx);
    static const char* fqdn() noexcept;
    static std::string fqdn(const char* subdomain);
    static std::string fqdn(const std::string& subdomain);
};

struct CzEnumZone : EnumZone
{
    explicit CzEnumZone(LibFred::OperationContext& ctx);
    static const char* fqdn() noexcept;
    static std::string fqdn(unsigned long long subdomain);
};

struct InitDomainNameCheckers
{
    explicit InitDomainNameCheckers(LibFred::OperationContext& ctx);
};

}//namespace Test

#endif//ZONE_HH_D8D996DA0D05D9258DCA58598AFE4CB3
