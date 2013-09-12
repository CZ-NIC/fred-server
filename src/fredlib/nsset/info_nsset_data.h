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
 *  common nsset info data
 */

#ifndef INFO_NSSET_DATA_H_
#define INFO_NSSET_DATA_H_

#include <string>
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/nsset/nsset_dns_host.h"

namespace Fred
{
    /**
     * Common data of nsset.
     * Current or history state of the nsset.
     */
    struct InfoNssetData
    {
        unsigned long long crhistoryid;/**< first historyid of nsset history */
        unsigned long long historyid;/**< last historyid of nsset history */
        Nullable<boost::posix_time::ptime> delete_time; /**< nsset delete time in UTC */
        std::string handle;/**< nsset handle */
        std::string roid;/**< registry object identifier of the nsset */
        std::string sponsoring_registrar_handle;/**< registrar administering the nsset */
        std::string create_registrar_handle;/**< registrar that created the nsset */
        Nullable<std::string> update_registrar_handle;/**< registrar which last time changed the nsset */
        boost::posix_time::ptime creation_time;/**< creation time of the nsset in UTC*/
        Nullable<boost::posix_time::ptime> update_time; /**< last update time of the nsset in UTC*/
        Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in UTC*/
        std::string authinfopw;/**< password for transfer */
        Nullable<short> tech_check_level; /**< nsset level of technical checks */
        std::vector<DnsHost> dns_hosts; /**< DNS hosts */
        std::vector<std::string> tech_contacts;/**< list of technical contact handles */

    private:
        bool print_diff_;/**< whether to print debug diff made by nsset comparison operators @ref operator==  and @ref operator!=*/
    public:

        /**
        * Constructor of the nsset data structure.
        */
        InfoNssetData();

        /**
        * Equality of the nsset data structure operator.
        * @param rhs is right hand side of the nsset data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoNssetData& rhs) const;

        /**
        * Inequality of the nsset data structure operator.
        * @param rhs is right hand side of the nsset data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoNssetData& rhs) const;

        /**
        * Set comparison operators to print debug diff.
        * @param print_diff is value set to @ref print_diff_ attribute
        */
        void set_diff_print(bool print_diff = true);

    };

}//namespace Fred

#endif//INFO_NSSET_DATA_H_
