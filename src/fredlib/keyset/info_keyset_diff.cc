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
 *  keyset info data diff
 */

#include <algorithm>
#include <string>
#include <vector>
#include <set>

#include <boost/algorithm/string.hpp>

#include "util/util.h"
#include "util/is_equal_optional_nullable.h"
#include "info_keyset_diff.h"

namespace Fred
{
    InfoKeysetDiff::InfoKeysetDiff()
    {}

    std::string InfoKeysetDiff::to_string() const
    {
        return Util::format_data_structure("InfoKeysetDiff",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("crhistoryid", crhistoryid.print_quoted()))
        (std::make_pair("historyid", historyid.print_quoted()))
        (std::make_pair("delete_time", delete_time.print_quoted()))
        (std::make_pair("handle", handle.print_quoted()))
        (std::make_pair("roid", roid.print_quoted()))
        (std::make_pair("sponsoring_registrar_handle", sponsoring_registrar_handle.print_quoted()))
        (std::make_pair("create_registrar_handle", create_registrar_handle.print_quoted()))
        (std::make_pair("update_registrar_handle", update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time", creation_time.print_quoted()))
        (std::make_pair("update_time", update_time.print_quoted()))
        (std::make_pair("transfer_time", transfer_time.print_quoted()))
        (std::make_pair("authinfopw", authinfopw.print_quoted()))

        (std::make_pair("dns_keys", dns_keys.print_quoted()))
        (std::make_pair("tech_contacts", tech_contacts.print_quoted()))

        (std::make_pair("id", id.print_quoted()))
        );//format_data_structure InfoKeysetDiff
    }

    bool InfoKeysetDiff::is_empty() const
    {
        return
            !( crhistoryid.isset()
            || historyid.isset()
            || delete_time.isset()
            || handle.isset()
            || roid.isset()
            || sponsoring_registrar_handle.isset()
            || create_registrar_handle.isset()
            || update_registrar_handle.isset()
            || creation_time.isset()
            || update_time.isset()
            || transfer_time.isset()
            || authinfopw.isset()

            || dns_keys.isset()
            || tech_contacts.isset()

            || id.isset()
            );
    }

    InfoKeysetDiff diff_keyset_data(const InfoKeysetData& first, const InfoKeysetData& second)
    {
        Fred::InfoKeysetDiff diff;

        //differing data
        if(first.crhistoryid != second.crhistoryid)
        {
            diff.crhistoryid = std::make_pair(first.crhistoryid,second.crhistoryid);
        }

        if(first.historyid != second.historyid)
        {
            diff.historyid = std::make_pair(first.historyid,second.historyid);
        }

        if(!Util::is_equal(first.delete_time, second.delete_time))
        {
            diff.delete_time = std::make_pair(first.delete_time,second.delete_time);
        }

        if(boost::algorithm::to_upper_copy(first.handle).compare(boost::algorithm::to_upper_copy(second.handle)) != 0)
        {
            diff.handle = std::make_pair(first.handle, second.handle);
        }

        if(first.roid.compare(second.roid) != 0)
        {
            diff.roid = std::make_pair(first.roid,second.roid);
        }

        if(boost::algorithm::to_upper_copy(first.sponsoring_registrar_handle)
            .compare(boost::algorithm::to_upper_copy(second.sponsoring_registrar_handle)) != 0)
        {
            diff.sponsoring_registrar_handle = std::make_pair(first.sponsoring_registrar_handle
                    ,second.sponsoring_registrar_handle);
        }

        if(boost::algorithm::to_upper_copy(first.create_registrar_handle)
        .compare(boost::algorithm::to_upper_copy(second.create_registrar_handle)) != 0)
        {
            diff.create_registrar_handle = std::make_pair(first.create_registrar_handle
                    ,second.create_registrar_handle);
        }

        if(!Util::is_equal_upper(first.update_registrar_handle, second.update_registrar_handle))
        {
            diff.update_registrar_handle = std::make_pair(first.update_registrar_handle
                    ,second.update_registrar_handle);
        }

        if(first.creation_time != second.creation_time)
        {
            diff.creation_time = std::make_pair(first.creation_time,second.creation_time);
        }

        if(!Util::is_equal(first.update_time, second.update_time))
        {
            diff.update_time = std::make_pair(first.update_time
                    ,second.update_time);
        }

        if(!Util::is_equal(first.transfer_time, second.transfer_time))
        {
            diff.transfer_time = std::make_pair(first.transfer_time
                    ,second.transfer_time);
        }

        if(first.authinfopw.compare(second.authinfopw) != 0)
        {
            diff.authinfopw = std::make_pair(first.authinfopw,second.authinfopw);
        }


        std::set<DnsKey> lhs_dns_keys;
        for(std::vector<DnsKey>::size_type i = 0
            ; i != first.dns_keys.size(); ++i)
        {
            lhs_dns_keys.insert(first.dns_keys[i]);
        }

        std::set<DnsKey> rhs_dns_keys;
        for(std::vector<DnsKey>::size_type i = 0
            ; i != second.dns_keys.size(); ++i)
        {
            rhs_dns_keys.insert(second.dns_keys[i]);
        }

        if(lhs_dns_keys != rhs_dns_keys)
        {
            diff.dns_keys = std::make_pair(first.dns_keys,second.dns_keys);
        }


        std::set<std::string> lhs_tech_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != first.tech_contacts.size(); ++i)
        {
            lhs_tech_contacts.insert(boost::algorithm::to_upper_copy(first.tech_contacts[i]));
        }

        std::set<std::string> rhs_tech_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != second.tech_contacts.size(); ++i)
        {
            rhs_tech_contacts.insert(boost::algorithm::to_upper_copy(second.tech_contacts[i]));
        }

        if(lhs_tech_contacts != rhs_tech_contacts)
        {
            diff.tech_contacts = std::make_pair(first.tech_contacts,second.tech_contacts);
        }


        if(first.id != second.id)
        {
            diff.id = std::make_pair(first.id,second.id);
        }

        return diff;
    }

}//namespace Fred
