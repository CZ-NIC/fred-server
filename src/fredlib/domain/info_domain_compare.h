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
 *  comparison of domain info
 */

#ifndef INFO_DOMAIN_COMPARE_H_
#define INFO_DOMAIN_COMPARE_H_

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    /**
    * Equality of the domain data with domain history data operator.
    * @param lhs is the domain data
    * @param rhs is the domain history data
    * @return true if equal, false if not
    */
    bool operator==(const InfoDomainOutput& lhs, const InfoDomainHistoryOutput& rhs);

    /**
    * Equality of domain history data with the domain data operator.
    * @param lhs is the domain history data
    * @param rhs is the domain data
    * @return true if equal, false if not
    */
    bool operator==(const InfoDomainHistoryOutput& lhs, const InfoDomainOutput& rhs);

    /**
    * Inequality of domain data with the domain history data operator.
    * @param lhs is the domain data
    * @param rhs is the domain history data
    * @return true if not equal, false if equal
    */
    bool operator!=(const InfoDomainOutput& lhs, const InfoDomainHistoryOutput& rhs);

    /**
    * Inequality of domain history data with the domain data operator.
    * @param lhs is the domain history data
    * @param rhs is the domain data
    * @return true if not equal, false if equal
    */
    bool operator!=(const InfoDomainHistoryOutput& lhs, const InfoDomainOutput& rhs);

}//namespace Fred

#endif//INFO_DOMAIN_COMPARE_H_
