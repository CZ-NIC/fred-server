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
 *  keyset history info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/keyset/info_keyset_history.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"



namespace Fred
{

    InfoKeysetHistory::InfoKeysetHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , lock_(false)
    {}

    InfoKeysetHistory::InfoKeysetHistory(const std::string& roid)
    : roid_(roid)
    , lock_(false)
    {}

    InfoKeysetHistory& InfoKeysetHistory::set_history_timestamp(boost::posix_time::ptime history_timestamp)//set history timestamp
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoKeysetHistory& InfoKeysetHistory::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoKeysetOutput> InfoKeysetHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetOutput> keyset_history_res;

        try
        {
            //info about keyset history by roid and optional history timestamp
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
                std::string("SELECT kobr.id, kobr.roid, kobr.name, kobr.erdate "//keyset 0-3
                " , oh.historyid, h.id , h.next, h.valid_from, h.valid_to "//history 4-8
                " , oh.clid, clr.handle "//sponsoring registrar 9-10
                " , kobr.crid, crr.handle "//creating registrar 11-12
                " , oh.upid, upr.handle "//last updated by registrar 13-14
                " , kobr.crdate, oh.trdate, oh.update "//registration dates 15-17
                " , oh.authinfopw " //transfer passwd 18
                " , kobr.crhistoryid "//first historyid 19
                " , h.request_id "//logd request_id 20
                " FROM object_registry kobr "
                " JOIN object_history oh ON oh.id = kobr.id "
                " JOIN keyset_history kh ON kh.historyid = oh.historyid "
                " JOIN history h ON h.id = kh.historyid "
                " JOIN registrar clr ON clr.id = oh.clid "
                " JOIN registrar crr ON crr.id = kobr.crid "
                " LEFT JOIN registrar upr ON upr.id = oh.upid"
                " WHERE "
                " kobr.roid = $1::text "
                " AND kobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) ")
                + (history_timestamp_.isset()
                ? std::string(" AND h.valid_from <= ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' "
                  " AND ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' < h.valid_to ")
                : std::string())
                + std::string(" ORDER BY h.id DESC ")
                + (lock_ ? std::string(" FOR UPDATE OF kobr") : std::string())
                , params);

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
                }

                keyset_history_res.reserve(res.size());//alloc
                for(Database::Result::size_type i = 0; i < res.size(); ++i)
                {
                    unsigned long long keyset_id = 0;//keyset id
                    InfoKeysetOutput keyset_history_output;

                    keyset_id = static_cast<unsigned long long>(res[i][0]);//kobr.id

                    keyset_history_output.info_keyset_data.roid = static_cast<std::string>(res[i][1]);//kobr.roid

                    keyset_history_output.info_keyset_data.handle = static_cast<std::string>(res[i][2]);//kobr.name

                    keyset_history_output.info_keyset_data.delete_time = res[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][3])));//kobr.erdate

                    keyset_history_output.info_keyset_data.historyid = static_cast<unsigned long long>(res[i][4]);//oh.historyid

                    keyset_history_output.next_historyid = res[i][6].isnull() ? Nullable<unsigned long long>()
                    : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][6]));//h.next

                    keyset_history_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(res[i][7]));//h.valid_from

                    keyset_history_output.history_valid_to = res[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][8])));//h.valid_to

                    keyset_history_output.info_keyset_data.sponsoring_registrar_handle = static_cast<std::string>(res[i][10]);//clr.handle

                    keyset_history_output.info_keyset_data.create_registrar_handle = static_cast<std::string>(res[i][12]);//crr.handle

                    keyset_history_output.info_keyset_data.update_registrar_handle = res[i][14].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[i][14]));//upr.handle

                    keyset_history_output.info_keyset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[i][15]));//kobr.crdate

                    keyset_history_output.info_keyset_data.transfer_time = res[i][16].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][16])));//oh.trdate

                    keyset_history_output.info_keyset_data.update_time = res[i][17].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][17])));//oh.update

                    keyset_history_output.info_keyset_data.authinfopw = static_cast<std::string>(res[i][18]);//oh.authinfopw

                    keyset_history_output.info_keyset_data.crhistoryid = static_cast<unsigned long long>(res[i][19]);//kobr.crhistoryid

                    keyset_history_output.logd_request_id = res[i][20].isnull() ? Nullable<unsigned long long>()
                        : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][20]));

                    //list of historic technical contacts
                    Database::Result tech_contact_res = ctx.get_conn().exec_params(
                        "SELECT cobr.name "
                         " FROM keyset_contact_map_history kcmh "
                         " JOIN object_registry cobr ON kcmh.contactid = cobr.id "
                         " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                         " WHERE kcmh.keysetid = $1::bigint "
                         " AND kcmh.historyid = $2::bigint "
                         " ORDER BY cobr.name "
                    , Database::query_param_list(keyset_id)(keyset_history_output.info_keyset_data.historyid));

                    keyset_history_output.info_keyset_data.tech_contacts.reserve(tech_contact_res.size());
                    for(Database::Result::size_type j = 0; j < tech_contact_res.size(); ++j)
                    {
                        keyset_history_output.info_keyset_data.tech_contacts.push_back(static_cast<std::string>(tech_contact_res[j][0]));
                    }

                    //history of dns keys
                    {
                        Database::Result hres = ctx.get_conn().exec_params(
                        "SELECT dh.id, dh.historyid, dh.flags, dh.protocol, dh.alg, dh.key "
                        " FROM dnskey_history dh "
                        " WHERE dh.keysetid = $1::bigint "
                        " AND dh.historyid = $2::bigint "
                        " ORDER BY dh.id "
                        , Database::query_param_list(keyset_id)(keyset_history_output.info_keyset_data.historyid));

                        keyset_history_output.info_keyset_data.dns_keys.reserve(hres.size());//alloc
                        for(Database::Result::size_type i = 0; i < hres.size(); ++i)
                        {
                            unsigned short flags = static_cast<unsigned int>(hres[i][2]);//dh.flags
                            unsigned short protocol = static_cast<unsigned int>(hres[i][3]);//dh.protocol
                            unsigned short alg = static_cast<unsigned int>(hres[i][4]);//dh.alg
                            std::string key = static_cast<std::string>(hres[i][5]);//dh.key

                            keyset_history_output.info_keyset_data.dns_keys.push_back(DnsKey(flags, protocol, alg, key));
                        }//for dns key history
                    }//history of dns keys

                    keyset_history_res.push_back(keyset_history_output);
                }//for res
            }//if roid
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return keyset_history_res;
    }//InfoKeysetHistory::exec

    std::string InfoKeysetOutput::to_string() const
    {
        return Util::format_data_structure("InfoKeysetOutput",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_keyset_data",info_keyset_data.to_string()))
        (std::make_pair("utc_timestamp",boost::lexical_cast<std::string>(utc_timestamp)))
        (std::make_pair("local_timestamp",boost::lexical_cast<std::string>(local_timestamp)))
        (std::make_pair("next_historyid",next_historyid.print_quoted()))
        (std::make_pair("history_valid_from",boost::lexical_cast<std::string>(history_valid_from)))
        (std::make_pair("history_valid_to",history_valid_to.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id.print_quoted()))
        );
    }

    std::string InfoKeysetHistory::to_string() const
    {
        return Util::format_operation_state("InfoKeysetHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }


    InfoKeyset::InfoKeyset()
    : history_query_(false)
    , lock_(false)
    {}

    InfoKeyset& InfoKeyset::set_handle(const std::string& handle)
    {
        handle_ = handle;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_roid(const std::string& keyset_roid)
    {
        keyset_roid_ = keyset_roid;
        return *this;
    }
    InfoKeyset& InfoKeyset::set_id(unsigned long long keyset_id)
    {
        keyset_id_ = keyset_id;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_historyid(unsigned long long keyset_historyid)
    {
        keyset_historyid_ = keyset_historyid;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_history_timestamp(boost::posix_time::ptime history_timestamp)
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_history_query(bool history_query)
    {
        history_query_ = history_query;
        return *this;
    }

    InfoKeyset& InfoKeyset::set_lock(bool lock)
    {
        lock_ = lock;
        return *this;
    }

    std::pair<std::string, Database::QueryParams> InfoKeyset::make_keyset_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        //query params
        Database::QueryParams params;
        std::ostringstream sql;

        params.push_back(local_timestamp_pg_time_zone_name);//refered as $1

        sql << "SELECT kobr.id, kobr.roid, kobr.name " //keyset 0-2
        " , (kobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE $1::text " //keyset 3
        " , obj.id, h.id , h.next, (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE $1::text "
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE $1::text " //historyid 4-8
        " , obj.clid, clr.handle "//sponsoring registrar 9-10
        " , kobr.crid, crr.handle "//creating registrar 11-12
        " , obj.upid, upr.handle "//last updated by registrar 13-14
        " , (kobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 15
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 16
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 17
        " , obj.authinfopw "//transfer passwd 18
        " , kobr.crhistoryid " //first historyid 19
        " , h.request_id " //logd request_id 20
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp "// utc timestamp 21
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp  "// local zone timestamp 22
        " FROM object_registry kobr ";
        if(history_query_)
        {
            sql << " JOIN object_history obj ON obj.id = kobr.id "
            " JOIN keyset_history kt ON kt.historyid = obj.historyid "
            " JOIN history h ON h.id = kt.historyid ";
        }
        else
        {
            sql << " JOIN object obj ON obj.id = kobr.id "
            " JOIN keyset kt ON kt.id = obj.id "
            " JOIN history h ON h.id = kobr.historyid ";
        }

        sql << " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = kobr.crid "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " WHERE "
        " kobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) ";


        if(handle_.isset())
        {
            params.push_back(handle_);
            sql << " AND kobr.name = UPPER($"<< params.size() <<"::text) ";
        }


        if(keyset_roid_.isset())
        {
            params.push_back(keyset_roid_);
            sql << " AND kobr.roid = $"<< params.size() <<"::text ";
        }

        if(keyset_id_.isset())
        {
            params.push_back(keyset_id_);
            sql << " AND kobr.id = $"<< params.size() <<"::bigint ";
        }

        if(keyset_historyid_.isset())
        {
            params.push_back(keyset_historyid_);
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
            sql << " FOR UPDATE of kobr ";
        }


        return std::make_pair(sql.str(), params);

    }

    std::pair<std::string, Database::QueryParams> InfoKeyset::make_tech_contact_query(
            unsigned long long id, unsigned long long historyid)
    {
        //technical contacts
        Database::QueryParams params;
        std::ostringstream sql;

        sql << "SELECT cobr.name ";
        if(history_query_)
        {
            params.push_back(id);
            sql << " FROM keyset_contact_map_history kcm "
                    " JOIN object_registry cobr ON kcm.contactid = cobr.id "
                    " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                    " WHERE kcm.keysetid = $"<< params.size() <<"::bigint ";
            params.push_back(historyid);
            sql << " AND kcm.historyid = $"<< params.size() <<"::bigint ";
        }
        else
        {
            params.push_back(id);
            sql << " FROM keyset_contact_map kcm "
            " JOIN object_registry cobr ON kcm.contactid = cobr.id AND cobr.erdate IS NULL "
            " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
            " WHERE kcm.keysetid = $"<< params.size() <<"::bigint ";
        }
        sql << " ORDER BY cobr.name ";

        return std::make_pair(sql.str(), params);
    }

    std::pair<std::string, Database::QueryParams> InfoKeyset::make_dns_keys_query(
            unsigned long long id, unsigned long long historyid)
    {
        Database::QueryParams params;
        std::ostringstream sql;

        sql << "SELECT d.id, d.flags, d.protocol, d.alg, d.key ";
        if(history_query_)
        {
            params.push_back(id);
            sql << " FROM dnskey_history d "
                    " WHERE d.keysetid = $"<< params.size() <<"::bigint ";
            params.push_back(historyid);
            sql << " AND d.historyid = $"<< params.size() <<"::bigint ";
        }
        else
        {
            params.push_back(id);
            sql << " FROM dnskey d "
            " WHERE d.keysetid = $"<< params.size() <<"::bigint ";
        }
        sql << " ORDER BY d.id ";

        return std::make_pair(sql.str(), params);
    }

    std::vector<InfoKeysetOutput> InfoKeyset::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)//return data
    {
        std::vector<InfoKeysetOutput> result;

        std::pair<std::string, Database::QueryParams> keyset_query = make_keyset_query(local_timestamp_pg_time_zone_name);
        Database::Result query_result = ctx.get_conn().exec_params(keyset_query.first,keyset_query.second);

        result.reserve(query_result.size());//alloc

        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoKeysetOutput info_keyset_output;

            info_keyset_output.info_keyset_data.id = static_cast<unsigned long long>(query_result[i][0]);//kobr.id

            info_keyset_output.info_keyset_data.roid = static_cast<std::string>(query_result[i][1]);//kobr.roid

            info_keyset_output.info_keyset_data.handle = static_cast<std::string>(query_result[i][2]);//kobr.name

            info_keyset_output.info_keyset_data.delete_time = query_result[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][3])));//kobr.erdate

            info_keyset_output.info_keyset_data.historyid = static_cast<unsigned long long>(query_result[i][5]);//h.id

            info_keyset_output.next_historyid = query_result[i][6].isnull() ? Nullable<unsigned long long>()
            : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][6]));//h.next

            info_keyset_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][7]));//h.valid_from

            info_keyset_output.history_valid_to = query_result[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][8])));//h.valid_to

            info_keyset_output.info_keyset_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i][10]);//clr.handle

            info_keyset_output.info_keyset_data.create_registrar_handle = static_cast<std::string>(query_result[i][12]);//crr.handle

            info_keyset_output.info_keyset_data.update_registrar_handle = query_result[i][14].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][14]));//upr.handle

            info_keyset_output.info_keyset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][15]));//kobr.crdate

            info_keyset_output.info_keyset_data.transfer_time = query_result[i][16].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][16])));//oh.trdate

            info_keyset_output.info_keyset_data.update_time = query_result[i][17].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][17])));//oh.update

            info_keyset_output.info_keyset_data.authinfopw = static_cast<std::string>(query_result[i][18]);//oh.authinfopw

            info_keyset_output.info_keyset_data.crhistoryid = static_cast<unsigned long long>(query_result[i][19]);//kobr.crhistoryid

            info_keyset_output.logd_request_id = query_result[i][20].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][20]));

            info_keyset_output.utc_timestamp = query_result[i][21].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][21]));// utc timestamp
            info_keyset_output.local_timestamp = query_result[i][22].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][22]));//local zone timestamp

            //tech contacts
            std::pair<std::string, Database::QueryParams> tech_contact_query = make_tech_contact_query(
                    info_keyset_output.info_keyset_data.id, info_keyset_output.info_keyset_data.historyid);
            Database::Result tech_contact_res = ctx.get_conn().exec_params(tech_contact_query.first, tech_contact_query.second);
            info_keyset_output.info_keyset_data.tech_contacts.reserve(tech_contact_res.size());
            for(Database::Result::size_type j = 0; j < tech_contact_res.size(); ++j)
            {
                info_keyset_output.info_keyset_data.tech_contacts.push_back(static_cast<std::string>(tech_contact_res[j][0]));
            }

            //DNS keys
            std::pair<std::string, Database::QueryParams> dns_keys_query = make_dns_keys_query(
                    info_keyset_output.info_keyset_data.id, info_keyset_output.info_keyset_data.historyid);
            Database::Result dns_keys_res = ctx.get_conn().exec_params(dns_keys_query.first, dns_keys_query.second);
            info_keyset_output.info_keyset_data.tech_contacts.reserve(dns_keys_res.size());
            for(Database::Result::size_type j = 0; j < dns_keys_res.size(); ++j)
            {
                unsigned short flags = static_cast<unsigned int>(dns_keys_res[j][2]);//d.flags
                unsigned short protocol = static_cast<unsigned int>(dns_keys_res[j][3]);//d.protocol
                unsigned short alg = static_cast<unsigned int>(dns_keys_res[j][4]);//d.alg
                std::string key = static_cast<std::string>(dns_keys_res[j][5]);//d.key
                info_keyset_output.info_keyset_data.dns_keys.push_back(DnsKey(flags, protocol, alg, key));
            }

            result.push_back(info_keyset_output);
        }//for res

        return result;
    }

    std::string InfoKeyset::explain_analyze(OperationContext& ctx, std::vector<InfoKeysetOutput>& result
            , const std::string& local_timestamp_pg_time_zone_name)
    {
        result = exec(ctx,local_timestamp_pg_time_zone_name);
        std::pair<std::string, Database::QueryParams> keyset_query = make_keyset_query(local_timestamp_pg_time_zone_name);
        std::string query_plan("\nKeyset query: EXPLAIN ANALYZE ");
        query_plan += keyset_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(keyset_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result keyset_query_result = ctx.get_conn().exec_params(
            std::string("EXPLAIN ANALYZE ") + keyset_query.first,keyset_query.second);
        for(Database::Result::size_type i = 0; i < keyset_query_result.size(); ++i)
            query_plan += std::string(keyset_query_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> tech_contact_query = make_tech_contact_query(
                result.at(0).info_keyset_data.id, result.at(0).info_keyset_data.historyid);
        query_plan += "\nTech contact query: EXPLAIN ANALYZE ";
        query_plan += tech_contact_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(tech_contact_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result tech_contact_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN ANALYZE ") + tech_contact_query.first,tech_contact_query.second);
        for(Database::Result::size_type i = 0; i < tech_contact_result.size(); ++i)
                query_plan += std::string(tech_contact_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> dns_keys_query = make_dns_keys_query(
                result.at(0).info_keyset_data.id, result.at(0).info_keyset_data.historyid);
        query_plan += "\nTech contact query: EXPLAIN ANALYZE ";
        query_plan += dns_keys_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(dns_keys_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result dns_keys_query_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN ANALYZE ") + dns_keys_query.first,dns_keys_query.second);
        for(Database::Result::size_type i = 0; i < dns_keys_query_result.size(); ++i)
                query_plan += std::string(dns_keys_query_result[i][0])+"\n";

        return query_plan;
    }


}//namespace Fred

