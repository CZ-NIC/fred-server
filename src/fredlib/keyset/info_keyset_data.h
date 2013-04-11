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
 *  @file info_keyset_data.h
 *  common keyset info data
 */

#ifndef INFO_KEYSET_DATA_H_
#define INFO_KEYSET_DATA_H_

#include <string>
#include <vector>
#include <set>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"

#include "fredlib/keyset/keyset_dns_key.h"

namespace Fred
{
    struct InfoKeysetData
    {
        unsigned long long crhistoryid;//first historyid
        unsigned long long historyid;//last historyid
        Nullable<boost::posix_time::ptime> delete_time; //keyset delete time
        std::string handle;//keyset identifier
        std::string roid;//keyset identifier
        std::string sponsoring_registrar_handle;//registrar which have right for change
        std::string create_registrar_handle;//registrar which created domain
        Nullable<std::string> update_registrar_handle;//registrar which last time changed domain
        boost::posix_time::ptime creation_time;//time of creation
        Nullable<boost::posix_time::ptime> update_time; //last update time
        Nullable<boost::posix_time::ptime> transfer_time; //last transfer time
        std::string authinfopw;//password for transfer
        std::vector<DnsKey> dns_keys; //dns keys
        std::vector<std::string> tech_contacts;//list of technical contacts

    private:
        bool print_diff_;
    public:

        InfoKeysetData();
        bool operator==(const InfoKeysetData& rhs) const;
        bool operator!=(const InfoKeysetData& rhs) const;

        void set_diff_print(bool print_diff = true);

    };

}//namespace Fred

#endif//INFO_KEYSET_DATA_H_
