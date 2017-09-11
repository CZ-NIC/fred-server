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

#ifndef GET_REGISTRAR_ZONE_CREDIT_H_F5B012B7AE1C00C057707D35FFD0A1C9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define GET_REGISTRAR_ZONE_CREDIT_H_F5B012B7AE1C00C057707D35FFD0A1C9

#include "src/fredlib/registrar/registrar_zone_credit.h"
#include "src/fredlib/opcontext.h"

namespace Fred {

class GetRegistrarZoneCredit
{
public:
    RegistrarZoneCredit exec(OperationContext& _ctx, const std::string& _registrar_handle)const;
};

}//namespace Fred

#endif//GET_REGISTRAR_ZONE_CREDIT_H_F5B012B7AE1C00C057707D35FFD0A1C9
