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
#include "util/db/param_query_composition.h"
#include "util/util.h"

namespace Fred
{
    InfoNsset::InfoNsset()
    : history_query_(false)
    , lock_(false)
    {}

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

    InfoNsset& InfoNsset::set_inline_view_filter(const Database::ParamQuery& filter_expr)
    {
        info_nsset_inline_view_filter_expr_ = filter_expr;
        return *this;
    }

    InfoNsset& InfoNsset::set_cte_id_filter(const Database::ParamQuery& cte_id_filter_query)
    {
        info_nsset_id_filter_cte_ = cte_id_filter_query;
        return *this;
    }

    Database::ParamQuery InfoNsset::make_info_nsset_projection_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        Database::ReusableParameter p_local_zone(local_timestamp_pg_time_zone_name, "text");
        Database::ParamQuery info_nsset_query;

        if(info_nsset_id_filter_cte_.isset())
        {
            info_nsset_query("WITH id_filter(id) as (")(info_nsset_id_filter_cte_.get_value())(") ");
        }

        info_nsset_query(
        "SELECT * FROM ("
        "SELECT nobr.id AS info_nsset_id"
        " , nobr.roid AS info_nsset_roid"
        " , nobr.name AS info_nsset_handle"
        " , (nobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE ").param(p_local_zone)(" AS info_nsset_delete_time"
        " , h.id AS info_nsset_historyid"
        " , h.next AS info_nsset_next_historyid"
        " , (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_nsset_history_valid_from"
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_nsset_history_valid_to"
        " , obj.clid AS info_nsset_sponsoring_registrar_id"
        " , clr.handle AS info_nsset_sponsoring_registrar_handle"
        " , nobr.crid AS info_nsset_creating_registrar_id"
        " , crr.handle AS info_nsset_creating_registrar_handle"
        " , obj.upid AS info_nsset_last_updated_by_registrar_id"
        " , upr.handle AS info_nsset_last_updated_by_registrar_handle"
        " , (nobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_nsset_creation_time"
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_nsset_transfer_time"
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_nsset_update_time"
        " , nt.checklevel AS info_nsset_tech_check_level"
        " , obj.authinfopw AS info_nsset_authinfopw"
        " , nobr.crhistoryid AS info_nsset_first_historyid"
        " , h.request_id AS info_nsset_logd_request_id"
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS info_nsset_utc_timestamp"
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE ").param(p_local_zone)(")::timestamp AS info_nsset_local_timestamp"
        " FROM object_registry nobr ");
        if(history_query_)
        {
            info_nsset_query(
            " JOIN object_history obj ON obj.id = nobr.id "
            " JOIN nsset_history nt ON nt.historyid = obj.historyid "
            " JOIN history h ON h.id = nt.historyid ");
        }
        else
        {
            info_nsset_query(
            " JOIN object obj ON obj.id = nobr.id "
            " JOIN nsset nt ON nt.id = obj.id "
            " JOIN history h ON h.id = nobr.historyid ");
        }
        info_nsset_query(
        " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = nobr.crid "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " WHERE "
        " nobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) ");

        if(info_nsset_id_filter_cte_.isset())
        {
            info_nsset_query(" AND nobr.id IN (SELECT id FROM id_filter) ");
        }

        if(!history_query_)
        {
            info_nsset_query(
            " AND nobr.erdate IS NULL ");
        }

        if(lock_)
        {
            info_nsset_query(
            " FOR UPDATE of nobr ");
        }
        else
        {
            info_nsset_query(" FOR SHARE of nobr ");
        }
        info_nsset_query(") as tmp");

        //inline view sub-select locking example at:
        //http://www.postgresql.org/docs/9.1/static/sql-select.html#SQL-FOR-UPDATE-SHARE
        if(info_nsset_inline_view_filter_expr_.isset())
        {
            info_nsset_query(" WHERE ")(info_nsset_inline_view_filter_expr_.get_value());
        }

        info_nsset_query(
            " ORDER BY info_nsset_historyid DESC ");

        return info_nsset_query;
    }


    Database::ParamQuery InfoNsset::make_tech_contact_query(
            unsigned long long id, unsigned long long historyid)
    {
        //technical contacts query
        Database::ParamQuery query;

        query("SELECT cobr.id AS tech_contact_id, cobr.name AS tech_contact_handle");
        if(history_query_)
        {
            query(" FROM nsset_contact_map_history ncm "
                " JOIN object_registry cobr ON ncm.contactid = cobr.id "
                " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                " WHERE ncm.nssetid = ").param_bigint(id)
                (" AND ncm.historyid = ").param_bigint(historyid);
        }
        else
        {
            query(" FROM nsset_contact_map ncm "
                " JOIN object_registry cobr ON ncm.contactid = cobr.id AND cobr.erdate IS NULL "
                " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                " WHERE ncm.nssetid = ").param_bigint(id);
        }
        query(" ORDER BY cobr.name ");

        return query;
    }

    Database::ParamQuery InfoNsset::make_dns_host_query(
            unsigned long long nssetid, unsigned long long historyid)
    {
        Database::ParamQuery query;

        query("SELECT h.nssetid AS host_nssetid,h.id AS host_id, h.fqdn AS host_fqdn");
        if(history_query_)
        {
            query(" FROM host_history h WHERE h.nssetid = ").param_bigint(nssetid)
                (" AND h.historyid = ").param_bigint(historyid);
        }
        else
        {
            query(" FROM host h "
                " JOIN object_registry nobr ON nobr.id = h.nssetid AND nobr.erdate IS NULL "
                " JOIN enum_object_type neot ON neot.id = nobr.type AND neot.name='nsset'::text "
                " WHERE h.nssetid = ").param_bigint(nssetid);
        }
        query(" ORDER BY h.fqdn ");

        return query;
    }

    Database::ParamQuery InfoNsset::make_dns_ip_query(unsigned long long hostid, unsigned long long historyid)
    {
        Database::ParamQuery query;

        query("SELECT him.ipaddr AS host_ipaddr");
        if(history_query_)
        {
            query(" FROM host_ipaddr_map_history him "
            " WHERE him.hostid = ").param_bigint(hostid)
            (" AND him.historyid = ").param_bigint(historyid);
        }
        else
        {
            query(" FROM host_ipaddr_map him WHERE him.hostid = ").param_bigint(hostid);
        }
        query(" ORDER BY him.ipaddr ");

        return query;
    }



    std::vector<InfoNssetOutput> InfoNsset::exec(OperationContext& ctx,
        const std::string& local_timestamp_pg_time_zone_name)//return data
    {
        std::vector<InfoNssetOutput> result;

        Database::ParamQuery nsset_param_query = make_info_nsset_projection_query(
            local_timestamp_pg_time_zone_name);

        std::pair<std::string,Database::query_param_list> nsset_query_with_params
            = nsset_param_query.get_query();

        Database::Result param_query_result = ctx.get_conn().exec_params(
            nsset_query_with_params.first, nsset_query_with_params.second);

        result.reserve(param_query_result.size());//alloc

        for(Database::Result::size_type i = 0; i < param_query_result.size(); ++i)
        {
            InfoNssetOutput info_nsset_output;
            info_nsset_output.info_nsset_data.id = static_cast<unsigned long long>(param_query_result[i]["info_nsset_id"]);
            info_nsset_output.info_nsset_data.roid = static_cast<std::string>(param_query_result[i]["info_nsset_roid"]);
            info_nsset_output.info_nsset_data.handle = static_cast<std::string>(param_query_result[i]["info_nsset_handle"]);
            info_nsset_output.info_nsset_data.delete_time = param_query_result[i]["info_nsset_delete_time"].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_delete_time"])));
            info_nsset_output.info_nsset_data.historyid = static_cast<unsigned long long>(param_query_result[i]["info_nsset_historyid"]);
            info_nsset_output.next_historyid = param_query_result[i]["info_nsset_next_historyid"].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(param_query_result[i]["info_nsset_next_historyid"]));
            info_nsset_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_history_valid_from"]));
            info_nsset_output.history_valid_to = param_query_result[i]["info_nsset_history_valid_to"].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_history_valid_to"])));
            info_nsset_output.info_nsset_data.sponsoring_registrar_handle = static_cast<std::string>(param_query_result[i]["info_nsset_sponsoring_registrar_handle"]);
            info_nsset_output.info_nsset_data.create_registrar_handle = static_cast<std::string>(param_query_result[i]["info_nsset_creating_registrar_handle"]);
            info_nsset_output.info_nsset_data.update_registrar_handle = param_query_result[i]["info_nsset_last_updated_by_registrar_handle"].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(param_query_result[i]["info_nsset_last_updated_by_registrar_handle"]));
            info_nsset_output.info_nsset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_creation_time"]));
            info_nsset_output.info_nsset_data.transfer_time = param_query_result[i]["info_nsset_transfer_time"].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_transfer_time"])));
            info_nsset_output.info_nsset_data.update_time = param_query_result[i]["info_nsset_update_time"].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_update_time"])));
            info_nsset_output.info_nsset_data.tech_check_level = param_query_result[i]["info_nsset_tech_check_level"].isnull() ? Nullable<short>()
                : Nullable<short>(static_cast<short>(param_query_result[i]["info_nsset_tech_check_level"]));
            info_nsset_output.info_nsset_data.authinfopw = static_cast<std::string>(param_query_result[i]["info_nsset_authinfopw"]);
            info_nsset_output.info_nsset_data.crhistoryid = static_cast<unsigned long long>(param_query_result[i]["info_nsset_first_historyid"]);
            info_nsset_output.logd_request_id = param_query_result[i]["info_nsset_logd_request_id"].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(param_query_result[i]["info_nsset_logd_request_id"]));
            info_nsset_output.utc_timestamp = param_query_result[i]["info_nsset_utc_timestamp"].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_utc_timestamp"]));
            info_nsset_output.local_timestamp = param_query_result[i]["info_nsset_local_timestamp"].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(param_query_result[i]["info_nsset_local_timestamp"]));

            //tech contacts
            std::pair<std::string, Database::QueryParams> tech_contact_query = make_tech_contact_query(
                    info_nsset_output.info_nsset_data.id, info_nsset_output.info_nsset_data.historyid).get_query();
            Database::Result tech_contact_res = ctx.get_conn().exec_params(tech_contact_query.first, tech_contact_query.second);
            info_nsset_output.info_nsset_data.tech_contacts.reserve(tech_contact_res.size());
            for(Database::Result::size_type j = 0; j < tech_contact_res.size(); ++j)
            {
                info_nsset_output.info_nsset_data.tech_contacts.push_back(Fred::ObjectIdHandlePair(
                    static_cast<unsigned long long>(tech_contact_res[j]["tech_contact_id"]),
                    static_cast<std::string>(tech_contact_res[j]["tech_contact_handle"])
                ));
            }

            //DNS keys
            std::pair<std::string, Database::QueryParams> dns_hosts_query = make_dns_host_query(
                    info_nsset_output.info_nsset_data.id, info_nsset_output.info_nsset_data.historyid).get_query();
            Database::Result dns_hosts_res = ctx.get_conn().exec_params(dns_hosts_query.first, dns_hosts_query.second);
            info_nsset_output.info_nsset_data.dns_hosts.reserve(dns_hosts_res.size());
            for(Database::Result::size_type j = 0; j < dns_hosts_res.size(); ++j)
            {
                unsigned long long dns_host_id = static_cast<unsigned long long>(dns_hosts_res[j]["host_id"]);
                std::string dns_host_fqdn = static_cast<std::string>(dns_hosts_res[j]["host_fqdn"]);

                std::pair<std::string, Database::QueryParams> dns_ip_query = make_dns_ip_query(
                        dns_host_id, info_nsset_output.info_nsset_data.historyid ).get_query();

                Database::Result dns_ip_res = ctx.get_conn().exec_params(dns_ip_query.first, dns_ip_query.second);
                std::vector<boost::asio::ip::address> dns_ip;
                dns_ip.reserve(dns_ip_res.size());
                for(Database::Result::size_type k = 0; k < dns_ip_res.size(); ++k)
                {
                    dns_ip.push_back(boost::asio::ip::address::from_string(static_cast<std::string>(dns_ip_res[k]["host_ipaddr"])));
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
        std::pair<std::string, Database::QueryParams> nsset_query = make_info_nsset_projection_query(local_timestamp_pg_time_zone_name).get_query();
        std::string query_plan("\nNsset query: EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ");
        query_plan += nsset_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_container(nsset_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result nsset_query_result = ctx.get_conn().exec_params(
            std::string("EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ") + nsset_query.first,nsset_query.second);
        for(Database::Result::size_type i = 0; i < nsset_query_result.size(); ++i)
            query_plan += std::string(nsset_query_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> tech_contact_query = make_tech_contact_query(
                1, 1).get_query();
        query_plan += "\nTech contact query: EXPLAIN (ANALYZE, VERBOSE, BUFFERS)  ";
        query_plan += tech_contact_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_container(tech_contact_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result tech_contact_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ") + tech_contact_query.first,tech_contact_query.second);
        for(Database::Result::size_type i = 0; i < tech_contact_result.size(); ++i)
                query_plan += std::string(tech_contact_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> dns_hosts_query = make_dns_host_query(
                1, 1).get_query();
        query_plan += "\nDNS hosts query: EXPLAIN (ANALYZE, VERBOSE, BUFFERS)  ";
        query_plan += dns_hosts_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_container(dns_hosts_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result dns_hosts_query_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ") + dns_hosts_query.first,dns_hosts_query.second);
        for(Database::Result::size_type i = 0; i < dns_hosts_query_result.size(); ++i)
                query_plan += std::string(dns_hosts_query_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> dns_ip_query = make_dns_ip_query(
                1, 1).get_query();
        query_plan += "\nDNS hosts IP addresses query: EXPLAIN (ANALYZE, VERBOSE, BUFFERS)  ";
        query_plan += dns_ip_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_container(dns_ip_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result dns_ip_query_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ") + dns_ip_query.first,dns_ip_query.second);
        for(Database::Result::size_type i = 0; i < dns_ip_query_result.size(); ++i)
                query_plan += std::string(dns_ip_query_result[i][0])+"\n";

        return query_plan;
    }


}//namespace Fred

