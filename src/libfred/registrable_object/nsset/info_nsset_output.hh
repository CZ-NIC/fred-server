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
 *  nsset info output structure
 */

#ifndef INFO_NSSET_OUTPUT_HH_3B3A6EFB6BB4457A999D7E84BD9FA805
#define INFO_NSSET_OUTPUT_HH_3B3A6EFB6BB4457A999D7E84BD9FA805

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/util/db/nullable.hh"
#include "src/util/printable.hh"
#include "src/libfred/registrable_object/nsset/info_nsset_data.hh"


namespace LibFred
{

    /**
    * Element of nsset info data.
    */
    struct InfoNssetOutput : public Util::Printable
    {
        InfoNssetData info_nsset_data;/**< data of the nsset */

        boost::posix_time::ptime utc_timestamp;/**< timestamp of getting the nsset data in UTC */

        Nullable<unsigned long long> next_historyid; /**< next historyid of the nsset history*/
        boost::posix_time::ptime history_valid_from;/**< history data valid from time */
        Nullable<boost::posix_time::ptime> history_valid_to;/**< history data valid to time, null means open end */
        Nullable<unsigned long long> logd_request_id; /**< id of the request that changed nsset data*/

        /**
        * Empty constructor of the nsset info data structure.
        */
        InfoNssetOutput()
        {}

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

        /**
        * Equality of the nsset info data structure operator. Compares only InfoNssetData member.
        * @param rhs is right hand side of the nsset data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoNssetOutput& rhs) const;

        /**
        * Inequality of the nsset info data structure operator. Compares only InfoNssetData member.
        * @param rhs is right hand side of the nsset data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoNssetOutput& rhs) const;
    };

} // namespace LibFred

#endif
