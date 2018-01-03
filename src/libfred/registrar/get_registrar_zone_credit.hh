/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef GET_REGISTRAR_ZONE_CREDIT_HH_D79316DD78B143EAB0BFE211A4C3AD0F
#define GET_REGISTRAR_ZONE_CREDIT_HH_D79316DD78B143EAB0BFE211A4C3AD0F

#include "src/libfred/registrar/registrar_zone_credit.hh"
#include "src/libfred/opcontext.hh"

namespace LibFred {

class GetRegistrarZoneCredit
{
public:
    RegistrarZoneCredit exec(OperationContext& _ctx, const std::string& _registrar_handle)const;
};

} // namespace LibFred

#endif
