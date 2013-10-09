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
 *  @file
 *  comparison of nsset info
 */

#ifndef INFO_NSSET_COMPARE_H_
#define INFO_NSSET_COMPARE_H_

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/nsset/info_nsset.h"
#include "fredlib/nsset/info_nsset_history.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    /**
    * Equality of the nsset data with nsset history data operator.
    * @param lhs is the nsset data
    * @param rhs is the nsset history data
    * @return true if equal, false if not
    */
    bool operator==(const InfoNssetOut& lhs, const InfoNssetOutput& rhs);

    /**
    * Equality of nsset history data with the nsset data operator.
    * @param lhs is the nsset history data
    * @param rhs is the nsset data
    * @return true if equal, false if not
    */
    bool operator==(const InfoNssetOutput& lhs, const InfoNssetOut& rhs);

    /**
    * Inequality of nsset data with the nsset history data operator.
    * @param lhs is the nsset data
    * @param rhs is the nsset history data
    * @return true if not equal, false if equal
    */
    bool operator!=(const InfoNssetOut& lhs, const InfoNssetOutput& rhs);

    /**
    * Inequality of nsset history data with the nsset data operator.
    * @param lhs is the nsset history data
    * @param rhs is the nsset data
    * @return true if not equal, false if equal
    */
    bool operator!=(const InfoNssetOutput& lhs, const InfoNssetOut& rhs);

}//namespace Fred

#endif//INFO_NSSET_COMPARE_H_
