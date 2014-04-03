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
 *  common contact info data
 */

#ifndef INFO_CONTACT_DATA_H_
#define INFO_CONTACT_DATA_H_

#include <string>

#include <boost/date_time/posix_time/ptime.hpp>

#include "util/db/nullable.h"
#include "util/printable.h"
#include "src/fredlib/contact/place_address.h"

namespace Fred
{
    /**
     * Common data of contact.
     * Current or history state of the contact.
     */
    struct InfoContactData : public Util::Printable
    {
        unsigned long long crhistoryid;/**< first historyid of contact history*/
        unsigned long long historyid;/**< last historyid of contact history*/
        Nullable<boost::posix_time::ptime> delete_time; /**< contact delete time in set local zone*/
        std::string handle;/**< contact handle */
        std::string roid;/**< registry object identifier of the contact */
        std::string sponsoring_registrar_handle;/**< registrar administering the contact */
        std::string create_registrar_handle;/**< registrar that created the contact */
        Nullable<std::string> update_registrar_handle;/**< registrar which last time changed the contact */
        boost::posix_time::ptime creation_time;/**< creation time of the contact in set local zone*/
        Nullable<boost::posix_time::ptime> update_time; /**< last update time of the contact in set local zone*/
        Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in set local zone*/
        std::string authinfopw;/**< password for transfer */
        Nullable<std::string> name ;/**< name of contact person */
        Nullable<std::string> organization;/**< full trade name of organization */
        Nullable< Contact::PlaceAddress > place;/**< place address of contact */
        Nullable<std::string> telephone;/**<  telephone number */
        Nullable<std::string> fax;/**< fax number */
        Nullable<std::string> email;/**< e-mail address */
        Nullable<std::string> notifyemail;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
        Nullable<std::string> vat;/**< taxpayer identification number */
        Nullable<std::string> ssntype;/**< type of identification from enumssntype table */
        Nullable<std::string> ssn;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
        bool disclosename;/**< whether to reveal contact name */
        bool discloseorganization;/**< whether to reveal organization */
        bool discloseaddress;/**< whether to reveal address */
        bool disclosetelephone;/**< whether to reveal phone number */
        bool disclosefax;/**< whether to reveal fax number */
        bool discloseemail;/**< whether to reveal email address */
        bool disclosevat;/**< whether to reveal taxpayer identification number */
        bool discloseident;/**< whether to reveal unambiguous identification number */
        bool disclosenotifyemail;/**< whether to reveal notify email */
        unsigned long long id;/**< id of the contact object*/

    public:
        /**
        * Constructor of the contact data structure.
        */
        InfoContactData();
        /**
        * Equality of the contact data structure operator.
        * @param rhs is right hand side of the contact data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoContactData& rhs) const;

        /**
        * Inequality of the contact data structure operator.
        * @param rhs is right hand side of the contact data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoContactData& rhs) const;

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

}//namespace Fred

#endif//INFO_CONTACT_DATA_H_
