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
 *  nsset info implementation
 */

#include <string>
#include <vector>
#include <utility>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>

#include "info_nsset_impl.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    InfoNsset::InfoNsset()
    : history_query_(false)
    , lock_(false)
    {}

    InfoNsset& InfoNsset::set_handle(const std::string& handle)
    {
        handle_ = handle;
        return *this;
    }

    InfoNsset& InfoNsset::set_roid(const std::string& nsset_roid)
    {
        nsset_roid_ = nsset_roid;
        return *this;
    }
    InfoNsset& InfoNsset::set_id(unsigned long long nsset_id)
    {
        nsset_id_ = nsset_id;
        return *this;
    }

    InfoNsset& InfoNsset::set_historyid(unsigned long long nsset_historyid)
    {
        nsset_historyid_ = nsset_historyid;
        return *this;
    }

    InfoNsset& InfoNsset::set_history_timestamp(boost::posix_time::ptime history_timestamp)
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoNsset& InfoNsset::set_history_query(bool history_query)
    {
        history_query_ = history_query;
        return *this;
    }

    InfoNsset& InfoNsset::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::pair<std::string, Database::QueryParams> InfoNsset::make_nsset_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        //query params
        Database::QueryParams params;
        std::ostringstream sql;

        params.push_back(local_timestamp_pg_time_zone_name);//refered as $1

        sql << "SELECT nobr.id, nobr.roid, nobr.name " //nsset 0-2
        " , (nobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE $1::text " //nsset 3
        " , obj.id, h.id , h.next, (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE $1::text "
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE $1::text " //historyid 4-8
        " , obj.clid, clr.handle "//sponsoring registrar 9-10
        " , nobr.crid, crr.handle "//creating registrar 11-12
        " , obj.upid, upr.handle "//last updated by registrar 13-14
        " , (nobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 15
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 16
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 17
        " , nt.checklevel "//checklevel 18
        " , obj.authinfopw "//transfer passwd 19
        " , nobr.crhistoryid " //first historyid 20
        " , h.request_id " //logd request_id 21
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp "// utc timestamp 22
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp  "// local zone timestamp 23
        " FROM object_registry nobr ";
        if(history_query_)
        {
            sql << " JOIN object_history obj ON obj.id = nobr.id "
            " JOIN nsset_history nt ON nt.historyid = obj.historyid "
            " JOIN history h ON h.id = nt.historyid ";
        }
        else
        {
            sql << " JOIN object obj ON obj.id = nobr.id "
            " JOIN nsset nt ON nt.id = obj.id "
            " JOIN history h ON h.id = nobr.historyid ";
        }

        sql << " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = nobr.crid "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " WHERE "
        " nobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) ";


        if(handle_.isset())
        {
            params.push_back(handle_);
            sql << " AND nobr.name = UPPER($"<< params.size() <<"::text) ";
        }


        if(nsset_roid_.isset())
        {
            params.push_back(nsset_roid_);
            sql << " AND nobr.roid = $"<< params.size() <<"::text ";
        }

        if(nsset_id_.isset())
        {
            params.push_back(nsset_id_);
            sql << " AND nobr.id = $"<< params.size() <<"::bigint ";
        }

        if(nsset_historyid_.isset())
        {
            params.push_back(nsset_historyid_);
            sql << " AND h.id = $"<< params.size() <<"::bigint ";
        }

        if(history_timestamp_.isset())
        {
            params.push_back(history_timestamp_);
            sql << " AND h.valid_from <= ($"<< params.size() <<"::timestamp AT TIME ZONE $1::text) AT TIME ZONE 'UTC' "
            " AND (($"<< params.size() <<"::timestamp AT TIME ZONE $1::text) AT TIME ZONE 'UTC' < h.valid_to OR h.valid_to IS NULL)";
        }

        sql << " ORDER BY h.id DESC ";

        if(lock_)
        {
            sql << " FOR UPDATE of nobr ";
        }
        else
        {
            sql << " FOR SHARE of nobr ";
        }

        return std::make_pair(sql.str(), params);

    }

    std::pair<std::string, Database::QueryParams> InfoNsset::make_tech_contact_query(
            unsigned long long id, unsigned long long historyid)
    {
        //technical contacts
        Database::QueryParams params;
        std::ostringstream sql;

        sql << "SELECT cobr.id, cobr.name ";
        if(history_query_)
        {
            params.push_back(id);
            sql << " FROM nsset_contact_map_history ncm "
                    " JOIN object_registry cobr ON ncm.contactid = cobr.id "
                    " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                    " WHERE ncm.nssetid = $"<< params.size() <<"::bigint ";
            params.push_back(historyid);
            sql << " AND ncm.historyid = $"<< params.size() <<"::bigint ";
        }
        else
        {
            params.push_back(id);
            sql << " FROM nsset_contact_map ncm "
            " JOIN object_registry cobr ON ncm.contactid = cobr.id AND cobr.erdate IS NULL "
            " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
            " WHERE ncm.nssetid = $"<< params.size() <<"::bigint ";
        }
        sql << " ORDER BY cobr.name ";

        return std::make_pair(sql.str(), params);
    }

    std::pair<std::string, Database::QueryParams> InfoNsset::make_dns_host_query(
            unsigned long long nssetid, unsigned long long historyid)
    {
        Database::QueryParams params;
        std::ostringstream sql;

        sql << "SELECT h.nssetid,h.id, h.fqdn ";
        if(history_query_)
        {
            params.push_back(nssetid);
            sql << " FROM host_history h "
            " WHERE h.nssetid = $"<< params.size() <<"::bigint ";
            params.push_back(historyid);
            sql << " AND h.historyid = $"<< params.size() <<"::bigint ";
        }
        else
        {
            params.push_back(nssetid);
            sql << " FROM host h "
            " JOIN object_registry nobr ON nobr.id = h.nssetid AND nobr.erdate IS NULL "
            " JOIN enum_object_type neot ON neot.id = nobr.type AND neot.name='nsset'::text "
            " WHERE h.nssetid = $"<< params.size() <<"::bigint ";
        }
        sql << " ORDER BY h.fqdn ";

        return std::make_pair(sql.str(), params);
    }

    std::pair<std::string, Database::QueryParams> InfoNsset::make_dns_ip_query(unsigned long long hostid, unsigned long long historyid)
    {
        Database::QueryParams params;
        std::ostringstream sql;

        sql <<  "SELECT him.ipaddr ";
        if(history_query_)
        {
            params.push_back(hostid);
            sql << " FROM host_ipaddr_map_history him "
            " WHERE him.hostid = $"<< params.size() <<"::bigint ";
            params.push_back(historyid);
            sql << " AND him.historyid = $"<< params.size() <<"::bigint ";
        }
        else
        {
            params.push_back(hostid);
            sql << " FROM host_ipaddr_map him "
            " WHERE him.hostid = $"<< params.size() <<"::bigint ";
        }
        sql << " ORDER BY him.ipaddr ";


        return std::make_pair(sql.str(), params);
    }

    std::vector<InfoNssetOutput> InfoNsset::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)//return data
    {
        std::vector<InfoNssetOutput> result;

        std::pair<std::string, Database::QueryParams> nsset_query = make_nsset_query(local_timestamp_pg_time_zone_name);
        Database::Result query_result = ctx.get_conn().exec_params(nsset_query.first,nsset_query.second);

        result.reserve(query_result.size());//alloc

        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoNssetOutput info_nsset_output;

            info_nsset_output.info_nsset_data.id = static_cast<unsigned long long>(query_result[i][0]);//nobr.id

            info_nsset_output.info_nsset_data.roid = static_cast<std::string>(query_result[i][1]);//nobr.roid

            info_nsset_output.info_nsset_data.handle = static_cast<std::string>(query_result[i][2]);//nobr.name

            info_nsset_output.info_nsset_data.delete_time = query_result[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][3])));//nobr.erdate

            info_nsset_output.info_nsset_data.historyid = static_cast<unsigned long long>(query_result[i][5]);//h.id

            info_nsset_output.next_historyid = query_result[i][6].isnull() ? Nullable<unsigned long long>()
            : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][6]));//h.next

            info_nsset_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][7]));//h.valid_from

            info_nsset_output.history_valid_to = query_result[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][8])));//h.valid_to

            info_nsset_output.info_nsset_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i][10]);//clr.handle

            info_nsset_output.info_nsset_data.create_registrar_handle = static_cast<std::string>(query_result[i][12]);//crr.handle

            info_nsset_output.info_nsset_data.update_registrar_handle = query_result[i][14].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][14]));//upr.handle

            info_nsset_output.info_nsset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][15]));//nobr.crdate

            info_nsset_output.info_nsset_data.transfer_time = query_result[i][16].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][16])));//oh.trdate

            info_nsset_output.info_nsset_data.update_time = query_result[i][17].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][17])));//oh.update

            info_nsset_output.info_nsset_data.tech_check_level = query_result[i][18].isnull() ? Nullable<short>()
                       : Nullable<short>(static_cast<short>(query_result[i][18]));//nt.checklevel

            info_nsset_output.info_nsset_data.authinfopw = static_cast<std::string>(query_result[i][19]);//oh.authinfopw

            info_nsset_output.info_nsset_data.crhistoryid = static_cast<unsigned long long>(query_result[i][20]);//nobr.crhistoryid

            info_nsset_output.logd_request_id = query_result[i][21].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][21]));

            info_nsset_output.utc_timestamp = query_result[i][22].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][22]));// utc timestamp
            info_nsset_output.local_timestamp = query_result[i][23].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][23]));//local zone timestamp

            //tech contacts
            std::pair<std::string, Database::QueryParams> tech_contact_query = make_tech_contact_query(
                    info_nsset_output.info_nsset_data.id, info_nsset_output.info_nsset_data.historyid);
            Database::Result tech_contact_res = ctx.get_conn().exec_params(tech_contact_query.first, tech_contact_query.second);
            info_nsset_output.info_nsset_data.tech_contacts.reserve(tech_contact_res.size());
            for(Database::Result::size_type j = 0; j < tech_contact_res.size(); ++j)
            {
                info_nsset_output.info_nsset_data.tech_contacts.push_back(Fred::ObjectIdHandlePair(
                    static_cast<unsigned long long>(tech_contact_res[j][0]),
                    static_cast<std::string>(tech_contact_res[j][1])
                ));
            }

            //DNS keys
            std::pair<std::string, Database::QueryParams> dns_hosts_query = make_dns_host_query(
                    info_nsset_output.info_nsset_data.id, info_nsset_output.info_nsset_data.historyid);
            Database::Result dns_hosts_res = ctx.get_conn().exec_params(dns_hosts_query.first, dns_hosts_query.second);
            info_nsset_output.info_nsset_data.dns_hosts.reserve(dns_hosts_res.size());
            for(Database::Result::size_type j = 0; j < dns_hosts_res.size(); ++j)
            {
                unsigned long long dns_host_id = static_cast<unsigned long long>(dns_hosts_res[j][1]);//h.id
                std::string dns_host_fqdn = static_cast<std::string>(dns_hosts_res[j][2]);//h.fqdn

                std::pair<std::string, Database::QueryParams> dns_ip_query = make_dns_ip_query(
                        dns_host_id, info_nsset_output.info_nsset_data.historyid );

                Database::Result dns_ip_res = ctx.get_conn().exec_params(dns_ip_query.first, dns_ip_query.second);
                std::vector<boost::asio::ip::address> dns_ip;
                dns_ip.reserve(dns_ip_res.size());
                for(Database::Result::size_type k = 0; k < dns_ip_res.size(); ++k)
                {
                    dns_ip.push_back(boost::asio::ip::address::from_string(static_cast<std::string>(dns_ip_res[k][0])));
                }
                info_nsset_output.info_nsset_data.dns_hosts.push_back(DnsHost(dns_host_fqdn, dns_ip));
            }

            result.push_back(info_nsset_output);
        }//for res

        return result;
    }

    std::string InfoNsset::explain_analyze(OperationContext& ctx, std::vector<InfoNssetOutput>& result
            , const std::string& local_timestamp_pg_time_zone_name)
    {
        result = exec(ctx,local_timestamp_pg_time_zone_name);
        std::pair<std::string, Database::QueryParams> nsset_query = make_nsset_query(local_timestamp_pg_time_zone_name);
        std::string query_plan("\nNsset query: EXPLAIN ANALYZE ");
        query_plan += nsset_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(nsset_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result nsset_query_result = ctx.get_conn().exec_params(
            std::string("EXPLAIN ANALYZE ") + nsset_query.first,nsset_query.second);
        for(Database::Result::size_type i = 0; i < nsset_query_result.size(); ++i)
            query_plan += std::string(nsset_query_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> tech_contact_query = make_tech_contact_query(
                result.at(0).info_nsset_data.id, result.at(0).info_nsset_data.historyid);
        query_plan += "\nTech contact query: EXPLAIN ANALYZE ";
        query_plan += tech_contact_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(tech_contact_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result tech_contact_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN ANALYZE ") + tech_contact_query.first,tech_contact_query.second);
        for(Database::Result::size_type i = 0; i < tech_contact_result.size(); ++i)
                query_plan += std::string(tech_contact_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> dns_hosts_query = make_dns_host_query(
                result.at(0).info_nsset_data.id, result.at(0).info_nsset_data.historyid);
        query_plan += "\nDNS hosts query: EXPLAIN ANALYZE ";
        query_plan += dns_hosts_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(dns_hosts_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result dns_hosts_query_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN ANALYZE ") + dns_hosts_query.first,dns_hosts_query.second);
        for(Database::Result::size_type i = 0; i < dns_hosts_query_result.size(); ++i)
                query_plan += std::string(dns_hosts_query_result[i][0])+"\n";


        std::pair<std::string, Database::QueryParams> dns_ip_query = make_dns_ip_query(
                1, result.at(0).info_nsset_data.historyid);
        query_plan += "\nDNS hosts IP addresses query: EXPLAIN ANALYZE ";
        query_plan += dns_ip_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(dns_ip_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result dns_ip_query_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN ANALYZE ") + dns_ip_query.first,dns_ip_query.second);
        for(Database::Result::size_type i = 0; i < dns_ip_query_result.size(); ++i)
                query_plan += std::string(dns_ip_query_result[i][0])+"\n";

        return query_plan;
    }


}//namespace Fred

