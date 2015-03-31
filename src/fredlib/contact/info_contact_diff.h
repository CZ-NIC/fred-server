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
 *  contact info data diff
 */

#ifndef INFO_CONTACT_DIFF_H_
#define INFO_CONTACT_DIFF_H_

#include <algorithm>
#include <string>

#include "info_contact_data.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"


namespace Fred
{
    /**
     * Diff of contact data.
     * Data of the contact difference with the same members as contact data but in optional pairs. Optional pair member is set in case of difference in compared contact data.
     */
    struct InfoContactDiff : public Util::Printable
    {
        template <class T> struct DiffMemeber { typedef Optional<std::pair<T,T> > Type;};

        DiffMemeber<unsigned long long>::Type crhistoryid;/**< first historyid of contact history*/
        DiffMemeber<unsigned long long>::Type historyid;/**< last historyid of contact history*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type delete_time; /**< contact delete time in set local zone*/
        DiffMemeber<std::string>::Type handle;/**< contact handle */
        DiffMemeber<std::string>::Type roid;/**< registry object identifier of the contact */
        DiffMemeber<std::string>::Type sponsoring_registrar_handle;/**< registrar administering the contact */
        DiffMemeber<std::string>::Type create_registrar_handle;/**< registrar that created the contact */
        DiffMemeber<Nullable<std::string> >::Type update_registrar_handle;/**< registrar which last time changed the contact */
        DiffMemeber<boost::posix_time::ptime>::Type creation_time;/**< creation time of the contact in set local zone*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type update_time; /**< last update time of the contact in set local zone*/
        DiffMemeber<Nullable<boost::posix_time::ptime> >::Type transfer_time; /**<last transfer time in set local zone*/
        DiffMemeber<std::string>::Type authinfopw;/**< password for transfer */
        DiffMemeber<Nullable<std::string> >::Type name ;/**< name of contact person */
        DiffMemeber<Nullable<std::string> >::Type organization;/**< full trade name of organization */
        DiffMemeber< Nullable< Fred::Contact::PlaceAddress > >::Type place;/**< place address of contact */
        DiffMemeber<Nullable<std::string> >::Type telephone;/**<  telephone number */
        DiffMemeber<Nullable<std::string> >::Type fax;/**< fax number */
        DiffMemeber<Nullable<std::string> >::Type email;/**< e-mail address */
        DiffMemeber<Nullable<std::string> >::Type notifyemail;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
        DiffMemeber<Nullable<std::string> >::Type vat;/**< taxpayer identification number */
        DiffMemeber<Nullable<std::string> >::Type ssntype;/**< type of identification from enumssntype table */
        DiffMemeber<Nullable<std::string> >::Type ssn;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
        DiffMemeber<bool>::Type disclosename;/**< whether to reveal contact name */
        DiffMemeber<bool>::Type discloseorganization;/**< whether to reveal organization */
        DiffMemeber<bool>::Type discloseaddress;/**< whether to reveal address */
        DiffMemeber<bool>::Type disclosetelephone;/**< whether to reveal phone number */
        DiffMemeber<bool>::Type disclosefax;/**< whether to reveal fax number */
        DiffMemeber<bool>::Type discloseemail;/**< whether to reveal email address */
        DiffMemeber<bool>::Type disclosevat;/**< whether to reveal taxpayer identification number */
        DiffMemeber<bool>::Type discloseident;/**< whether to reveal unambiguous identification number */
        DiffMemeber<bool>::Type disclosenotifyemail;/**< whether to reveal notify email */
        DiffMemeber<unsigned long long>::Type id;/**< id of the contact object*/
        DiffMemeber<Fred::ContactAddressList>::Type addresses;/**< additional contact addresses */
        DiffMemeber<Nullable<bool> >::Type warning_letter;/**< contact preference for sending domain expiration letters */

        /**
        * Constructor of the contact data diff structure.
        */
        InfoContactDiff();

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
     * Diff data of the contact.
     * @param first
     * @param second
     * @return diff of given contact
     */
    InfoContactDiff diff_contact_data(const InfoContactData& first, const InfoContactData& second);

}//namespace Fred

#endif//INFO_CONTACT_DIFF_H_
