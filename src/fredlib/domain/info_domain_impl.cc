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
 *  domain info implementation
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "info_domain_impl.h"

#include "src/fredlib/opcontext.h"
#include "util/util.h"
#include "util/printable.h"
#include "util/db/param_query_composition.h"

namespace Fred
{

    InfoDomain::InfoDomain()
    : history_query_(false)
    , lock_(false)
    {}

    InfoDomain& InfoDomain::set_history_query(bool history_query)
    {
        history_query_ = history_query;
        return *this;
    }

    InfoDomain& InfoDomain::set_lock()
    {
        lock_ = true;
        return *this;
    }

    InfoDomain& InfoDomain::set_inline_view_filter(const Database::ParamQuery& filter_expr)
    {
        info_domain_inline_view_filter_expr_ = filter_expr;
        return *this;
    }

    InfoDomain& InfoDomain::set_cte_id_filter(const Database::ParamQuery& cte_id_filter_query)
    {
        info_domain_id_filter_cte_ = cte_id_filter_query;
        return *this;
    }

    Database::ParamQuery InfoDomain::make_domain_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        Database::ParamQueryParameter p_local_zone(local_timestamp_pg_time_zone_name, "text");
        Database::ParamQuery info_domain_query;

        if(info_domain_id_filter_cte_.isset())
        {
            info_domain_query("WITH id_filter(id) as (")(info_domain_id_filter_cte_.get_value())(") ");
        }

