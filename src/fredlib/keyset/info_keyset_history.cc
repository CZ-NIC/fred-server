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
 *  @file info_keyset_history.cc
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

    std::vector<InfoKeysetHistoryOutput> InfoKeysetHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoKeysetHistoryOutput> keyset_history_res;

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
                    InfoKeysetHistoryOutput keyset_history_output;

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

    std::ostream& operator<<(std::ostream& os, const InfoKeysetHistory& i)
    {
        return os << "#InfoKeysetHistory roid: " << i.roid_
            << " history_timestamp: " << i.history_timestamp_.print_quoted()
            << " lock: " << i.lock_;
    }

    std::string InfoKeysetHistory::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred

