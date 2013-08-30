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
 *  @file info_nsset.cc
 *  nsset info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/nsset/info_nsset.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    InfoNsset::InfoNsset(const std::string& handle)
    : handle_(handle)
    , lock_(false)
    {}

    InfoNsset& InfoNsset::set_lock(bool lock)//set lock object_registry row
    {
        lock_ = lock;
        return *this;
    }

    InfoNssetOutput InfoNsset::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        InfoNssetOutput nsset_info_output;

        try
        {
            //info about nsset
            unsigned long long nsset_id = 0;
            {
                Database::Result res = ctx.get_conn().exec_params(std::string(
                "SELECT (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp " // utc timestamp 0
                " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp " // local zone timestamp 1
                " , nobr.crhistoryid "//first historyid 2
                " , nobr.historyid " // last historyid 3
                " , nobr.erdate "// nsset delete time 4
                " , nobr.id,nobr.name,nobr.roid " //nsset 5-7
                " , o.clid,clr.handle " //sponzoring registrar 8-9
                " , nobr.crid, crr.handle "//creating registrar 10-11
                " , o.upid, upr.handle " //updated by registrar 12-13
                " , nobr.crdate,o.trdate,o.update "//registration dates 14-16
                " , o.authinfopw "//authinfo 17
                " , n.checklevel "//tech check level 18
                " FROM object_registry nobr "
                " JOIN nsset n ON nobr.id=n.id "
                " JOIN object o ON n.id=o.id "
                " JOIN registrar clr ON clr.id = o.clid "
                " JOIN registrar crr ON crr.id = nobr.crid "
                " LEFT JOIN registrar upr ON upr.id = o.upid "
                " WHERE nobr.name=UPPER($2::text) AND nobr.erdate IS NULL "
                " AND nobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text)")
                + (lock_ ? std::string(" FOR UPDATE OF nobr") : std::string(""))
                , Database::query_param_list(local_timestamp_pg_time_zone_name)(handle_));

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_nsset_handle(handle_));
                }
                if (res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get nsset"));
                }

                nsset_info_output.utc_timestamp = res[0][0].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][0]));// utc timestamp

                nsset_info_output.local_timestamp = res[0][1].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][1]));//local zone timestamp

                nsset_info_output.info_nsset_data.crhistoryid = static_cast<unsigned long long>(res[0][2]);//nobr.crhistoryid

                nsset_info_output.info_nsset_data.historyid = static_cast<unsigned long long>(res[0][3]);//nobr.historyid

                nsset_info_output.info_nsset_data.delete_time = res[0][4].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][4])));//nobr.erdate

                nsset_id = static_cast<unsigned long long>(res[0][5]);//nobr.id

                nsset_info_output.info_nsset_data.handle = static_cast<std::string>(res[0][6]);//nobr.name

                nsset_info_output.info_nsset_data.roid = static_cast<std::string>(res[0][7]);//nobr.roid

                nsset_info_output.info_nsset_data.sponsoring_registrar_handle = static_cast<std::string>(res[0][9]);//clr.handle

                nsset_info_output.info_nsset_data.create_registrar_handle = static_cast<std::string>(res[0][11]);//crr.handle

                nsset_info_output.info_nsset_data.update_registrar_handle = res[0][13].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[0][13]));//upr.handle

                nsset_info_output.info_nsset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[0][14]));//nobr.crdate

                nsset_info_output.info_nsset_data.transfer_time = res[0][15].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][15])));//o.trdate

                nsset_info_output.info_nsset_data.update_time = res[0][16].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][16])));//o.update

                nsset_info_output.info_nsset_data.authinfopw = static_cast<std::string>(res[0][17]);//o.authinfopw

                nsset_info_output.info_nsset_data.tech_check_level = res[0][18].isnull() ? Nullable<short>()
                : Nullable<short>(static_cast<short>(res[0][18]));//n.checklevel
            }

            //list of dns hosts
            {
                Database::Result hres = ctx.get_conn().exec_params(
                "SELECT h.nssetid,h.id, h.fqdn "
                " FROM object_registry nobr "
                " JOIN enum_object_type neot ON neot.id = nobr.type "
                " JOIN host h ON nobr.id = h.nssetid "
                " WHERE nobr.name=UPPER($1::text) AND nobr.erdate IS NULL AND neot.name='nsset'::text "
                " ORDER BY h.fqdn "
                , Database::query_param_list(handle_));

                nsset_info_output.info_nsset_data.dns_hosts.reserve(hres.size());//alloc
                for(Database::Result::size_type i = 0; i < hres.size(); ++i)
                {
                    unsigned long long dns_host_id = static_cast<unsigned long long>(hres[i][1]);//h.id
                    std::string dns_host_fqdn = static_cast<std::string>(hres[i][2]);//h.fqdn

                    Database::Result ipres = ctx.get_conn().exec_params(
                    "SELECT him.ipaddr "
                    " FROM host_ipaddr_map him WHERE him.hostid = $1::bigint "
                    " ORDER BY him.ipaddr "
                    , Database::query_param_list(dns_host_id));

                    std::vector<std::string> dns_host_ips;

                    dns_host_ips.reserve(ipres.size());

                    for(Database::Result::size_type j = 0; j < ipres.size(); ++j)
                    {
                        std::string dns_host_ipaddr = static_cast<std::string>(ipres[j][0]);//him.ipaddr
                        dns_host_ips.push_back(dns_host_ipaddr);
                    }//for dns host ip
                    nsset_info_output.info_nsset_data.dns_hosts.push_back(DnsHost(dns_host_fqdn, dns_host_ips));

                }//for dns host
            }//list of dns hosts

            //list of tech contacts
            {
                Database::Result result = ctx.get_conn().exec_params(
                        "SELECT cobr.name "
                        " FROM nsset_contact_map ncm "
                        " JOIN object_registry cobr ON ncm.contactid = cobr.id AND cobr.erdate IS NULL "
                        " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                        " WHERE ncm.nssetid = $1::bigint "
                        " ORDER BY cobr.name "
                , Database::query_param_list(nsset_id));

                nsset_info_output.info_nsset_data.tech_contacts.reserve(result.size());
                for(Database::Result::size_type i = 0; i < result.size(); ++i)
                {
                    nsset_info_output.info_nsset_data.tech_contacts.push_back(
                    static_cast<std::string>(result[i][0]));
                }
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_info_output;
    }//InfoNsset::exec

    std::ostream& operator<<(std::ostream& os, const InfoNsset& i)
    {
        return os << "#InfoNsset handle: " << i.handle_
            << " lock: " << i.lock_;
    }

    std::string InfoNsset::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred

