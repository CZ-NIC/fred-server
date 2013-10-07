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
 *  keyset info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/keyset/info_keyset.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    OldInfoKeyset::OldInfoKeyset(const std::string& handle)
    : handle_(handle)
    , lock_(false)
    {}

    OldInfoKeyset& OldInfoKeyset::set_lock(bool lock)//set lock object_registry row
    {
        lock_ = lock;
        return *this;
    }

    InfoKeysetOut OldInfoKeyset::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        InfoKeysetOut keyset_info_output;

        try
        {
            //info about keyset
            unsigned long long keyset_id = 0;
            {
                Database::Result res = ctx.get_conn().exec_params(std::string(
                "SELECT (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp " // utc timestamp 0
                " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp " // local zone timestamp 1
                " , kobr.crhistoryid "//first historyid 2
                " , kobr.historyid " // last historyid 3
                " , kobr.erdate "// keyset delete time 4
                " , kobr.id,kobr.name,kobr.roid " //keyset 5-7
                " , o.clid,clr.handle " //sponzoring registrar 8-9
                " , kobr.crid, crr.handle "//creating registrar 10-11
                " , o.upid, upr.handle " //updated by registrar 12-13
                " , kobr.crdate,o.trdate,o.update "//registration dates 14-16
                " , o.authinfopw "//authinfo 17
                " FROM object_registry kobr "
                " JOIN keyset k ON kobr.id=k.id "
                " JOIN object o ON k.id=o.id "
                " JOIN registrar clr ON clr.id = o.clid "
                " JOIN registrar crr ON crr.id = kobr.crid "
                " LEFT JOIN registrar upr ON upr.id = o.upid "
                " WHERE kobr.name=UPPER($2::text) AND kobr.erdate IS NULL "
                " AND kobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text)")
                + (lock_ ? std::string(" FOR UPDATE OF kobr") : std::string(""))
                , Database::query_param_list(local_timestamp_pg_time_zone_name)(handle_));

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_keyset_handle(handle_));
                }
                if (res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get keyset"));
                }

                keyset_info_output.utc_timestamp = res[0][0].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][0]));// utc timestamp

                keyset_info_output.local_timestamp = res[0][1].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][1]));//local zone timestamp

                keyset_info_output.info_keyset_data.crhistoryid = static_cast<unsigned long long>(res[0][2]);//kobr.crhistoryid

                keyset_info_output.info_keyset_data.historyid = static_cast<unsigned long long>(res[0][3]);//kobr.historyid

                keyset_info_output.info_keyset_data.delete_time = res[0][4].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][4])));//nobr.erdate

                keyset_id = static_cast<unsigned long long>(res[0][5]);//kobr.id

                keyset_info_output.info_keyset_data.handle = static_cast<std::string>(res[0][6]);//kobr.name

                keyset_info_output.info_keyset_data.roid = static_cast<std::string>(res[0][7]);//kobr.roid

                keyset_info_output.info_keyset_data.sponsoring_registrar_handle = static_cast<std::string>(res[0][9]);//clr.handle

                keyset_info_output.info_keyset_data.create_registrar_handle = static_cast<std::string>(res[0][11]);//crr.handle

                keyset_info_output.info_keyset_data.update_registrar_handle = res[0][13].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[0][13]));//upr.handle

                keyset_info_output.info_keyset_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[0][14]));//kobr.crdate

                keyset_info_output.info_keyset_data.transfer_time = res[0][15].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][15])));//o.trdate

                keyset_info_output.info_keyset_data.update_time = res[0][16].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][16])));//o.update

                keyset_info_output.info_keyset_data.authinfopw = static_cast<std::string>(res[0][17]);//o.authinfopw
            }

            //list of dns keys
            {
                Database::Result kres = ctx.get_conn().exec_params(
                "SELECT d.id, d.flags, d.protocol, d.alg, d.key "
                " FROM dnskey d "
                " WHERE d.keysetid = $1::bigint "
                " ORDER BY d.id "
                , Database::query_param_list(keyset_id));

                keyset_info_output.info_keyset_data.dns_keys.reserve(kres.size());//alloc
                for(Database::Result::size_type i = 0; i < kres.size(); ++i)
                {
                    //unsigned long long dns_key_id = static_cast<unsigned long long>(kres[i][0]);//d.id
                    unsigned short flags = static_cast<unsigned int>(kres[i][1]);//d.flags
                    unsigned short protocol = static_cast<unsigned int>(kres[i][2]);//d.protocol
                    unsigned short alg = static_cast<unsigned int>(kres[i][3]);//d.alg
                    std::string key = static_cast<std::string>(kres[i][4]);//d.key
                    keyset_info_output.info_keyset_data.dns_keys.push_back(DnsKey(flags, protocol, alg, key));

                }//for dns keys
            }//list of dns keyss

            //list of tech contacts
            {
                Database::Result result = ctx.get_conn().exec_params(
                        "SELECT cobr.name "
                        " FROM keyset_contact_map kcm "
                        " JOIN object_registry cobr ON kcm.contactid = cobr.id AND cobr.erdate IS NULL "
                        " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                        " WHERE kcm.keysetid = $1::bigint "
                        " ORDER BY cobr.name "
                , Database::query_param_list(keyset_id));

                keyset_info_output.info_keyset_data.tech_contacts.reserve(result.size());
                for(Database::Result::size_type i = 0; i < result.size(); ++i)
                {
                    keyset_info_output.info_keyset_data.tech_contacts.push_back(
                    static_cast<std::string>(result[i][0]));
                }
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return keyset_info_output;
    }//InfoKeyset::exec

    std::string OldInfoKeyset::to_string() const
    {
        return Util::format_operation_state("InfoKeyset",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    std::string InfoKeysetOut::to_string() const
    {
        return Util::format_data_structure("InfoKeysetOut",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_keyset_data",info_keyset_data.to_string()))
        (std::make_pair("utc_timestamp",boost::lexical_cast<std::string>(utc_timestamp)))
        (std::make_pair("local_timestamp",boost::lexical_cast<std::string>(local_timestamp)))
        );
    }


}//namespace Fred

