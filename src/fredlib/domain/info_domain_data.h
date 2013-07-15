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
 *  @file info_domain_data.h
 *  common domain info data
 */

#ifndef INFO_DOMAIN_DATA_H_
#define INFO_DOMAIN_DATA_H_

#include <string>
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "fredlib/domain/enum_validation_extension.h"

namespace Fred
{
    struct InfoDomainData
    {
        std::string roid;//domain identifier
        std::string fqdn;//domain name
        std::string registrant_handle;//domain owner
        Nullable<std::string> nsset_handle;//nsset might not be set
        Nullable<std::string> keyset_handle;//keyset might not be set
        std::string sponsoring_registrar_handle;//registrar which have right for change
        std::string create_registrar_handle;//registrar which created domain
        Nullable<std::string> update_registrar_handle;//registrar which last time changed domain
        boost::posix_time::ptime creation_time;//time of domain creation
        Nullable<boost::posix_time::ptime> update_time; //last update time
        Nullable<boost::posix_time::ptime> transfer_time; //last transfer time
        boost::gregorian::date expiration_date; //domain expiration date
        std::string authinfopw;//password for domain transfer
        std::vector<std::string> admin_contacts;//list of administrative contacts
        Nullable<ENUMValidationExtension > enum_domain_validation;//enum domain validation info
        boost::posix_time::ptime outzone_time; //domain outzone time
        boost::posix_time::ptime cancel_time; //domain cancel time
        Nullable<boost::posix_time::ptime> delete_time; //domain delete time
        unsigned long long historyid;//last historyid
        unsigned long long crhistoryid;//first historyid

    private:
        bool print_diff_;
    public:

        InfoDomainData();
        bool operator==(const InfoDomainData& rhs) const;
        bool operator!=(const InfoDomainData& rhs) const;

        void set_diff_print(bool print_diff = true);

    };

}//namespace Fred

#endif//INFO_DOMAIN_DATA_H_
