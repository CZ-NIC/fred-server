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
 *  domain history info output structure
 */

#ifndef INFO_DOMAIN_OUTPUT_H_
#define INFO_DOMAIN_OUTPUT_H_

#include <string>

#include <boost/date_time/posix_time/ptime.hpp>

#include "util/db/nullable.h"
#include "util/printable.h"
#include "info_domain_data.h"

namespace Fred
{

    /**
    * Element of domain info data.
    */
    struct InfoDomainOutput : public Util::Printable
    {
        InfoDomainData info_domain_data;/**< data of the domain */

        boost::posix_time::ptime utc_timestamp;/**< timestamp of getting the domain data in UTC */

        Nullable<unsigned long long> next_historyid; /**< next historyid of the domain history*/
        boost::posix_time::ptime history_valid_from;/**< history data valid from time, in local time zone viz @ref local_timestamp_pg_time_zone_name */
        Nullable<boost::posix_time::ptime> history_valid_to;/**< history data valid to time in local time zone viz @ref local_timestamp_pg_time_zone_name, null means open end */
        Nullable<unsigned long long> logd_request_id; /**< id of the request that changed domain data*/

        /**
        * Empty constructor of the domain history data structure.
        */
        InfoDomainOutput()
        {}

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

        /**
        * Equality of the domain info data structure operator. Compares only InfoDomainData member.
        * @param rhs is right hand side of the domain data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoDomainOutput& rhs) const;

        /**
        * Inequality of the domain info data structure operator. Compares only InfoDomainData member.
        * @param rhs is right hand side of the domain data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoDomainOutput& rhs) const;
    };

}//namespace Fred

#endif//INFO_DOMAIN_OUTPUT_H_
