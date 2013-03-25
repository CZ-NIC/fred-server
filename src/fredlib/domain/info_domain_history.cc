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
 *  @file info_domain_history.cc
 *  domain history info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/domain/info_domain_history.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    InfoDomainHistory::InfoDomainHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp
            , const std::string& registrar)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , registrar_(registrar)
        , lock_(false)
    {}

    InfoDomainHistory::InfoDomainHistory(const std::string& roid, const std::string& registrar)
    : roid_(roid)
    , registrar_(registrar)
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

    std::vector<InfoDomainHistoryData> InfoDomainHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoDomainHistoryData> domain_history_info_data;

        try
        {
            //check roid and lock object_registry row for update if set
            {
                Database::Result res = ctx.get_conn().exec_params(
                    std::string("SELECT id FROM object_registry WHERE roid = $1::text "
                    " AND erdate IS NULL AND type = ( SELECT id FROM enum_object_type eot "
                    " WHERE eot.name='domain'::text) ")
                    + (lock_ ? std::string(" FOR UPDATE") : std::string(""))
                    , Database::query_param_list(roid_));

                if (res.size() != 1)
                {
                    std::string errmsg("|| not found:roid: ");
                    errmsg += boost::replace_all_copy(roid_,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw IDHEX(errmsg.c_str());
                }
            }

            //check registrar exists
            //TODO: check registrar access
            {
                Database::Result res = ctx.get_conn().exec_params(
                        "SELECT id FROM registrar WHERE handle = UPPER($1::text)"
                    , Database::query_param_list(registrar_));

                if (res.size() != 1)
                {
                    std::string errmsg("|| not found:registrar: ");
                    errmsg += boost::replace_all_copy(registrar_,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw IDHEX(errmsg.c_str());
                }
            }

            //info about domain history by roid and optional history timestamp
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
                std::string("SELECT dobr.id, dobr.roid, dobr.name, dobr.erdate " //domain 0-3
                ", oh.historyid, h.id , h.next, h.valid_from, h.valid_to " //historyid 4-8
                " , cor.id, cor.name " //registrant 9-10
                " , dh.nsset, nobr.name "//nsset id and nsset handle 11-12
                " , dh.keyset, kobr.name " //keyset id and keyset handle 13-14
                " , oh.clid, clr.handle "//sponsoring registrar 15-16
                " , dobr.crid, crr.handle "//creating registrar 16-18
                " , oh.upid, upr.handle "//last updated by registrar 19-20
                " , dobr.crdate, oh.trdate, oh.update, dh.exdate "//registration dates 21-24
                " , oh.authinfopw "//transfer passwd 25
                " , evh.exdate, evh.publish "//enumval_history 26-27
                //outzone data and cancel date from enum_parameters compute 28-29
                " ,(((dh.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval)::timestamp "
                " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
                " AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as outzonedate "
                " ,(((dh.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval)::timestamp "
                " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
                " AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as canceldate "
                " , dobr.crhistoryid " //first historyid 30
                " FROM object_registry dobr "
                " JOIN object_history oh ON oh.id = dobr.id "
                " JOIN domain_history dh ON dh.historyid = oh.historyid "
                " JOIN history h ON h.id = dh.historyid "
                " JOIN object_registry cor ON dh.registrant=cor.id "
                " JOIN registrar clr ON clr.id = oh.clid "
                " JOIN registrar crr ON crr.id = dobr.crid "
                " LEFT JOIN object_registry nobr ON nobr.id = dh.nsset "
                " AND nobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) "
                " LEFT JOIN object_registry kobr ON kobr.id = dh.keyset "
                " AND kobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) "
                " LEFT JOIN registrar upr ON upr.id = oh.upid "
                " LEFT JOIN  enumval_history evh ON evh.domainid = dh.id AND evh.historyid = h.id"
                " WHERE "
                " dobr.roid = $1::text "
                " AND dobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) ")
                + (history_timestamp_.isset()
                ? " AND h.valid_from <= ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' "
                  " AND ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' < h.valid_to "
                  " ORDER BY h.id DESC"
                : " ORDER BY h.id DESC ")
                , params);

                if (res.size() == 0)
                {
                    std::string errmsg("|| not found:roid: ");
                    errmsg += boost::replace_all_copy(roid_,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw IDHEX(errmsg.c_str());
                }

                domain_history_info_data.reserve(res.size());//alloc
                for(Database::Result::size_type i = 0; i < res.size(); ++i)
                {
                    unsigned long long domain_id = 0;//domain id
                    InfoDomainHistoryData domain_history_element;

                    domain_id = static_cast<unsigned long long>(res[i][0]);//dobr.id

                    domain_history_element.roid = static_cast<std::string>(res[i][1]);//dobr.roid

                    domain_history_element.fqdn = static_cast<std::string>(res[i][2]);//dobr.name

                    domain_history_element.delete_time = res[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][3])));//dobr.erdate

                    domain_history_element.historyid = static_cast<unsigned long long>(res[i][4]);//oh.historyid

                    domain_history_element.next_historyid = res[i][6].isnull() ? Nullable<unsigned long long>()
                    : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][6]));//h.next

                    domain_history_element.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(res[i][7]));//h.valid_from

                    domain_history_element.history_valid_to = res[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][8])));//h.valid_to

                    domain_history_element.registrant_handle = static_cast<std::string>(res[i][10]);//cor.name

                    domain_history_element.nsset_handle = res[i][12].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[i][12]));//nobr.name

                    domain_history_element.nsset_handle = res[i][14].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[i][14]));//kobr.name

                    domain_history_element.sponsoring_registrar_handle = static_cast<std::string>(res[i][16]);//clr.handle

                    domain_history_element.create_registrar_handle = static_cast<std::string>(res[i][18]);//crr.handle

                    domain_history_element.update_registrar_handle = res[i][20].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[i][20]));//upr.handle

                    domain_history_element.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[i][21]));//dobr.crdate

                    domain_history_element.transfer_time = res[i][22].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][22])));//oh.trdate

                    domain_history_element.update_time = res[i][23].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][23])));//oh.update

                    domain_history_element.expiration_date = res[i][24].isnull() ? boost::gregorian::date()
                    : boost::gregorian::from_string(static_cast<std::string>(res[i][24]));//dh.exdate

                    domain_history_element.authinfopw = static_cast<std::string>(res[i][25]);//oh.authinfopw

                    domain_history_element.enum_domain_validation = (res[i][26].isnull() || res[i][27].isnull())
                    ? Nullable<ENUMValidationExtension>()
                    : Nullable<ENUMValidationExtension>(ENUMValidationExtension(
                        boost::gregorian::from_string(static_cast<std::string>(res[i][26]))
                        ,static_cast<bool>(res[i][27])));

                    domain_history_element.outzone_time = res[i][28].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                    : boost::posix_time::time_from_string(static_cast<std::string>(res[i][28]));//outzonedate

                    domain_history_element.cancel_time = res[i][29].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                    : boost::posix_time::time_from_string(static_cast<std::string>(res[i][29]));//canceldate

                    domain_history_element.crhistoryid = static_cast<unsigned long long>(res[i][30]);//dobr.crhistoryid

                    //list of historic administrative contacts
                    Database::Result admin_contact_res = ctx.get_conn().exec_params(
                        "SELECT dcmh.historyid, cobr.name "
                        " FROM domain_contact_map_history dcmh "
                        " JOIN object_registry cobr ON dcmh.contactid = cobr.id "
                        " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                        " WHERE dcmh.domainid = $1::bigint "
                        " AND dcmh.historyid = $2::bigint "
                        " AND dcmh.role = 1 "// admin contact
                        " ORDER BY dcmh.historyid , cobr.name "
                    , Database::query_param_list(domain_id)(domain_history_element.historyid));

                    domain_history_element.admin_contacts.reserve(admin_contact_res.size());
                    for(Database::Result::size_type j = 0; j < admin_contact_res.size(); ++j)
                    {
                        domain_history_element.admin_contacts.push_back(static_cast<std::string>(admin_contact_res[j][1]));
                    }

                    domain_history_info_data.push_back(domain_history_element);
                }//for res
            }//if roid
        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<InfoDomainHistoryException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }
        return domain_history_info_data;
    }//InfoDomain::exec

}//namespace Fred

