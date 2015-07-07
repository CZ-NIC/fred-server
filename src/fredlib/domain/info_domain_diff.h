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
 *  domain info data diff
 */

#ifndef INFO_DOMAIN_DIFF_H_
#define INFO_DOMAIN_DIFF_H_

#include <string>
#include <vector>
#include <utility>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "info_domain_data.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"


namespace Fred
{
    /**
     * Diff of domain data.
     * Data of the domain difference with the same members as domain data but in optional pairs. Optional pair member is set in case of difference in compared domain data.
     */
    struct InfoDomainDiff : public Util::Printable
    {
        template <class T> struct DiffMemeber { typedef Optional<std::pair<T,T> > Type;};

        DiffMemeber<unsigned long long>::Type crhistoryid;/**< first historyid of domain history*/
        DiffMemeber<unsigned long long>::Type historyid;/**< last historyid of domain history*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type delete_time; /**< domain delete time in set local zone*/
        DiffMemeber<std::string>::Type fqdn;/**< fully qualified domain name */
        DiffMemeber<std::string>::Type roid;/**< registry object identifier of the domain */
        DiffMemeber<std::string>::Type sponsoring_registrar_handle;/**< registrar administering the domain */
        DiffMemeber<std::string>::Type create_registrar_handle;/**< registrar that created the domain */
        DiffMemeber<Nullable<std::string> >::Type update_registrar_handle;/**< registrar which last time changed the domain */
        DiffMemeber<boost::posix_time::ptime>::Type creation_time;/**< creation time of the domain in set local zone*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type update_time; /**< last update time of the domain in set local zone*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type transfer_time; /**<last transfer time in set local zone*/
        DiffMemeber<std::string>::Type authinfopw;/**< password for transfer */

        DiffMemeber<Fred::ObjectIdHandlePair>::Type registrant; /**< registrant contact id and handle, owner of domain*/
        DiffMemeber<Nullable<ObjectIdHandlePair> >::Type nsset;/**< nsset id and handle or NULL if missing */
        DiffMemeber<Nullable<ObjectIdHandlePair> >::Type keyset;/**< keyset id and handle or NULL if missing */
        DiffMemeber<boost::gregorian::date>::Type expiration_date;/**< domain expiration local date */
        DiffMemeber<std::vector<ObjectIdHandlePair> >::Type admin_contacts;/**< list of administrating contact handles */
        DiffMemeber<Nullable<ENUMValidationExtension> >::Type enum_domain_validation;/**< ENUM domain validation extension info */
        DiffMemeber<Fred::ObjectIdHandlePair>::Type zone;/**< zone id and fqdn */

        DiffMemeber<unsigned long long>::Type id;/**< id of the domain object*/



        /**
        * Constructor of the domain data diff structure.
        */
        InfoDomainDiff();

        /**
        * Get names of set fields.
        * @return string names of fields that actually changed
        */
        std::set<std::string> changed_fields() const;

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
     * Diff data of the domain.
     * @param first
     * @param second
     * @return diff of given domain
     */
    InfoDomainDiff diff_domain_data(const InfoDomainData& first, const InfoDomainData& second);

}//namespace Fred

#endif//INFO_DOMAIN_DIFF_H_
