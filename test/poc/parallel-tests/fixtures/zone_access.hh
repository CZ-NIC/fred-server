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

#ifndef ZONE_ACCESS_HH_85EFCDEA0343C1E627148F3F171509C1//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define ZONE_ACCESS_HH_85EFCDEA0343C1E627148F3F171509C1

#include "test/poc/parallel-tests/fixtures/registrar.hh"
#include "test/poc/parallel-tests/fixtures/zone.hh"

#include "libfred/opcontext.hh"


namespace Test {

class ZoneAccess
{
public:
    explicit ZoneAccess(LibFred::OperationContext& ctx, const Zone& zone, const Registrar& registrar);
    explicit ZoneAccess(LibFred::OperationContext& ctx, const EnumZone& zone, const Registrar& registrar);
    explicit ZoneAccess(LibFred::OperationContext& ctx, const Zone& zone, const Registrar& registrar, long long credit);
    explicit ZoneAccess(LibFred::OperationContext& ctx, const EnumZone& zone, const Registrar& registrar, long long credit);
private:
    explicit ZoneAccess(LibFred::OperationContext& ctx, const std::string& zone, const Registrar& registrar);
    explicit ZoneAccess(LibFred::OperationContext& ctx, const std::string& zone, const Registrar& registrar, long long credit);
};

}//namespace Test

#endif//ZONE_ACCESS_HH_85EFCDEA0343C1E627148F3F171509C1
