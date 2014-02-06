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
 *  common domain info data
 */

#ifndef INFO_DOMAIN_DATA_H_
#define INFO_DOMAIN_DATA_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "util/db/nullable.h"
#include "enum_validation_extension.h"
#include "util/printable.h"

namespace Fred
{
    /**
     * Common data of domain.
     * Current or history state of the domain.
     */
    struct InfoDomainData : public Util::Printable
    {
        std::string roid;/**< registry object identifier of domain */
        std::string fqdn;/**< fully qualified domain name */
        std::string registrant_handle;/**< registrant contact handle, owner of domain*/
        Nullable<std::string> nsset_handle;/**< nsset handle or NULL if missing */
        Nullable<std::string> keyset_handle;/**< keyset handle or NULL if missing */
        std::string sponsoring_registrar_handle;/**< handle of registrar administering domain */
        std::string create_registrar_handle;/**< handle of registrar which created domain */
        Nullable<std::string> update_registrar_handle;/**< handle of registrar which last time changed domain*/
        boost::posix_time::ptime creation_time;/**< time of domain creation in UTC*/
        Nullable<boost::posix_time::ptime> update_time; /**< time of last update time in UTC*/
        Nullable<boost::posix_time::ptime> transfer_time; /**< time of last transfer in UTC*/
        boost::gregorian::date expiration_date; /**< domain expiration local date */
        std::string authinfopw;/**< password for domain transfer */
        std::vector<std::string> admin_contacts;/**< list of administrating contact handles */
        Nullable<ENUMValidationExtension > enum_domain_validation;/**< ENUM domain validation extension info */
        boost::posix_time::ptime outzone_time; /**< domain outzone time in regular_day_procedure_zone from table enum_parameters */
        boost::posix_time::ptime cancel_time; /**< domain cancel time in regular_day_procedure_zone from table enum_parameters */
        Nullable<boost::posix_time::ptime> delete_time;/**< domain delete time in UTC*/
        unsigned long long historyid;/**< last historyid of domain history*/
        unsigned long long crhistoryid;/**< first historyid of domain history*/
        unsigned long long id;/**< id of the domain object*/

        /**
        * Constructor of domain data structure.
        */
        InfoDomainData();
        /**
        * Equality of domain data structure operator.
        * @param rhs is right hand side of domain data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoDomainData& rhs) const;
        /**
        * Inequality of the contact data structure operator.
        * @param rhs is right hand side of the contact data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoDomainData& rhs) const;

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

}//namespace Fred

#endif//INFO_DOMAIN_DATA_H_
