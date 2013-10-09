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
 *  nsset history info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/nsset/info_nsset_history.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    InfoNssetHistory::InfoNssetHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , lock_(false)
    {}

    InfoNssetHistory::InfoNssetHistory(const std::string& roid)
    : roid_(roid)
    , lock_(false)
    {}

    InfoNssetHistory& InfoNssetHistory::set_history_timestamp(boost::posix_time::ptime history_timestamp)//set history timestamp
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoNssetHistory& InfoNssetHistory::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoNssetOutput> InfoNssetHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_history_res;

        try
        {
            //info about nsset history by roid and optional history timestamp
            if(!roid_.empty())
            {
                //query params
                Database::QueryParams params;
                params.push_back(roid_);
                if (history_timestamp_.isset())
                {
                    params.push_back(history_timestamp_);
                    params.push_back(local_timestamp_pg_time_zone_name);
                }

                Database::Result res = ctx.get_conn().exec_params(
                std::string("SELECT nobr.id, nobr.roid, nobr.name, nobr.erdate "//nsset 0-3
                " , oh.historyid, h.id , h.next, h.valid_from, h.valid_to "//history 4-8
                " , oh.clid, clr.handle "//sponsoring registrar 9-10
                " , nobr.crid, crr.handle "//creating registrar 11-12
                " , oh.upid, upr.handle "//last updated by registrar 13-14
                " , nobr.crdate, oh.trdate, oh.update "//registration dates 15-17
                " , nh.checklevel "//checklevel 18
                " , oh.authinfopw " //transfer passwd 19
                " , nobr.crhistoryid "//first historyid 20
                " , h.request_id "//logd request_id 21
                " FROM object_registry nobr "
                " JOIN object_history oh ON oh.id = nobr.id "
                " JOIN nsset_history nh ON nh.historyid = oh.historyid "
                " JOIN history h ON h.id = nh.historyid "
                " JOIN registrar clr ON clr.id = oh.clid "
                " JOIN registrar crr ON crr.id = nobr.crid "
                " LEFT JOIN registrar upr ON upr.id = oh.upid"
                " WHERE "
                " nobr.roid = $1::text "
                " AND nobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) ")
                + (history_timestamp_.isset()
                ? std::string(" AND h.valid_from <= ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' "
                  " AND ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' < h.valid_to ")
                : std::string())
                + std::string(" ORDER BY h.id DESC ")
                + (lock_ ? std::string(" FOR UPDATE OF nobr") : std::string())
                , params);

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
                }

                nsset_history_res.reserve(res.size());//alloc
                for(Database::Result::size_type i = 0; i < res.size(); ++i)
                {
                    unsigned long long nsset_id = 0;//nsset id
                    InfoNssetOutput nsset_history_output;

                    nsset_id = static_cast<unsigned long long>(res[i][0]);//nobr.id

                    nsset_history_output.info_nsset_data.roid = static_cast<std::string>(res[i][1]);//nobr.roid

                    nsset_history_output.info_nsset_data.handle = static_cast<std::string>(res[i][2]);//nobr.name

                    nsset_history_output.info_nsset_data.delete_time = res[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][3])));//nobr.erdate

                    nsset_history_output.info_nsset_data.historyid = static_cast<unsigned long long>(res[i][4]);//oh.historyid

                    nsset_history_output.next_historyid = res[i][6].isnull() ? Nullable<unsigned long long>()
                    : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][6]));//h.next

                    nsset_history_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(res[i][7]));//h.valid_from

                    nsset_history_output.history_valid_to = res[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][8])));//h.valid_to

                    nsset_history_output.info_nsset_data.sponsoring_registrar_handle = static_cast<std::string>(res[i][10]);//clr.handle

                    nsset_history_output.info_nsset_data.create_registrar_handle = static_cast<std::string>(res[i][12]);//crr.handle

                    nsset_history_output.info_nsset_data.update_registrar_handle = res[i][14].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[i][14]));//upr.handle

                    nsset_history_output.info_nsset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[i][15]));//nobr.crdate

                    nsset_history_output.info_nsset_data.transfer_time = res[i][16].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][16])));//oh.trdate

                    nsset_history_output.info_nsset_data.update_time = res[i][17].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][17])));//oh.update

                    nsset_history_output.info_nsset_data.tech_check_level = res[i][18].isnull() ? Nullable<unsigned long long>()
                    : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][18]));

                    nsset_history_output.info_nsset_data.authinfopw = static_cast<std::string>(res[i][19]);//oh.authinfopw

                    nsset_history_output.info_nsset_data.crhistoryid = static_cast<unsigned long long>(res[i][20]);//nobr.crhistoryid

                    nsset_history_output.logd_request_id = res[i][21].isnull() ? Nullable<unsigned long long>()
                        : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][21]));

                    //list of historic technical contacts
                    Database::Result tech_contact_res = ctx.get_conn().exec_params(
                        "SELECT cobr.name "
                         " FROM nsset_contact_map_history ncmh "
                         " JOIN object_registry cobr ON ncmh.contactid = cobr.id "
                         " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                         " WHERE ncmh.nssetid = $1::bigint "
                         " AND ncmh.historyid = $2::bigint "
                         " ORDER BY cobr.name "
                    , Database::query_param_list(nsset_id)(nsset_history_output.info_nsset_data.historyid));

                    nsset_history_output.info_nsset_data.tech_contacts.reserve(tech_contact_res.size());
                    for(Database::Result::size_type j = 0; j < tech_contact_res.size(); ++j)
                    {
                        nsset_history_output.info_nsset_data.tech_contacts.push_back(static_cast<std::string>(tech_contact_res[j][0]));
                    }

                    //history of dns hosts
                    {
                        Database::Result hres = ctx.get_conn().exec_params(
                        "SELECT hh.id, hh.historyid, hh.fqdn "
                        " FROM host_history hh "
                        " WHERE hh.nssetid = $1::bigint "
                        " AND hh.historyid = $2::bigint "
                        " ORDER BY hh.fqdn "
                        , Database::query_param_list(nsset_id)(nsset_history_output.info_nsset_data.historyid));

                        nsset_history_output.info_nsset_data.dns_hosts.reserve(hres.size());//alloc
                        for(Database::Result::size_type i = 0; i < hres.size(); ++i)
                        {
                            unsigned long long dns_host_id = static_cast<unsigned long long>(hres[i][0]);//hh.id
                            std::string dns_host_fqdn = static_cast<std::string>(hres[i][2]);//hh.fqdn

                            Database::Result ipres = ctx.get_conn().exec_params(
                            "SELECT ih.historyid, ih.ipaddr "
                            " FROM host_ipaddr_map_history ih "
                            " WHERE ih.nssetid = $1::bigint "
                            " AND ih.hostid = $2::bigint "
                            " AND ih.historyid = $3::bigint "
                            " ORDER BY ih.ipaddr "
                            , Database::query_param_list(nsset_id)(dns_host_id)(nsset_history_output.info_nsset_data.historyid));

                            std::vector<std::string> dns_host_ips;

                            dns_host_ips.reserve(ipres.size());

                            for(Database::Result::size_type j = 0; j < ipres.size(); ++j)
                            {
                                std::string dns_host_ipaddr = static_cast<std::string>(ipres[j][1]);//ih.ipaddr
                                dns_host_ips.push_back(dns_host_ipaddr);
                            }//for dns host history ip
                            nsset_history_output.info_nsset_data.dns_hosts.push_back(DnsHost(dns_host_fqdn, dns_host_ips));
                        }//for dns host history
                    }//history of dns hosts

                    nsset_history_res.push_back(nsset_history_output);
                }//for res
            }//if roid
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return nsset_history_res;
    }//InfoNssetHistory::exec

    std::string InfoNssetOutput::to_string() const
    {
        return Util::format_data_structure("InfoNssetOutput",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_nsset_data",info_nsset_data.to_string()))
        (std::make_pair("next_historyid",next_historyid.print_quoted()))
        (std::make_pair("history_valid_from",boost::lexical_cast<std::string>(history_valid_from)))
        (std::make_pair("history_valid_to",history_valid_to.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id.print_quoted()))
        );
    }

    std::string InfoNssetHistory::to_string() const
    {
        return Util::format_operation_state("InfoNssetHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

}//namespace Fred

