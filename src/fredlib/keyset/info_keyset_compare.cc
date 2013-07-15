/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @info_keyset_compare.cc
 *  comparsion of keyset info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

#include "fredlib/keyset/info_keyset_compare.h"

namespace Fred
{

    bool operator==(const InfoKeysetOutput& lhs, const InfoKeysetHistoryOutput& rhs)
    {
            return lhs.info_keyset_data == rhs.info_keyset_data;
    }

    bool operator==(const InfoKeysetHistoryOutput& lhs, const InfoKeysetOutput& rhs)
    {
        return operator==(rhs,lhs);
    }

    bool operator!=(const InfoKeysetOutput& lhs, const InfoKeysetHistoryOutput& rhs)
    {
        return !operator==(lhs,rhs);
    }

    bool operator!=(const InfoKeysetHistoryOutput& lhs, const InfoKeysetOutput& rhs)
    {
        return !operator==(rhs,lhs);
    }

}//namespace Fred

