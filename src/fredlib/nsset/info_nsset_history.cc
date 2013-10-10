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

    InfoNssetByHandle::InfoNssetByHandle(const std::string& handle)
        : handle_(handle)
        , lock_(false)
    {}

    InfoNssetByHandle& InfoNssetByHandle::set_lock(bool lock)//set lock object_registry row for nsset
    {
        lock_ = lock;
        return *this;
    }

    InfoNssetOutput InfoNssetByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_res;

        try
        {
            nsset_res = InfoNsset()
                    .set_handle(handle_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_handle(handle_));
            }

            if (nsset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res.at(0);
    }//InfoNssetByHandle::exec

    std::string InfoNssetByHandle::to_string() const
    {
        return Util::format_operation_state("InfoNssetByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle", handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoNssetById::InfoNssetById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoNssetById& InfoNssetById::set_lock(bool lock)//set lock object_registry row for nsset
    {
        lock_ = lock;
        return *this;
    }

    InfoNssetOutput InfoNssetById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_res;

        try
        {
            nsset_res = InfoNsset()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (nsset_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_res.at(0);
    }//InfoNssetById::exec

    std::string InfoNssetById::to_string() const
    {
        return Util::format_operation_state("InfoNssetById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

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
        (std::make_pair("utc_timestamp",boost::lexical_cast<std::string>(utc_timestamp)))
        (std::make_pair("local_timestamp",boost::lexical_cast<std::string>(local_timestamp)))
        (std::make_pair("next_historyid",next_historyid.print_quoted()))
        (std::make_pair("history_valid_from",boost::lexical_cast<std::string>(history_valid_from)))
        (std::make_pair("history_valid_to",history_valid_to.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id.print_quoted()))
        );
    }

    bool InfoNssetOutput::operator==(const InfoNssetOutput& rhs) const
    {
        return info_nsset_data == rhs.info_nsset_data;
    }

    bool InfoNssetOutput::operator!=(const InfoNssetOutput& rhs) const
    {
        return !this->operator ==(rhs);
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


    HistoryInfoNssetById::HistoryInfoNssetById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    HistoryInfoNssetById& HistoryInfoNssetById::set_lock(bool lock)//set lock object_registry row for nsset
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoNssetOutput> HistoryInfoNssetById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_history_res;

        try
        {
            nsset_history_res = InfoNsset()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_history_res;
    }//HistoryInfoNssetById::exec

    std::string HistoryInfoNssetById::to_string() const
    {
        return Util::format_operation_state("HistoryInfoNssetById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    HistoryInfoNssetByHistoryid::HistoryInfoNssetByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    HistoryInfoNssetByHistoryid& HistoryInfoNssetByHistoryid::set_lock(bool lock)//set lock object_registry row for nsset
    {
        lock_ = lock;
        return *this;
    }

    InfoNssetOutput HistoryInfoNssetByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoNssetOutput> nsset_history_res;

        try
        {
            nsset_history_res = InfoNsset()
                    .set_historyid(historyid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (nsset_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (nsset_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return nsset_history_res.at(0);
    }//HistoryInfoNssetByHistoryid::exec

    std::string HistoryInfoNssetByHistoryid::to_string() const
    {
        return Util::format_operation_state("HistoryInfoNssetByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

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

    InfoNsset& InfoNsset::set_lock(bool lock)
    {
        lock_ = lock;
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


        return std::make_pair(sql.str(), params);

    }

    std::pair<std::string, Database::QueryParams> InfoNsset::make_tech_contact_query(
            unsigned long long id, unsigned long long historyid)
    {
        //technical contacts
        Database::QueryParams params;
        std::ostringstream sql;

        sql << "SELECT cobr.name ";
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
        sql << " ORDER BY h.fqdn ";


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

            info_nsset_output.info_nsset_data.tech_check_level = query_result[i][18].isnull() ? Nullable<unsigned long long>()
                       : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][18]));//nt.checklevel

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
                info_nsset_output.info_nsset_data.tech_contacts.push_back(static_cast<std::string>(tech_contact_res[j][0]));
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
                std::vector<std::string> dns_ip;
                dns_ip.reserve(dns_ip_res.size());
                for(Database::Result::size_type k = 0; k < dns_ip_res.size(); ++k)
                {
                    dns_ip.push_back(static_cast<std::string>(dns_ip_res[k][0]));
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

