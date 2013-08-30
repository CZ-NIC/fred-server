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
 *  @file info_domain.cc
 *  domain info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/domain_name.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    InfoDomain::InfoDomain(const std::string& fqdn)
    : fqdn_(fqdn)
    , lock_(false)
    {}

    InfoDomain& InfoDomain::set_lock(bool lock)//set lock object_registry row for domain
    {
        lock_ = lock;
        return *this;
    }

    InfoDomainOutput InfoDomain::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        InfoDomainOutput domain_info_output;

        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Domain::rem_trailing_dot(fqdn_);

            //info about domain and optionally lock object_registry row for update
            unsigned long long domain_id = 0;
            {
                Database::Result res = ctx.get_conn().exec_params(
                "SELECT dobr.id,dobr.name,dobr.roid,d.zone,d.nsset,nobr.name, "// domain, zone, nsset 0-5
                " cor.id,cor.name,c.name,c.organization, "// registrant 6-9
                " o.clid,clr.handle , " // sponsoring registrar 10-11
                " dobr.crid, crr.handle, "//creating registrar 12-13
                " o.upid, upr.handle, "//updated by registrar 14-15
                " dobr.crdate,o.trdate,o.update, "// registration dates 16-18
                " o.authinfopw, " // authinfo 19
                " d.exdate, "// expiration time 20
                // outzone data and cancel date from enum_parameters compute 21-22
                " (((d.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval)::timestamp "
                " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
                "    AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as outzonedate "
                " ,(((d.exdate + (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval)::timestamp "
                " + (SELECT val || ' hours' FROM enum_parameters WHERE name = 'regular_day_procedure_period')::interval) "
                " AT TIME ZONE (SELECT val FROM enum_parameters WHERE name = 'regular_day_procedure_zone'))::timestamp as canceldate "
                " , d.keyset, kobr.name "// keyset id and keyset handle 23-24
                " , ev.exdate, ev.publish "//enumval 25-26
                " , dobr.erdate "// domain delete time 27
                " , dobr.historyid as historyid " // last historyid 28
                " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp " // utc timestamp 29
                " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp " // local zone timestamp 30
                " , dobr.crhistoryid "//first historyid 31
                " FROM object_registry dobr "
                " JOIN domain d ON dobr.id=d.id "
                " JOIN object o ON d.id=o.id "
                " JOIN registrar clr ON clr.id = o.clid "
                " JOIN registrar crr ON crr.id = dobr.crid "
                " JOIN contact c ON d.registrant=c.id "
                " JOIN object_registry cor ON c.id=cor.id "
                " LEFT JOIN object_registry nobr ON nobr.id = d.nsset AND nobr.erdate IS NULL "
                " AND nobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) "
                " LEFT JOIN object_registry kobr ON kobr.id = d.keyset AND kobr.erdate IS NULL "
                 " AND kobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) "
                " LEFT JOIN registrar upr ON upr.id = o.upid "
                " LEFT JOIN  enumval ev ON ev.domainid = d.id "
                " WHERE dobr.name=LOWER($2::text) AND dobr.erdate IS NULL "
                " AND dobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text)"
                + (lock_ ? std::string(" FOR UPDATE OF dobr") : std::string(""))
                , Database::query_param_list(local_timestamp_pg_time_zone_name)(no_root_dot_fqdn));

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_domain_fqdn(fqdn_));
                }
                if (res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get domain"));
                }

                domain_id = static_cast<unsigned long long>(res[0][0]);//dobr.id

                domain_info_output.info_domain_data.fqdn = static_cast<std::string>(res[0][1]);//dobr.name

                domain_info_output.info_domain_data.roid = static_cast<std::string>(res[0][2]);//dobr.roid

                domain_info_output.info_domain_data.nsset_handle = res[0][5].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][5]));//nobr.name

                domain_info_output.info_domain_data.registrant_handle = static_cast<std::string>(res[0][7]);

                domain_info_output.info_domain_data.sponsoring_registrar_handle = static_cast<std::string>(res[0][11]);//clr.handle

                domain_info_output.info_domain_data.create_registrar_handle = static_cast<std::string>(res[0][13]);//crr.handle

                domain_info_output.info_domain_data.update_registrar_handle = res[0][15].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[0][15]));//upr.handle

                domain_info_output.info_domain_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[0][16]));//dobr.crdate

                domain_info_output.info_domain_data.transfer_time = res[0][17].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][17])));//o.trdate

                domain_info_output.info_domain_data.update_time = res[0][18].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][18])));//o.update

                domain_info_output.info_domain_data.authinfopw = static_cast<std::string>(res[0][19]);//o.authinfopw

                domain_info_output.info_domain_data.expiration_date = res[0][20].isnull() ? boost::gregorian::date()
                : boost::gregorian::from_string(static_cast<std::string>(res[0][20]));//d.exdate

                domain_info_output.info_domain_data.outzone_time = res[0][21].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][21]));//outzonedate

                domain_info_output.info_domain_data.cancel_time = res[0][22].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][22]));//canceldate

                domain_info_output.info_domain_data.keyset_handle = res[0][24].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(res[0][24]));//kobr.name

                domain_info_output.info_domain_data.enum_domain_validation = (res[0][25].isnull() || res[0][26].isnull())
                        ? Nullable<ENUMValidationExtension > ()
                        : Nullable<ENUMValidationExtension > (ENUMValidationExtension(
                            boost::gregorian::from_string(static_cast<std::string>(res[0][25]))
                            ,static_cast<bool>(res[0][26])));

                domain_info_output.info_domain_data.delete_time = res[0][27].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][27])));//dobr.erdate

                domain_info_output.info_domain_data.historyid = static_cast<unsigned long long>(res[0][28]);//last historyid

                domain_info_output.utc_timestamp = res[0][29].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][29]));// utc timestamp

                domain_info_output.local_timestamp = res[0][30].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][30]));//local zone timestamp

                domain_info_output.info_domain_data.crhistoryid = static_cast<unsigned long long>(res[0][31]);//dobr.crhistoryid
            }

            //list of administrative contacts
            {
                Database::Result result = ctx.get_conn().exec_params(
                "SELECT cobr.name "
                " FROM domain_contact_map dcm "
                " JOIN object_registry cobr ON dcm.contactid = cobr.id AND cobr.erdate IS NULL "
                " JOIN enum_object_type ceot ON ceot.id = cobr.type AND ceot.name='contact'::text "
                " WHERE dcm.domainid = $1::bigint "
                " AND dcm.role = 1 "// admin contact
                " ORDER BY cobr.name "
                , Database::query_param_list(domain_id));

                domain_info_output.info_domain_data.admin_contacts.reserve(result.size());
                for(Database::Result::size_type i = 0; i < result.size(); ++i)
                {
                    domain_info_output.info_domain_data.admin_contacts.push_back(
                    static_cast<std::string>(result[i][0]));
                }
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return domain_info_output;
    }//InfoDomain::exec

    std::ostream& operator<<(std::ostream& os, const InfoDomain& i)
    {
        return os << "#InfoDomain fqdn: " << i.fqdn_
                << " lock: " << i.lock_
                ;
    }
    std::string InfoDomain::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred

