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
 *  keyset info data diff
 */

#ifndef INFO_KEYSET_DIFF_H_
#define INFO_KEYSET_DIFF_H_

#include <string>
#include <vector>
#include <set>
#include <utility>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/keyset/info_keyset_data.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"


namespace Fred
{
    /**
     * Diff of keyset data.
     * Data of the keyset difference with the same members as keyset data but in optional pairs. Optional pair member is set in case of difference in compared keyset data.
     */
    struct InfoKeysetDiff : public Util::Printable
    {
        template <class T> struct DiffMemeber { typedef Optional<std::pair<T,T> > Type;};

        DiffMemeber<unsigned long long>::Type crhistoryid;/**< first historyid of keyset history*/
        DiffMemeber<unsigned long long>::Type historyid;/**< last historyid of keyset history*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type delete_time; /**< keyset delete time in set local zone*/
        DiffMemeber<std::string>::Type handle;/**< keyset handle */
        DiffMemeber<std::string>::Type roid;/**< registry object identifier of the keyset */
        DiffMemeber<std::string>::Type sponsoring_registrar_handle;/**< registrar administering the keyset */
        DiffMemeber<std::string>::Type create_registrar_handle;/**< registrar that created the keyset */
        DiffMemeber<Nullable<std::string> >::Type update_registrar_handle;/**< registrar which last time changed the keyset */
        DiffMemeber<boost::posix_time::ptime>::Type creation_time;/**< creation time of the keyset in set local zone*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type update_time; /**< last update time of the keyset in set local zone*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type transfer_time; /**<last transfer time in set local zone*/
        DiffMemeber<std::string>::Type authinfopw;/**< password for transfer */

        DiffMemeber<std::vector<DnsKey> >::Type dns_keys;/**< DNS keys */
        DiffMemeber<std::vector<std::string> >::Type tech_contacts;/**< list of technical contact handles */

        DiffMemeber<unsigned long long>::Type id;/**< id of the keyset object*/

        /**
        * Constructor of the keyset data diff structure.
        */
        InfoKeysetDiff();

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

        /**
        * Check if some data is set into the instance
        * @return false if instance contains differing data and true if not
        */
        bool is_empty() const;
    };

    /**
     * Diff data of the keyset.
     * @param first
     * @param second
     * @return diff of given keyset
     */
    InfoKeysetDiff diff_keyset_data(const InfoKeysetData& first, const InfoKeysetData& second);

}//namespace Fred

#endif//INFO_KEYSET_DIFF_H_
