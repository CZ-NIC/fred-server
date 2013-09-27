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
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

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
        Nullable<boost::posix_time::ptime> delete_time; /**< contact delete time in UTC*/
        std::string handle;/**< contact handle */
        std::string roid;/**< registry object identifier of the contact */
        std::string sponsoring_registrar_handle;/**< registrar administering the contact */
        std::string create_registrar_handle;/**< registrar that created the contact */
        Nullable<std::string> update_registrar_handle;/**< registrar which last time changed the contact */
        boost::posix_time::ptime creation_time;/**< creation time of the contact in UTC*/
        Nullable<boost::posix_time::ptime> update_time; /**< last update time of the contact in UTC*/
        Nullable<boost::posix_time::ptime> transfer_time; /**<last transfer time in UTC*/
        std::string authinfopw;/**< password for transfer */
        Nullable<std::string> name ;/**< name of contact person */
        Nullable<std::string> organization;/**< full trade name of organization */
        Nullable<std::string> street1;/**< part of address */
        Nullable<std::string> street2;/**< part of address */
        Nullable<std::string> street3;/**< part of address*/
        Nullable<std::string> city;/**< part of address - city */
        Nullable<std::string> stateorprovince;/**< part of address - region */
        Nullable<std::string> postalcode;/**< part of address - postal code */
        Nullable<std::string> country;/**< two character country code or country name */
        Nullable<std::string> telephone;/**<  telephone number */
        Nullable<std::string> fax;/**< fax number */
        Nullable<std::string> email;/**< e-mail address */
        Nullable<std::string> notifyemail;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
        Nullable<std::string> vat;/**< taxpayer identification number */
        Nullable<std::string> ssntype;/**< type of identification from enumssntype table */
        Nullable<std::string> ssn;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
        Nullable<bool> disclosename;/**< whether to reveal contact name */
        Nullable<bool> discloseorganization;/**< whether to reveal organization */
        Nullable<bool> discloseaddress;/**< whether to reveal address */
        Nullable<bool> disclosetelephone;/**< whether to reveal phone number */
        Nullable<bool> disclosefax;/**< whether to reveal fax number */
        Nullable<bool> discloseemail;/**< whether to reveal email address */
        Nullable<bool> disclosevat;/**< whether to reveal taxpayer identification number */
        Nullable<bool> discloseident;/**< whether to reveal unambiguous identification number */
        Nullable<bool> disclosenotifyemail;/**< whether to reveal notify email */

    private:
        bool print_diff_;/**< whether to print debug diff made by contact comparison operators @ref operator==  and @ref operator!=*/
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
        * Set comparison operators to print debug diff.
        * @param print_diff is value set to @ref print_diff_ attribute
        */
        void set_diff_print(bool print_diff = true);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

}//namespace Fred

#endif//INFO_CONTACT_DATA_H_
