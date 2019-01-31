/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef UTIL_HH_07EB9DDD81E745B199F551BBEB64141E
#define UTIL_HH_07EB9DDD81E745B199F551BBEB64141E

#include "libfred/db_settings.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {

inline bool is_country_code_valid(LibFred::OperationContext& ctx, const std::string& cc) {
    return
        ctx.get_conn().exec_params(
                "SELECT 1 FROM enum_country WHERE id = $1::text FOR SHARE ",
                Database::query_param_list(cc)
                ).size() > 0;
}

} // namespace Epp

#endif
