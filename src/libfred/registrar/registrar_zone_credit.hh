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

#ifndef REGISTRAR_ZONE_CREDIT_H_2C7193897FF02E1E1A6761E39870B8B1//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define REGISTRAR_ZONE_CREDIT_H_2C7193897FF02E1E1A6761E39870B8B1

#include "src/libfred/registrar/zone_credit.hh"

#include <set>

namespace LibFred {

struct OrderZoneCreditByZoneFqdn
{
    bool operator()(const ZoneCredit& a, const ZoneCredit& b)const;
};

typedef std::set<ZoneCredit, OrderZoneCreditByZoneFqdn> RegistrarZoneCredit;

} // namespace LibFred

#endif//REGISTRAR_ZONE_CREDIT_H_2C7193897FF02E1E1A6761E39870B8B1
