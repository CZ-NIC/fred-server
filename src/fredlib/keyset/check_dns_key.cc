/*
 * Copyright (C) 2016 CZ.NIC, z.s.p.o.
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

#include "src/fredlib/keyset/check_dns_key.h"
#include <stdexcept>

namespace Fred {
namespace DnsSec {

Algorithm::Usability get_algorithm_usability(OperationContext &ctx, int algorithm_number)
{
    const Database::Result result = ctx.get_conn().exec_params(
            "SELECT EXISTS(SELECT 1 FROM dnssec_algorithm_blacklist WHERE alg_number=id) "
            "FROM dnssec_algorithm WHERE id=$1::INTEGER", Database::query_param_list(algorithm_number));
    if (result.size() == 0)
    {
        return Algorithm::invalid_value;
    }
    if (result.size() != 1)
    {
        throw std::runtime_error("SELECT returns too many rows");
    }
    return static_cast< bool >(result[0][0]) ? Algorithm::forbidden : Algorithm::usable;
}

}//namespace Fred::DnsSec
}//namespace Fred
