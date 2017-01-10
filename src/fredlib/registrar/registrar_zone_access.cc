/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar_data.h"
#include "src/fredlib/registrar/registrar_zone_access.h"
#include "util/db/param_query_composition.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {

bool is_zone_accessible_by_registrar(unsigned long long _registrar_id,
        unsigned long long _zone_id,
        boost::gregorian::date _local_today,
        OperationContext& _ctx)
{
    const bool is_registrar_system = Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.system.get_value_or(false);

    if (is_registrar_system)
    {
        return true;
    }

    Database::ReusableParameter local_today = Database::ReusableParameter(_local_today, "date");
    return _ctx.get_conn().exec_params(Database::ParamQuery(
                                               "SELECT id "
                                               "FROM registrarinvoice "
                                               "WHERE registrarid=")
                                               .param_bigint(_registrar_id)(" AND zone=")
                                               .param_bigint(_zone_id)(" AND fromdate <= ")
                                               .param(local_today)(" AND (todate >= ")
                                               .param(local_today)(" OR todate IS NULL)"))
                                        .size();
}

} // namespace Fred
