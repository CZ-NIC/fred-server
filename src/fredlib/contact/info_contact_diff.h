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

#include <string>
#include <vector>
#include <set>
#include <utility>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/contact/info_contact_data.h"

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
        Optional<std::pair<unsigned long long,unsigned long long> > crhistoryid;/**< first historyid of contact history*/
        Optional<std::pair<unsigned long long,unsigned long long> > historyid;/**< last historyid of contact history*/
        Optional<std::pair<Nullable<boost::posix_time::ptime>, Nullable<boost::posix_time::ptime> > > delete_time; /**< contact delete time in set local zone*/
        Optional<std::pair<std::string,std::string> > handle;/**< contact handle */
        Optional<std::pair<std::string,std::string> > roid;/**< registry object identifier of the contact */
        Optional<std::pair<std::string,std::string> > sponsoring_registrar_handle;/**< registrar administering the contact */
        Optional<std::pair<std::string,std::string> > create_registrar_handle;/**< registrar that created the contact */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > update_registrar_handle;/**< registrar which last time changed the contact */
        Optional<std::pair<boost::posix_time::ptime,boost::posix_time::ptime> > creation_time;/**< creation time of the contact in set local zone*/
        Optional<std::pair<Nullable<boost::posix_time::ptime>, Nullable<boost::posix_time::ptime> > > update_time; /**< last update time of the contact in set local zone*/
        Optional<std::pair<Nullable<boost::posix_time::ptime>, Nullable<boost::posix_time::ptime> > > transfer_time; /**<last transfer time in set local zone*/
        Optional<std::pair<std::string,std::string> > authinfopw;/**< password for transfer */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > name ;/**< name of contact person */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > organization;/**< full trade name of organization */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > street1;/**< part of address */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > street2;/**< part of address */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > street3;/**< part of address*/
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > city;/**< part of address - city */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > stateorprovince;/**< part of address - region */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > postalcode;/**< part of address - postal code */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > country;/**< two character country code or country name */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > telephone;/**<  telephone number */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > fax;/**< fax number */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > email;/**< e-mail address */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > notifyemail;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > vat;/**< taxpayer identification number */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > ssntype;/**< type of identification from enumssntype table */
        Optional<std::pair<Nullable<std::string>, Nullable<std::string> > > ssn;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > disclosename;/**< whether to reveal contact name */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > discloseorganization;/**< whether to reveal organization */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > discloseaddress;/**< whether to reveal address */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > disclosetelephone;/**< whether to reveal phone number */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > disclosefax;/**< whether to reveal fax number */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > discloseemail;/**< whether to reveal email address */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > disclosevat;/**< whether to reveal taxpayer identification number */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > discloseident;/**< whether to reveal unambiguous identification number */
        Optional<std::pair<Nullable<bool>, Nullable<bool> > > disclosenotifyemail;/**< whether to reveal notify email */
        Optional<std::pair<unsigned long long,unsigned long long> > id;/**< id of the contact object*/

        /**
        * Constructor of the contact data diff structure.
        */
        InfoContactDiff();

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

}//namespace Fred

#endif//INFO_CONTACT_DIFF_H_
