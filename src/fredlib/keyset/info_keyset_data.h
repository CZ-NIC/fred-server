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
 *  common keyset info data
 */

#ifndef INFO_KEYSET_DATA_H_
#define INFO_KEYSET_DATA_H_

#include <string>

#include <boost/date_time/posix_time/ptime.hpp>

#include "util/db/nullable.h"
#include "util/printable.h"
#include "src/fredlib/object/object_id_handle_pair.h"

#include "keyset_dns_key.h"

namespace Fred
{

    /**
     * Common data of keyset.
     * Current or history state of the keyset.
     */
    struct InfoKeysetData : public Util::Printable
    {
        unsigned long long crhistoryid;/**< first historyid of keyset history */
        unsigned long long historyid;/**< last historyid of keyset history */
        unsigned long long id;/**< id of the keyset object*/
        Nullable<boost::posix_time::ptime> delete_time; /**< keyset delete time in local time zone viz @ref local_timestamp_pg_time_zone_name */
        std::string handle;/**< keyset handle */
        std::string roid;/**< registry object identifier of the keyset */
        std::string sponsoring_registrar_handle;/**< registrar administering the keyset */
        std::string create_registrar_handle;/**< registrar that created the keyset */
        Nullable<std::string> update_registrar_handle;/**< registrar which last time changed the keyset */
        boost::posix_time::ptime creation_time;/**< creation time of the keyset in local time zone viz @ref local_timestamp_pg_time_zone_name*/
        Nullable<boost::posix_time::ptime> update_time; /**< last update time of the keyset in local time zone viz @ref local_timestamp_pg_time_zone_name*/
        Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in local time zone viz @ref local_timestamp_pg_time_zone_name*/
        std::string authinfopw;/**< password for transfer */
        std::vector<DnsKey> dns_keys;/**< DNS keys */
        std::vector<ObjectIdHandlePair> tech_contacts;/**< list of technical contact handles */

        /**
        * Constructor of the keyset data structure.
        */
        InfoKeysetData();

        /**
        * Equality of the keyset data structure operator.
        * @param rhs is right hand side of the keyset data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoKeysetData& rhs) const;

        /**
        * Inequality of the keyset data structure operator.
        * @param rhs is right hand side of the keyset data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoKeysetData& rhs) const;

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

}//namespace Fred

#endif//INFO_KEYSET_DATA_H_
