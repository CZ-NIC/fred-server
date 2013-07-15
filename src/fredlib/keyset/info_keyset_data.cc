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
 *  @file info_keyset_data.cc
 *  common keyset info data
 */

#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/keyset/info_keyset_data.h"

namespace Fred
{

    InfoKeysetData::InfoKeysetData()
    : crhistoryid(0)
    , historyid(0)
    , print_diff_(false)
    {}

    bool InfoKeysetData::operator==(const InfoKeysetData& rhs) const
    {
        if(print_diff_) std::cout << "\nInfoKeysetData::operator== print_diff\n" << std::endl;

        bool result_roid = (roid.compare(rhs.roid) == 0);
        if(print_diff_ && !result_roid) std::cout << "roid: " << roid << " != "<< rhs.roid << std::endl;

        bool result_handle = (boost::algorithm::to_lower_copy(handle).compare(boost::algorithm::to_lower_copy(rhs.handle)) == 0);
        if(print_diff_ && !result_handle) std::cout << "handle: " << handle << " != "<< rhs.handle << std::endl;

        bool result_sponsoring_registrar_handle = (boost::algorithm::to_upper_copy(sponsoring_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.sponsoring_registrar_handle)) == 0);
        if(print_diff_ && !result_sponsoring_registrar_handle) std::cout << "sponsoring_registrar_handle: " << sponsoring_registrar_handle << " != "<< rhs.sponsoring_registrar_handle << std::endl;