        info_domain_query(
        "SELECT * FROM ("
        "SELECT dobr.id AS info_domain_id"
        " , dobr.roid AS info_domain_roid"
        " , dobr.name AS info_domain_fqdn"
        " , (dobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE ").param(p_local_zone)(" AS info_domain_delete_time"
        " , h.id AS info_domain_historyid"
        " , h.next AS info_domain_next_historyid"
        " , (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_domain_history_valid_from"
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_domain_history_valid_to"
        " , cor.id AS info_domain_registrant_id"
        " , cor.name  AS info_domain_registrant_handle"
        " , dt.nsset AS info_domain_nsset_id"
        " , nobr.name  AS info_domain_nsset_handle"
        " , dt.keyset AS info_domain_keyset_id"
        " , kobr.name  AS info_domain_keyset_handle"
        " , obj.clid AS info_domain_sponsoring_registrar_id"
        " , clr.handle AS info_domain_sponsoring_registrar_handle"
        " , dobr.crid AS info_domain_creating_registrar_id"
        " , crr.handle AS info_domain_creating_registrar_handle"
        " , obj.upid AS info_domain_last_updated_by_registrar_id"
        " , upr.handle AS info_domain_last_updated_by_registrar_handle"
        " , (dobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_domain_creation_time"
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_domain_transfer_time"
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS info_domain_update_time"
        " , dt.exdate AS info_domain_expiration_date"
        " , obj.authinfopw AS info_domain_authinfopw"
        " , ev.exdate AS info_domain_enum_validation_expiration"
        " , ev.publish AS info_domain_enum_publish"
        " ,(((dt.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval)::timestamp "
        " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
        " AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp AS info_domain_outzone_time"
        " ,(((dt.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval)::timestamp "
        " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
        " AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp AS info_domain_cancel_time"
        " , dobr.crhistoryid AS info_domain_first_historyid"
        " , h.request_id AS info_domain_logd_request_id"
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS info_domain_utc_timestamp "
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE ").param(p_local_zone)(")::timestamp AS info_domain_local_timestamp"
        " , z.enum_zone AS info_domain_is_enum"
        " , z.id AS info_domain_zone_id"
        " , z.fqdn AS info_domain_zone_fqdn"
        " FROM object_registry dobr ");

        if(history_query_)
        {
            info_domain_query(" JOIN object_history obj ON obj.id = dobr.id "
            " JOIN domain_history dt ON dt.historyid = obj.historyid "
            " JOIN history h ON h.id = dt.historyid ");
        }
        else
        {
            info_domain_query(" JOIN object obj ON obj.id = dobr.id "
            " JOIN domain dt ON dt.id = obj.id "
            " JOIN history h ON h.id = dobr.historyid ");
        }

        info_domain_query(" JOIN object_registry cor ON dt.registrant=cor.id "
        " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = dobr.crid "
        " JOIN zone z ON dt.zone = z.id "
        " LEFT JOIN object_registry nobr ON nobr.id = dt.nsset "
        " AND nobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) ");

        if(!history_query_)
        {
            info_domain_query(" AND nobr.erdate IS NULL ");
        }

        info_domain_query(" LEFT JOIN object_registry kobr ON kobr.id = dt.keyset "
        " AND kobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) ");

        if(!history_query_)
        {
            info_domain_query(" AND kobr.erdate IS NULL ");
        }


        info_domain_query(" LEFT JOIN registrar upr ON upr.id = obj.upid ");

        if(history_query_)
        {
            info_domain_query(" LEFT JOIN  enumval_history ev ON ev.domainid = dt.id AND ev.historyid = h.id");
        }
        else
        {
            info_domain_query(" LEFT JOIN  enumval ev ON ev.domainid = dt.id");
        }

        info_domain_query(" WHERE dobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) ");

        if(info_domain_id_filter_cte_.isset())
        {
            info_domain_query(" AND dobr.id IN (SELECT id FROM id_filter) ");
        }

        if(!history_query_)
        {
            info_domain_query(" AND dobr.erdate IS NULL ");
        }

        if(lock_)
        {
            info_domain_query(" FOR UPDATE of dobr ");
        }
        else
        {
            info_domain_query(" FOR SHARE of dobr ");
        }

        info_domain_query(") as tmp");

        if(info_domain_inline_view_filter_expr_.isset())
        {
            info_domain_query(" WHERE ")(info_domain_inline_view_filter_expr_.get_value());
        }

        info_domain_query(" ORDER BY info_domain_historyid DESC ");

        return info_domain_query;

    }

    Database::ParamQuery InfoDomain::make_admin_query(unsigned long long id, unsigned long long historyid)
    {
        //admin contacts
        Database::ParamQuery query;

        query("SELECT cobr.id AS admin_contact_id, cobr.name AS admin_contact_handle");
        if(history_query_)
        {
            query(" FROM domain_contact_map_history dcm "
                " JOIN object_registry cobr ON dcm.contactid = cobr.id "
                " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                " WHERE dcm.domainid = ").param_bigint(id)
                (" AND dcm.historyid = ").param_bigint(historyid);
        }
        else
        {
            query(" FROM domain_contact_map dcm "
            " JOIN object_registry cobr ON dcm.contactid = cobr.id AND cobr.erdate IS NULL "
            " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
            " WHERE dcm.domainid = ").param_bigint(id);
        }
        query(" AND dcm.role = 1 "// admin contact
        " ORDER BY cobr.name ");

        return query;
    }

    std::vector<InfoDomainOutput> InfoDomain::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)//return data
    {
        std::vector<InfoDomainOutput> result;

        std::pair<std::string, Database::QueryParams> domain_query = make_domain_query(local_timestamp_pg_time_zone_name).get_query();
        Database::Result query_result = ctx.get_conn().exec_params(domain_query.first,domain_query.second);

        result.reserve(query_result.size());//alloc

        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoDomainOutput info_domain_output;
            info_domain_output.info_domain_data.id = static_cast<unsigned long long>(query_result[i]["info_domain_id"]);
            info_domain_output.info_domain_data.roid = static_cast<std::string>(query_result[i]["info_domain_roid"]);
            info_domain_output.info_domain_data.fqdn = static_cast<std::string>(query_result[i]["info_domain_fqdn"]);

            info_domain_output.info_domain_data.delete_time = query_result[i]["info_domain_delete_time"].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_delete_time"])));

            info_domain_output.info_domain_data.historyid = static_cast<unsigned long long>(query_result[i]["info_domain_historyid"]);

            info_domain_output.next_historyid = query_result[i]["info_domain_next_historyid"].isnull() ? Nullable<unsigned long long>()
            : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i]["info_domain_next_historyid"]));

            info_domain_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_history_valid_from"]));

            info_domain_output.history_valid_to = query_result[i]["info_domain_history_valid_to"].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_history_valid_to"])));

            info_domain_output.info_domain_data.registrant = Fred::ObjectIdHandlePair(
                static_cast<unsigned long long>(query_result[i]["info_domain_registrant_id"])
                , static_cast<std::string>(query_result[i]["info_domain_registrant_handle"]));

            info_domain_output.info_domain_data.nsset = (query_result[i]["info_domain_nsset_id"].isnull() || query_result[i]["info_domain_nsset_handle"].isnull())
                ? Nullable<Fred::ObjectIdHandlePair>()
                : Nullable<Fred::ObjectIdHandlePair> (Fred::ObjectIdHandlePair(
                    static_cast<unsigned long long>(query_result[i]["info_domain_nsset_id"]),
                    static_cast<std::string>(query_result[i]["info_domain_nsset_handle"])));

            info_domain_output.info_domain_data.keyset =(query_result[i]["info_domain_keyset_id"].isnull() || query_result[i]["info_domain_keyset_handle"].isnull())
                ? Nullable<Fred::ObjectIdHandlePair>()
                : Nullable<Fred::ObjectIdHandlePair> (Fred::ObjectIdHandlePair(
                    static_cast<unsigned long long>(query_result[i]["info_domain_keyset_id"]),
                    static_cast<std::string>(query_result[i]["info_domain_keyset_handle"])));

            info_domain_output.info_domain_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i]["info_domain_sponsoring_registrar_handle"]);
            info_domain_output.info_domain_data.create_registrar_handle = static_cast<std::string>(query_result[i]["info_domain_creating_registrar_handle"]);

            info_domain_output.info_domain_data.update_registrar_handle = query_result[i]["info_domain_last_updated_by_registrar_handle"].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i]["info_domain_last_updated_by_registrar_handle"]));//upr.handle

            info_domain_output.info_domain_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_creation_time"]));

            info_domain_output.info_domain_data.transfer_time = query_result[i]["info_domain_transfer_time"].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_transfer_time"])));

            info_domain_output.info_domain_data.update_time = query_result[i]["info_domain_update_time"].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_update_time"])));

            info_domain_output.info_domain_data.expiration_date = query_result[i]["info_domain_expiration_date"].isnull() ? boost::gregorian::date()
            : boost::gregorian::from_string(static_cast<std::string>(query_result[i]["info_domain_expiration_date"]));

            info_domain_output.info_domain_data.authinfopw = static_cast<std::string>(query_result[i]["info_domain_authinfopw"]);

            info_domain_output.info_domain_data.enum_domain_validation = (static_cast<bool>(query_result[i]["info_domain_is_enum"]) == false)//if not ENUM
            ? Nullable<ENUMValidationExtension>()
            : Nullable<ENUMValidationExtension>(ENUMValidationExtension(
                boost::gregorian::from_string(static_cast<std::string>(query_result[i]["info_domain_enum_validation_expiration"]))
                ,static_cast<bool>(query_result[i]["info_domain_enum_publish"])));

            info_domain_output.info_domain_data.outzone_time = query_result[i]["info_domain_outzone_time"].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_outzone_time"]));

            info_domain_output.info_domain_data.cancel_time = query_result[i]["info_domain_cancel_time"].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_cancel_time"]));

            info_domain_output.info_domain_data.crhistoryid = static_cast<unsigned long long>(query_result[i]["info_domain_first_historyid"]);

            info_domain_output.info_domain_data.zone =
                ObjectIdHandlePair(
                    static_cast<unsigned long long>(query_result[i]["info_domain_zone_id"]),
                    static_cast<std::string>(query_result[i]["info_domain_zone_fqdn"])
                );

            info_domain_output.logd_request_id = query_result[i]["info_domain_logd_request_id"].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i]["info_domain_logd_request_id"]));

            info_domain_output.utc_timestamp = query_result[i]["info_domain_utc_timestamp"].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_utc_timestamp"]));
            info_domain_output.local_timestamp = query_result[i]["info_domain_local_timestamp"].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i]["info_domain_local_timestamp"]));

            //admin contacts
            std::pair<std::string, Database::QueryParams> admin_query = make_admin_query(
                    info_domain_output.info_domain_data.id, info_domain_output.info_domain_data.historyid).get_query();

            //list of administrative contacts
            Database::Result admin_contact_res = ctx.get_conn().exec_params(admin_query.first, admin_query.second);
            info_domain_output.info_domain_data.admin_contacts.reserve(admin_contact_res.size());
            for(Database::Result::size_type j = 0; j < admin_contact_res.size(); ++j)
            {
                info_domain_output.info_domain_data.admin_contacts.push_back(Fred::ObjectIdHandlePair(
                        static_cast<unsigned long long>(admin_contact_res[j]["admin_contact_id"]),
                        static_cast<std::string>(admin_contact_res[j]["admin_contact_handle"])
                        ));
            }

            result.push_back(info_domain_output);
        }//for res

        return result;
    }

    std::string InfoDomain::explain_analyze(OperationContext& ctx, std::vector<InfoDomainOutput>& result
            , const std::string& local_timestamp_pg_time_zone_name)
    {
        result = exec(ctx,local_timestamp_pg_time_zone_name);
        std::pair<std::string, Database::QueryParams> domain_query = make_domain_query(local_timestamp_pg_time_zone_name).get_query();
        std::string query_plan("\nDomain query: EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ");
        query_plan += domain_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_container(domain_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result domain_query_result = ctx.get_conn().exec_params(
            std::string("EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ") + domain_query.first,domain_query.second);
        for(Database::Result::size_type i = 0; i < domain_query_result.size(); ++i)
            query_plan += std::string(domain_query_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> admin_query = make_admin_query(1,1).get_query();
        query_plan += "\nAdmin query: EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ";
        query_plan += admin_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_container(admin_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result admin_query_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN (ANALYZE, VERBOSE, BUFFERS) ") + admin_query.first,admin_query.second);
        for(Database::Result::size_type i = 0; i < admin_query_result.size(); ++i)
                query_plan += std::string(admin_query_result[i][0])+"\n";

        return query_plan;
    }

}//namespace Fred

