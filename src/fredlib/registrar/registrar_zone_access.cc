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

/**
 *  @file
 *  registrar zone access
 */


#include <boost/date_time/gregorian/gregorian.hpp>
#include "src/fredlib/opcontext.h"
#include "util/db/param_query_composition.h"
#include "src/fredlib/registrar/registrar_zone_access.h"
namespace Fred
{

    /**
     * check if registrar have allowed access to the zone
     */
    bool registrar_zone_access(unsigned long long registrar_id,
            unsigned long long zone_id,
            boost::gregorian::date _local_today, OperationContext& ctx)
    {
        Database::ReusableParameter local_today = Database::ReusableParameter(_local_today,"date");
        return ctx.get_conn().exec_params(Database::ParamQuery(
            "SELECT id "
            "FROM registrarinvoice "
            "WHERE registrarid=").param_bigint(registrar_id)
            (" AND zone=").param_bigint(zone_id)
            (" AND fromdate <= ").param(local_today)
            (" AND (todate >= ").param(local_today)(" OR todate IS NULL)")
            ).size();
    }
}//namespace Fred