        bool result_create_registrar_handle = (boost::algorithm::to_upper_copy(create_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.create_registrar_handle)) == 0);
        if(print_diff_ && !result_create_registrar_handle) std::cout << "create_registrar_handle: " << create_registrar_handle << " != "<< rhs.create_registrar_handle << std::endl;

        bool result_creation_time = (creation_time == rhs.creation_time);
        if(print_diff_ && !result_creation_time) std::cout << "creation_time: " << creation_time << " != "<< rhs.creation_time << std::endl;

        bool result_authinfopw = (authinfopw.compare(rhs.authinfopw) == 0);
        if(print_diff_ && !result_authinfopw) std::cout << "authinfopw: " << authinfopw << " != "<< rhs.authinfopw << std::endl;


        bool result_historyid = (historyid == rhs.historyid);
        if(print_diff_ && !result_historyid) std::cout << "historyid: " << historyid << " != "<< rhs.historyid << std::endl;

        bool result_crhistoryid = (crhistoryid == rhs.crhistoryid);
        if(print_diff_ && !result_crhistoryid) std::cout << "crhistoryid: " << crhistoryid << " != "<< rhs.crhistoryid << std::endl;

        bool result_update_registrar_handle = (update_registrar_handle.isnull() == rhs.update_registrar_handle.isnull());
        if(!update_registrar_handle.isnull() && !rhs.update_registrar_handle.isnull())
        {
            result_update_registrar_handle = (boost::algorithm::to_upper_copy(std::string(update_registrar_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.update_registrar_handle))) == 0);
        }
        if(print_diff_ && !result_update_registrar_handle) std::cout << "update_registrar_handle: " << std::string(update_registrar_handle) << " != "<< std::string(rhs.update_registrar_handle) << std::endl;


        bool result_update_time = (update_time.isnull() == rhs.update_time.isnull());
        if(!update_time.isnull() && !rhs.update_time.isnull())
        {
            result_update_time = (boost::posix_time::ptime(update_time) == boost::posix_time::ptime(rhs.update_time));
        }
        if(print_diff_ && !result_update_time) std::cout << "update_time: " << update_time << " != "<< rhs.update_time << std::endl;

        bool result_transfer_time = (transfer_time.isnull() == rhs.transfer_time.isnull());
        if(!transfer_time.isnull() && !rhs.transfer_time.isnull())
        {
            result_transfer_time = (boost::posix_time::ptime(transfer_time) == boost::posix_time::ptime(rhs.transfer_time));
        }
        if(print_diff_ && !result_transfer_time) std::cout << "transfer_time: " << transfer_time << " != "<< rhs.transfer_time << std::endl;

        std::set<DnsKey> lhs_dns_keys;
        for(std::vector<DnsKey>::size_type i = 0
            ; i != dns_keys.size(); ++i)
        {
            lhs_dns_keys.insert(dns_keys[i]);
        }

        std::set<DnsKey> rhs_dns_keys;
        for(std::vector<DnsKey>::size_type i = 0
            ; i != rhs.dns_keys.size(); ++i)
        {
            rhs_dns_keys.insert(rhs.dns_keys[i]);
        }

        bool result_dns_keys = (lhs_dns_keys == rhs_dns_keys);
        if(print_diff_ && !result_dns_keys)
        {
            std::set<DnsKey> lhs_rhs;
            std::set<DnsKey> rhs_lhs;

            std::set_difference(lhs_dns_keys.begin(), lhs_dns_keys.end()
                , rhs_dns_keys.begin(), rhs_dns_keys.end()
                , std::inserter(lhs_rhs, lhs_rhs.end()));

            std::set_difference(rhs_dns_keys.begin(), rhs_dns_keys.end()
                , lhs_dns_keys.begin(), lhs_dns_keys.end()
                , std::inserter(rhs_lhs, rhs_lhs.end()));

            std::cout << "dns_keys differ\nlhs - rhs: ";
            std::copy(lhs_rhs.begin(),lhs_rhs.end(),std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << "\nrhs - lhs: ";
            std::copy(rhs_lhs.begin(),rhs_lhs.end(),std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << std::endl;
        }

        std::set<std::string> lhs_tech_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != tech_contacts.size(); ++i)
        {
            lhs_tech_contacts.insert(boost::algorithm::to_upper_copy(tech_contacts[i]));
        }

        std::set<std::string> rhs_tech_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != rhs.tech_contacts.size(); ++i)
        {
            rhs_tech_contacts.insert(boost::algorithm::to_upper_copy(rhs.tech_contacts[i]));
        }

        bool result_tech_contacts = (lhs_tech_contacts == rhs_tech_contacts);
        if(print_diff_ && !result_tech_contacts)
        {
            std::set<std::string> lhs_rhs;
            std::set<std::string> rhs_lhs;

            std::set_difference(lhs_tech_contacts.begin(), lhs_tech_contacts.end()
                , rhs_tech_contacts.begin(), rhs_tech_contacts.end()
                , std::inserter(lhs_rhs, lhs_rhs.end()));

            std::set_difference(rhs_tech_contacts.begin(), rhs_tech_contacts.end()
                , lhs_tech_contacts.begin(), lhs_tech_contacts.end()
                , std::inserter(rhs_lhs, rhs_lhs.end()));

            std::cout << "tech_contacts differ\nlhs - rhs: ";
            std::copy(lhs_rhs.begin(),lhs_rhs.end(),std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << "\nrhs - lhs: ";
            std::copy(rhs_lhs.begin(),rhs_lhs.end(),std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << std::endl;
        }

        bool result_delete_time = (delete_time.isnull() == rhs.delete_time.isnull());
        if(!delete_time.isnull() && !rhs.delete_time.isnull())
        {
            result_delete_time = (boost::posix_time::ptime(delete_time) == boost::posix_time::ptime(rhs.delete_time));
        }
        if(print_diff_ && !result_delete_time) std::cout << "delete_time: " << delete_time << " != "<< rhs.delete_time << std::endl;

        return  result_roid
                && result_handle
                && result_sponsoring_registrar_handle
                && result_create_registrar_handle
                && result_creation_time
                && result_authinfopw
                && result_historyid
                && result_crhistoryid
                && result_update_registrar_handle
                && result_update_time
                && result_transfer_time
                && result_dns_keys
                && result_tech_contacts
                && result_delete_time
                ;
    }

    bool InfoKeysetData::operator!=(const InfoKeysetData& rhs) const
    {
        return !this->operator ==(rhs);
    }

    void InfoKeysetData::set_diff_print(bool print_diff)
    {
        print_diff_ = print_diff;
    }

}//namespace Fred

