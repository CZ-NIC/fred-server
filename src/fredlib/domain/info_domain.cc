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
 *  domain history info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/domain/info_domain.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "util/printable.h"

namespace Fred
{

    InfoDomainByHandle::InfoDomainByHandle(const std::string& fqdn)
        : fqdn_(fqdn)
        , lock_(false)
    {}

    InfoDomainByHandle& InfoDomainByHandle::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    InfoDomainOutput InfoDomainByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            domain_res = InfoDomain()
                    .set_fqdn(fqdn_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_fqdn(fqdn_));
            }

            if (domain_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res.at(0);
    }//InfoDomainByHandle::exec

    std::string InfoDomainByHandle::to_string() const
    {
        return Util::format_operation_state("InfoDomainByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle", fqdn_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoDomainById::InfoDomainById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoDomainById& InfoDomainById::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    InfoDomainOutput InfoDomainById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            domain_res = InfoDomain()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (domain_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res.at(0);
    }//InfoDomainById::exec

    std::string InfoDomainById::to_string() const
    {
        return Util::format_operation_state("InfoDomainById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoDomainHistory::InfoDomainHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , lock_(false)
    {}

    InfoDomainHistory::InfoDomainHistory(const std::string& roid)
    : roid_(roid)
    , lock_(false)
    {}

    InfoDomainHistory& InfoDomainHistory::set_history_timestamp(boost::posix_time::ptime history_timestamp)//set history timestamp
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoDomainHistory& InfoDomainHistory::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoDomainOutput> InfoDomainHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_res;

        try
        {
            domain_res = InfoDomain()
                    .set_roid(roid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_res;
    }//InfoDomainHistory::exec

    std::string InfoDomainHistory::to_string() const
    {
        return Util::format_operation_state("InfoDomainHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    std::string InfoDomainOutput::to_string() const
    {
        return Util::format_data_structure("InfoDomainOutput",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_domain_data",info_domain_data.to_string()))
        (std::make_pair("utc_timestamp",boost::lexical_cast<std::string>(utc_timestamp)))
        (std::make_pair("local_timestamp",boost::lexical_cast<std::string>(local_timestamp)))
        (std::make_pair("next_historyid",next_historyid.print_quoted()))
        (std::make_pair("history_valid_from",boost::lexical_cast<std::string>(history_valid_from)))
        (std::make_pair("history_valid_to",history_valid_to.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id.print_quoted()))
        );
    }

    bool InfoDomainOutput::operator==(const InfoDomainOutput& rhs) const
    {
        return info_domain_data == rhs.info_domain_data;
    }

    bool InfoDomainOutput::operator!=(const InfoDomainOutput& rhs) const
    {
        return !this->operator ==(rhs);
    }


    HistoryInfoDomainById::HistoryInfoDomainById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    HistoryInfoDomainById& HistoryInfoDomainById::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoDomainOutput> HistoryInfoDomainById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_history_res;

        try
        {
            domain_history_res = InfoDomain()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_history_res;
    }//HistoryInfoDomainById::exec

    std::string HistoryInfoDomainById::to_string() const
    {
        return Util::format_operation_state("HistoryInfoDomainById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    HistoryInfoDomainByHistoryid::HistoryInfoDomainByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    HistoryInfoDomainByHistoryid& HistoryInfoDomainByHistoryid::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    InfoDomainOutput HistoryInfoDomainByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainOutput> domain_history_res;

        try
        {
            domain_history_res = InfoDomain()
                    .set_historyid(historyid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (domain_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (domain_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return domain_history_res.at(0);
    }//HistoryInfoDomainByHistoryid::exec

    std::string HistoryInfoDomainByHistoryid::to_string() const
    {
        return Util::format_operation_state("HistoryInfoDomainByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }


    InfoDomain::InfoDomain()
    : history_query_(false)
    , lock_(false)
    {}

    InfoDomain& InfoDomain::set_fqdn(const std::string& fqdn)
    {
        fqdn_ = fqdn;
        return *this;
    }

    InfoDomain& InfoDomain::set_roid(const std::string& domain_roid)
    {
        domain_roid_ = domain_roid;
        return *this;
    }
    InfoDomain& InfoDomain::set_id(unsigned long long domain_id)
    {
        domain_id_ = domain_id;
        return *this;
    }

    InfoDomain& InfoDomain::set_historyid(unsigned long long domain_historyid)
    {
        domain_historyid_ = domain_historyid;
        return *this;
    }

    InfoDomain& InfoDomain::set_history_timestamp(boost::posix_time::ptime history_timestamp)
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoDomain& InfoDomain::set_history_query(bool history_query)
    {
        history_query_ = history_query;
        return *this;
    }

    InfoDomain& InfoDomain::set_lock(bool lock)
    {
        lock_ = lock;
        return *this;
    }

    std::pair<std::string, Database::QueryParams> InfoDomain::make_domain_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        //query params
        Database::QueryParams params;
        std::ostringstream sql;

        params.push_back(local_timestamp_pg_time_zone_name);//refered as $1

        sql << "SELECT dobr.id, dobr.roid, dobr.name " //domain 0-2
        " , (dobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE $1::text " //domain 3
        " , obj.id, h.id , h.next, (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE $1::text "
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE $1::text " //historyid 4-8
        " , cor.id, cor.name " //registrant 9-10
        " , dt.nsset, nobr.name "//nsset id and nsset handle 11-12
        " , dt.keyset, kobr.name " //keyset id and keyset handle 13-14
        " , obj.clid, clr.handle "//sponsoring registrar 15-16
        " , dobr.crid, crr.handle "//creating registrar 16-18
        " , obj.upid, upr.handle "//last updated by registrar 19-20
        " , (dobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 21
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 22
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 23
        " , dt.exdate "//registration dates 24
        " , obj.authinfopw "//transfer passwd 25
        " , evh.exdate, evh.publish "//enumval_history 26-27
        //outzone data and cancel date from enum_parameters compute 28-29
        " ,(((dt.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval)::timestamp "
        " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
        " AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as outzonedate "
        " ,(((dt.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval)::timestamp "
        " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
        " AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as canceldate "
        " , dobr.crhistoryid " //first historyid 30
        " , h.request_id " //logd request_id 31
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp "// utc timestamp 32
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp  "// local zone timestamp 33
        " FROM object_registry dobr ";

        if(history_query_)
        {
            sql << " JOIN object_history obj ON obj.id = dobr.id "
            " JOIN domain_history dt ON dt.historyid = obj.historyid "
            " JOIN history h ON h.id = dt.historyid ";
        }
        else
        {
            sql << " JOIN object obj ON obj.id = dobr.id "
            " JOIN domain dt ON dt.id = obj.id "
            " JOIN history h ON h.id = dobr.historyid ";
        }

        sql << " JOIN object_registry cor ON dt.registrant=cor.id "
        " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = dobr.crid "
        " LEFT JOIN object_registry nobr ON nobr.id = dt.nsset "
        " AND nobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) "
        " LEFT JOIN object_registry kobr ON kobr.id = dt.keyset "
        " AND kobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " LEFT JOIN  enumval_history evh ON evh.domainid = dt.id AND evh.historyid = h.id"
        " WHERE "
        " dobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) ";


        if(fqdn_.isset())
        {
            params.push_back(fqdn_);
            sql << " AND dobr.name = LOWER($"<< params.size() <<"::text) ";
        }


        if(domain_roid_.isset())
        {
            params.push_back(domain_roid_);
            sql << " AND dobr.roid = $"<< params.size() <<"::text ";
        }

        if(domain_id_.isset())
        {
            params.push_back(domain_id_);
            sql << " AND dobr.id = $"<< params.size() <<"::bigint ";
        }

        if(domain_historyid_.isset())
        {
            params.push_back(domain_historyid_);
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
            sql << " FOR UPDATE of dobr ";
        }


        return std::make_pair(sql.str(), params);

    }

    std::pair<std::string, Database::QueryParams> InfoDomain::make_admin_query(unsigned long long id, unsigned long long historyid)
    {
        //admin contacts
        Database::QueryParams params;
        std::ostringstream sql;

        sql << "SELECT cobr.name ";
        if(history_query_)
        {
            params.push_back(id);
            sql << " FROM domain_contact_map_history dcm "
                    " JOIN object_registry cobr ON dcm.contactid = cobr.id "
                    " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                    " WHERE dcm.domainid = $"<< params.size() <<"::bigint ";
            params.push_back(historyid);
            sql << " AND dcm.historyid = $"<< params.size() <<"::bigint ";
        }
        else
        {
            params.push_back(id);
            sql << " FROM domain_contact_map dcm "
            " JOIN object_registry cobr ON dcm.contactid = cobr.id AND cobr.erdate IS NULL "
            " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
            " WHERE dcm.domainid = $"<< params.size() <<"::bigint ";
        }
        sql << " AND dcm.role = 1 "// admin contact
        " ORDER BY cobr.name ";

        return std::make_pair(sql.str(), params);
    }

    std::vector<InfoDomainOutput> InfoDomain::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)//return data
    {
        std::vector<InfoDomainOutput> result;

        std::pair<std::string, Database::QueryParams> domain_query = make_domain_query(local_timestamp_pg_time_zone_name);
        Database::Result query_result = ctx.get_conn().exec_params(domain_query.first,domain_query.second);

        result.reserve(query_result.size());//alloc

        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoDomainOutput info_domain_output;

            info_domain_output.info_domain_data.id = static_cast<unsigned long long>(query_result[i][0]);//dobr.id

            info_domain_output.info_domain_data.roid = static_cast<std::string>(query_result[i][1]);//dobr.roid

            info_domain_output.info_domain_data.fqdn = static_cast<std::string>(query_result[i][2]);//dobr.name

            info_domain_output.info_domain_data.delete_time = query_result[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][3])));//dobr.erdate

            info_domain_output.info_domain_data.historyid = static_cast<unsigned long long>(query_result[i][5]);//h.id

            info_domain_output.next_historyid = query_result[i][6].isnull() ? Nullable<unsigned long long>()
            : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][6]));//h.next

            info_domain_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][7]));//h.valid_from

            info_domain_output.history_valid_to = query_result[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][8])));//h.valid_to

            info_domain_output.info_domain_data.registrant_handle = static_cast<std::string>(query_result[i][10]);//cor.name

            info_domain_output.info_domain_data.nsset_handle = query_result[i][12].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][12]));//nobr.name

            info_domain_output.info_domain_data.keyset_handle = query_result[i][14].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][14]));//kobr.name

            info_domain_output.info_domain_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i][16]);//clr.handle

            info_domain_output.info_domain_data.create_registrar_handle = static_cast<std::string>(query_result[i][18]);//crr.handle

            info_domain_output.info_domain_data.update_registrar_handle = query_result[i][20].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][20]));//upr.handle

            info_domain_output.info_domain_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][21]));//dobr.crdate

            info_domain_output.info_domain_data.transfer_time = query_result[i][22].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][22])));//oh.trdate

            info_domain_output.info_domain_data.update_time = query_result[i][23].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][23])));//oh.update

            info_domain_output.info_domain_data.expiration_date = query_result[i][24].isnull() ? boost::gregorian::date()
            : boost::gregorian::from_string(static_cast<std::string>(query_result[i][24]));//dh.exdate

            info_domain_output.info_domain_data.authinfopw = static_cast<std::string>(query_result[i][25]);//oh.authinfopw

            info_domain_output.info_domain_data.enum_domain_validation = (query_result[i][26].isnull() || query_result[i][27].isnull())
            ? Nullable<ENUMValidationExtension>()
            : Nullable<ENUMValidationExtension>(ENUMValidationExtension(
                boost::gregorian::from_string(static_cast<std::string>(query_result[i][26]))
                ,static_cast<bool>(query_result[i][27])));

            info_domain_output.info_domain_data.outzone_time = query_result[i][28].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][28]));//outzonedate

            info_domain_output.info_domain_data.cancel_time = query_result[i][29].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][29]));//canceldate

            info_domain_output.info_domain_data.crhistoryid = static_cast<unsigned long long>(query_result[i][30]);//dobr.crhistoryid

            info_domain_output.logd_request_id = query_result[i][31].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][31]));

            info_domain_output.utc_timestamp = query_result[i][32].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][32]));// utc timestamp
            info_domain_output.local_timestamp = query_result[i][33].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][33]));//local zone timestamp

            //admin contacts
            std::pair<std::string, Database::QueryParams> admin_query = make_admin_query(
                    info_domain_output.info_domain_data.id, info_domain_output.info_domain_data.historyid);

            //list of administrative contacts
            Database::Result admin_contact_res = ctx.get_conn().exec_params(admin_query.first, admin_query.second);
            info_domain_output.info_domain_data.admin_contacts.reserve(admin_contact_res.size());
            for(Database::Result::size_type j = 0; j < admin_contact_res.size(); ++j)
            {
                info_domain_output.info_domain_data.admin_contacts.push_back(static_cast<std::string>(admin_contact_res[j][0]));
            }

            result.push_back(info_domain_output);
        }//for res

        return result;
    }

    std::string InfoDomain::explain_analyze(OperationContext& ctx, std::vector<InfoDomainOutput>& result
            , const std::string& local_timestamp_pg_time_zone_name)
    {
        result = exec(ctx,local_timestamp_pg_time_zone_name);
        std::pair<std::string, Database::QueryParams> domain_query = make_domain_query(local_timestamp_pg_time_zone_name);
        std::string query_plan("\nDomain query: EXPLAIN ANALYZE ");
        query_plan += domain_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(domain_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result domain_query_result = ctx.get_conn().exec_params(
            std::string("EXPLAIN ANALYZE ") + domain_query.first,domain_query.second);
        for(Database::Result::size_type i = 0; i < domain_query_result.size(); ++i)
            query_plan += std::string(domain_query_result[i][0])+"\n";

        std::pair<std::string, Database::QueryParams> admin_query = make_admin_query(
                result.at(0).info_domain_data.id, result.at(0).info_domain_data.historyid);
        query_plan += "\nAdmin query: EXPLAIN ANALYZE ";
        query_plan += admin_query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(admin_query.second);
        query_plan += "\n\nPlan:\n";
        Database::Result admin_query_result = ctx.get_conn().exec_params(
                std::string("EXPLAIN ANALYZE ") + domain_query.first,domain_query.second);
        for(Database::Result::size_type i = 0; i < admin_query_result.size(); ++i)
                query_plan += std::string(admin_query_result[i][0])+"\n";

        return query_plan;
    }

}//namespace Fred

