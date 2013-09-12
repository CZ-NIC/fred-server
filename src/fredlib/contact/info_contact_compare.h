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
 *  comparison of contact info
 */

#ifndef INFO_CONTACT_COMPARE_H_
#define INFO_CONTACT_COMPARE_H_

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/info_contact_history.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    /**
    * Equality of the contact data with contact history data operator.
    * @param lhs is the contact data
    * @param rhs is the contact history data
    * @return true if equal, false if not
    */
    bool operator==(const InfoContactOutput& lhs, const InfoContactHistoryOutput& rhs);

    /**
    * Equality of contact history data with the contact data operator.
    * @param lhs is the contact history data
    * @param rhs is the contact data
    * @return true if equal, false if not
    */
    bool operator==(const InfoContactHistoryOutput& lhs, const InfoContactOutput& rhs);

    /**
    * Inequality of contact data with the contact history data operator.
    * @param lhs is the contact data
    * @param rhs is the contact history data
    * @return true if not equal, false if equal
    */
    bool operator!=(const InfoContactOutput& lhs, const InfoContactHistoryOutput& rhs);

    /**
    * Inequality of contact history data with the contact data operator.
    * @param lhs is the contact history data
    * @param rhs is the contact data
    * @return true if not equal, false if equal
    */
    bool operator!=(const InfoContactHistoryOutput& lhs, const InfoContactOutput& rhs);

}//namespace Fred

#endif//INFO_CONTACT_COMPARE_H_
