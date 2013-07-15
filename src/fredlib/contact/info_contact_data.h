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
 *  @file info_contact_data.h
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


namespace Fred
{
    struct InfoContactData
    {
        unsigned long long crhistoryid;//first historyid
        unsigned long long historyid;//last historyid
        Nullable<boost::posix_time::ptime> delete_time; //keyset delete time
        std::string handle;//contact identifier
        std::string roid;//contact identifier
        std::string sponsoring_registrar_handle;//registrar which have right for change
        std::string create_registrar_handle;//registrar which created contact
        Nullable<std::string> update_registrar_handle;//registrar which last time changed contact
        boost::posix_time::ptime creation_time;//time of creation
        Nullable<boost::posix_time::ptime> update_time; //last update time
        Nullable<boost::posix_time::ptime> transfer_time; //last transfer time
        std::string authinfopw;//password for transfer
        Nullable<std::string> name;
        Nullable<std::string> organization;
        Nullable<std::string> street1;
        Nullable<std::string> street2;
        Nullable<std::string> street3;
        Nullable<std::string> city;
        Nullable<std::string> stateorprovince;
        Nullable<std::string> postalcode;
        Nullable<std::string> country;
        Nullable<std::string> telephone;
        Nullable<std::string> fax;
        Nullable<std::string> email;
        Nullable<std::string> notifyemail;
        Nullable<std::string> vat;
        Nullable<std::string> ssntype;
        Nullable<std::string> ssn;
        Nullable<bool> disclosename;
        Nullable<bool> discloseorganization;
        Nullable<bool> discloseaddress;
        Nullable<bool> disclosetelephone;
        Nullable<bool> disclosefax;
        Nullable<bool> discloseemail;
        Nullable<bool> disclosevat;
        Nullable<bool> discloseident;
        Nullable<bool> disclosenotifyemail;

    private:
        bool print_diff_;
    public:

        InfoContactData();
        bool operator==(const InfoContactData& rhs) const;
        bool operator!=(const InfoContactData& rhs) const;
        void set_diff_print(bool print_diff = true);
    };

}//namespace Fred

#endif//INFO_CONTACT_DATA_H_
