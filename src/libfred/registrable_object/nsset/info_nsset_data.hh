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

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/util/optional_value.hh"
#include "src/util/db/nullable.hh"
#include "src/util/printable.hh"
#include "src/libfred/object/object_id_handle_pair.hh"

#include "src/libfred/registrable_object/nsset/nsset_dns_host.hh"

namespace LibFred
{
    /**
     * Common data of nsset.
     * Current or history state of the nsset.
     */
    struct InfoNssetData : public Util::Printable
    {
        unsigned long long crhistoryid;/**< first historyid of nsset history */
        unsigned long long historyid;/**< last historyid of nsset history */
        unsigned long long id;/**< id of the nsset object*/
        Nullable<boost::posix_time::ptime> delete_time; /**< nsset delete time in set local zone */
        std::string handle;/**< nsset handle */
        std::string roid;/**< registry object identifier of the nsset */
        std::string sponsoring_registrar_handle;/**< registrar administering the nsset */
        std::string create_registrar_handle;/**< registrar that created the nsset */
        Nullable<std::string> update_registrar_handle;/**< registrar which last time changed the nsset */
        boost::posix_time::ptime creation_time;/**< creation time of the nsset in set local zone*/
        Nullable<boost::posix_time::ptime> update_time; /**< last update time of the nsset in set local zone*/
        Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in set local zone*/
        std::string authinfopw;/**< password for transfer */
        Nullable<short> tech_check_level; /**< nsset level of technical checks */
        std::vector<DnsHost> dns_hosts; /**< DNS hosts */
        std::vector<ObjectIdHandlePair> tech_contacts;/**< list of technical contacts */

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
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

} // namespace LibFred

#endif//INFO_NSSET_DATA_H_
