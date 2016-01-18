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
 *  keyset info implementation
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "info_keyset_impl.h"
#include "src/fredlib/opcontext.h"
#include "util/db/param_query_composition.h"
#include "util/util.h"



namespace Fred
{

    InfoKeyset::InfoKeyset()
    : history_query_(false)
    , lock_(false)
    {}

    InfoKeyset& InfoKeyset::set_history_query(bool history_query)
    {
        history_query_ = history_query;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_inline_view_filter(const Database::ParamQuery& filter_expr)
    {
        info_keyset_inline_view_filter_expr_ = filter_expr;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_cte_id_filter(const Database::ParamQuery& cte_id_filter_query)
    {
        info_keyset_id_filter_cte_ = cte_id_filter_query;
        return *this;
    }

    Database::ParamQuery InfoKeyset::make_info_keyset_projection_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        const Database::ReusableParameter p_local_zone(local_timestamp_pg_time_zone_name, "text");
        Database::ParamQuery info_keyset_query;

        if(info_keyset_id_filter_cte_.isset())
        {
            info_keyset_query("WITH id_filter(id) as (")(info_keyset_id_filter_cte_.get_value())(") ");
        }

        info_keyset_query(
        "SELECT * FROM ("
        "SELECT kobr.id AS ")(GetAlias::id())(
        " , kobr.roid AS ")(GetAlias::roid())(
        " , kobr.name AS ")(GetAlias::handle())(
        " , (kobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::delete_time())(
        " , h.id AS ")(GetAlias::historyid())(
        " , h.next AS ")(GetAlias::next_historyid())(
        " , (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::history_valid_from())(
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::history_valid_to())(
        " , obj.clid AS ")(GetAlias::sponsoring_registrar_id())(
        " , clr.handle AS ")(GetAlias::sponsoring_registrar_handle())(
        " , kobr.crid AS ")(GetAlias::creating_registrar_id())(
        " , crr.handle AS ")(GetAlias::creating_registrar_handle())(
        " , obj.upid AS ")(GetAlias::last_updated_by_registrar_id())(
        " , upr.handle AS ")(GetAlias::last_updated_by_registrar_handle())(
        " , (kobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::creation_time())(
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::transfer_time())(
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::update_time())(
        " , obj.authinfopw AS ")(GetAlias::authinfopw())(
        " , kobr.crhistoryid AS ")(GetAlias::first_historyid())(
        " , h.request_id AS ")(GetAlias::logd_request_id())(
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS ")(GetAlias::utc_timestamp())(
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE ").param(p_local_zone)(")::timestamp AS ")(GetAlias::local_timestamp())(
        " FROM object_registry kobr ");
        if(history_query_)
        {
            info_keyset_query(
            " JOIN object_history obj ON obj.id = kobr.id "
            " JOIN keyset_history kt ON kt.historyid = obj.historyid "
            " JOIN history h ON h.id = kt.historyid ");
        }
        else
        {
            info_keyset_query(
            " JOIN object obj ON obj.id = kobr.id "
            " JOIN keyset kt ON kt.id = obj.id "
            " JOIN history h ON h.id = kobr.historyid ");
        }
        info_keyset_query(
        " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = kobr.crid "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " WHERE "
        " kobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) ");

        if(info_keyset_id_filter_cte_.isset())
        {
            info_keyset_query(" AND kobr.id IN (SELECT id FROM id_filter) ");
        }

        if(!history_query_)
        {
            info_keyset_query(
            " AND kobr.erdate IS NULL ");
        }

        if(lock_)
        {
            info_keyset_query(
            " FOR UPDATE of kobr ");
        }
        else
        {
            info_keyset_query(" FOR SHARE of kobr ");
        }
        info_keyset_query(") as tmp");

        if(info_keyset_inline_view_filter_expr_.isset())
        {
            info_keyset_query(" WHERE ")(info_keyset_inline_view_filter_expr_.get_value());
        }

        info_keyset_query(
            " ORDER BY ")(GetAlias::historyid())(" DESC ");

        return info_keyset_query;
    }


