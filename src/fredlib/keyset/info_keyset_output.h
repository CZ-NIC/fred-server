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
 *  keyset info
 */

#ifndef INFO_KEYSET_OUTPUT_H_
#define INFO_KEYSET_OUTPUT_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "util/db/nullable.h"
#include "info_keyset_data.h"
#include "util/printable.h"
namespace Fred
{

    /**
    * Element of keyset info data.
    */
    struct InfoKeysetOutput : public Util::Printable
    {
        InfoKeysetData info_keyset_data;/**< data of the keyset */

        boost::posix_time::ptime utc_timestamp;/**< timestamp of getting the keyset data in UTC */
        boost::posix_time::ptime local_timestamp;/**< timestamp of getting the keyset data in local time zone viz @ref local_timestamp_pg_time_zone_name */

        Nullable<unsigned long long> next_historyid; /**< next historyid of the keyset history*/
        boost::posix_time::ptime history_valid_from;/**< history data valid from time in local time zone viz @ref local_timestamp_pg_time_zone_name */
        Nullable<boost::posix_time::ptime> history_valid_to;/**< history data valid to time in local time zone viz @ref local_timestamp_pg_time_zone_name, null means open end */
        Nullable<unsigned long long> logd_request_id; /**< id of the request that changed keyset data*/

        /**
        * Empty constructor of the keyset info data structure.
        */
        InfoKeysetOutput()
        {}

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

        /**
        * Equality of the keyset info data structure operator. Compares only InfoKeysetData member.
        * @param rhs is right hand side of the keyset data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoKeysetOutput& rhs) const;

        /**
        * Inequality of the keyset info data structure operator. Compares only InfoKeysetData member.
        * @param rhs is right hand side of the keyset data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoKeysetOutput& rhs) const;
    };

}//namespace Fred

#endif//INFO_KEYSET_OUTPUT_H_