    Database::ParamQuery InfoKeyset::make_tech_contact_query(
            unsigned long long id, unsigned long long historyid)
    {
        //technical contacts
        Database::ParamQuery query;

        query("SELECT cobr.id AS tech_contact_id, cobr.name AS tech_contact_handle");
        if(history_query_)
        {
            query(" FROM keyset_contact_map_history kcm "
                " JOIN object_registry cobr ON kcm.contactid = cobr.id "
                " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                " WHERE kcm.keysetid = ").param_bigint(id)
                (" AND kcm.historyid = ").param_bigint(historyid);
        }
        else
        {
            query(" FROM keyset_contact_map kcm "
                " JOIN object_registry cobr ON kcm.contactid = cobr.id AND cobr.erdate IS NULL "
                " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                " WHERE kcm.keysetid = ").param_bigint(id);
        }
        query(" ORDER BY cobr.name ");

        return query;
    }

    Database::ParamQuery InfoKeyset::make_dns_keys_query(
            unsigned long long id, unsigned long long historyid)
    {
        Database::ParamQuery query;

        query("SELECT d.id, d.flags AS flags, d.protocol AS protocol, d.alg AS alg, d.key AS key");
        if(history_query_)
        {
            query(" FROM dnskey_history d "
                " WHERE d.keysetid = ").param_bigint(id)
                (" AND d.historyid = ").param_bigint(historyid);
        }
        else
        {
            query(" FROM dnskey d "
                " WHERE d.keysetid = ").param_bigint(id);
        }
        query(" ORDER BY d.id ");

        return query;
    }

    std::vector<InfoKeysetOutput> InfoKeyset::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetOutput> result;

        Database::Result query_result = ctx.get_conn().exec_params(make_info_keyset_projection_query(local_timestamp_pg_time_zone_name));

        result.reserve(query_result.size());

        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoKeysetOutput info_keyset_output;
            info_keyset_output.info_keyset_data.id = static_cast<unsigned long long>(query_result[i][GetAlias::id()]);
            info_keyset_output.info_keyset_data.roid = static_cast<std::string>(query_result[i][GetAlias::roid()]);
            info_keyset_output.info_keyset_data.handle = static_cast<std::string>(query_result[i][GetAlias::handle()]);
            info_keyset_output.info_keyset_data.delete_time = query_result[i][GetAlias::delete_time()].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::delete_time()])));
            info_keyset_output.info_keyset_data.historyid = static_cast<unsigned long long>(query_result[i][GetAlias::historyid()]);
            info_keyset_output.next_historyid = query_result[i][GetAlias::next_historyid()].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][GetAlias::next_historyid()]));
            info_keyset_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::history_valid_from()]));
            info_keyset_output.history_valid_to = query_result[i][GetAlias::history_valid_to()].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::history_valid_to()])));
            info_keyset_output.info_keyset_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i][GetAlias::sponsoring_registrar_handle()]);
            info_keyset_output.info_keyset_data.create_registrar_handle = static_cast<std::string>(query_result[i][GetAlias::creating_registrar_handle()]);
            info_keyset_output.info_keyset_data.update_registrar_handle = query_result[i][GetAlias::last_updated_by_registrar_handle()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::last_updated_by_registrar_handle()]));
            info_keyset_output.info_keyset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::creation_time()]));
            info_keyset_output.info_keyset_data.transfer_time = query_result[i][GetAlias::transfer_time()].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::transfer_time()])));
            info_keyset_output.info_keyset_data.update_time = query_result[i][GetAlias::update_time()].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::update_time()])));
            info_keyset_output.info_keyset_data.authinfopw = static_cast<std::string>(query_result[i][GetAlias::authinfopw()]);
            info_keyset_output.info_keyset_data.crhistoryid = static_cast<unsigned long long>(query_result[i][GetAlias::first_historyid()]);
            info_keyset_output.logd_request_id = query_result[i][GetAlias::logd_request_id()].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][GetAlias::logd_request_id()]));
            info_keyset_output.utc_timestamp = query_result[i][GetAlias::utc_timestamp()].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::utc_timestamp()]));
            info_keyset_output.local_timestamp = query_result[i][GetAlias::local_timestamp()].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::local_timestamp()]));

            //tech contacts
            Database::Result tech_contact_res = ctx.get_conn().exec_params(make_tech_contact_query(
                info_keyset_output.info_keyset_data.id, info_keyset_output.info_keyset_data.historyid));
            info_keyset_output.info_keyset_data.tech_contacts.reserve(tech_contact_res.size());
            for(Database::Result::size_type j = 0; j < tech_contact_res.size(); ++j)
            {
                info_keyset_output.info_keyset_data.tech_contacts.push_back(Fred::ObjectIdHandlePair(
                    static_cast<unsigned long long>(tech_contact_res[j]["tech_contact_id"]),
                    static_cast<std::string>(tech_contact_res[j]["tech_contact_handle"])
                ));
            }

            //DNS keys
            Database::Result dns_keys_res = ctx.get_conn().exec_params(make_dns_keys_query(
                info_keyset_output.info_keyset_data.id, info_keyset_output.info_keyset_data.historyid));
            info_keyset_output.info_keyset_data.tech_contacts.reserve(dns_keys_res.size());
            for(Database::Result::size_type j = 0; j < dns_keys_res.size(); ++j)
            {
                unsigned short flags = static_cast<unsigned int>(dns_keys_res[j]["flags"]);
                unsigned short protocol = static_cast<unsigned int>(dns_keys_res[j]["protocol"]);
                unsigned short alg = static_cast<unsigned int>(dns_keys_res[j]["alg"]);
                std::string key = static_cast<std::string>(dns_keys_res[j]["key"]);
                info_keyset_output.info_keyset_data.dns_keys.push_back(DnsKey(flags, protocol, alg, key));
            }

            result.push_back(info_keyset_output);
        }//for res

        return result;
    }

}//namespace Fred

