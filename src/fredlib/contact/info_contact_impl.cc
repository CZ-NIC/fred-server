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
 *  contact info implementation
 */

#include <string>
#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include "info_contact_impl.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"

namespace Fred
{
    InfoContact::InfoContact()
        : history_query_(false)
        , lock_(false)
        {}

    InfoContact& InfoContact::set_handle(const std::string& contact_handle)
    {
        contact_handle_ = contact_handle;
        return *this;
    }

    InfoContact& InfoContact::set_roid(const std::string& contact_roid)
    {
        contact_roid_ = contact_roid;
        return *this;
    }

    InfoContact& InfoContact::set_id(unsigned long long contact_id)
    {
        contact_id_ = contact_id;
        return *this;
    }

    InfoContact& InfoContact::set_historyid(unsigned long long contact_historyid)
    {
        contact_historyid_ = contact_historyid;
        return *this;
    }

    InfoContact& InfoContact::set_history_timestamp(const boost::posix_time::ptime& history_timestamp)
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoContact& InfoContact::set_history_query(bool history_query)
    {
        history_query_ = history_query;
        return *this;
    }

    InfoContact& InfoContact::set_lock()
    {
        lock_ = true;
        return *this;
    }

    std::pair<std::string, Database::QueryParams> InfoContact::make_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        Database::QueryParams params;
        std::ostringstream sql;

        params.push_back(local_timestamp_pg_time_zone_name);//refered as $1

        //query to get info and lock object_registry row for update if set
        sql << "SELECT cobr.id, cobr.roid, cobr.name "//contact 0-2
        " , (cobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE $1::text " //contact 3
        " , obj.id, h.id , h.next, (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//history 4-7
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//history 8
        " , obj.clid, clr.handle "//sponsoring registrar 9-10
        " , cobr.crid, crr.handle "//creating registrar 11-12
        " , obj.upid, upr.handle "//last updated by registrar 13-14
        " , (cobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 15
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 16
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 17
        " , obj.authinfopw "//transfer passwd 18
        " , cobr.crhistoryid "//first historyid 19
        " , h.request_id "//logd request_id 20
        " , ct.name, ct.organization, ct.street1, ct.street2, ct.street3, ct.city, ct.stateorprovince, ct.postalcode, ct.country "//contact data
        " , ct.telephone, ct.fax, ct.email , ct.notifyemail, ct.vat, ct.ssn, est.type "
        " , ct.disclosename, ct.discloseorganization, ct.discloseaddress, ct.disclosetelephone, ct.disclosefax "
        " , ct.discloseemail, ct.disclosevat, ct.discloseident, ct.disclosenotifyemail "
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp "// utc timestamp 46
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp  "// local zone timestamp 47
        " FROM object_registry cobr ";
        if(history_query_)
        {
            sql << " JOIN object_history obj ON obj.id = cobr.id "
            " JOIN contact_history ct ON ct.historyid = obj.historyid "
            " JOIN history h ON h.id = ct.historyid ";
        }
        else
        {
            sql << " JOIN object obj ON obj.id = cobr.id "
            " JOIN contact ct ON ct.id = obj.id "
            " JOIN history h ON h.id = cobr.historyid ";
        }
        sql << " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = cobr.crid "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " LEFT JOIN enum_ssntype est ON est.id = ct.ssntype "
        " WHERE "
        " cobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) ";

        if(!history_query_)
        {
            sql << " AND cobr.erdate IS NULL ";
        }

        if(contact_handle_.isset())
        {
            params.push_back(contact_handle_);
            sql << " AND cobr.name = UPPER($"<< params.size() <<"::text) ";
        }

        if(contact_roid_.isset())
        {
            params.push_back(contact_roid_);
            sql << " AND cobr.roid = $"<< params.size() <<"::text ";
        }

        if(contact_id_.isset())
        {
            params.push_back(contact_id_);
            sql << " AND cobr.id = $"<< params.size() <<"::bigint ";
        }

        if(contact_historyid_.isset())
        {
            params.push_back(contact_historyid_);
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
            sql << " FOR UPDATE of cobr ";
        }
        else
        {
            sql << " FOR SHARE of cobr ";
        }

        return std::make_pair(sql.str(), params);
    }

    std::vector<InfoContactOutput> InfoContact::exec(OperationContext& ctx
            , const std::string& local_timestamp_pg_time_zone_name
        )
    {
        std::vector<InfoContactOutput> result;

        std::pair<std::string, Database::QueryParams> query = make_query(local_timestamp_pg_time_zone_name);

        Database::Result query_result = ctx.get_conn().exec_params(query.first,query.second);
        result.reserve(query_result.size());//alloc
        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoContactOutput info_contact_output;

            info_contact_output.info_contact_data.id = static_cast<unsigned long long>(query_result[i][0]);//cobr.id
            info_contact_output.info_contact_data.roid = static_cast<std::string>(query_result[i][1]);//cobr.roid

            info_contact_output.info_contact_data.handle = static_cast<std::string>(query_result[i][2]);//cobr.name

            info_contact_output.info_contact_data.delete_time = query_result[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][3])));//cobr.erdate

            info_contact_output.info_contact_data.historyid = static_cast<unsigned long long>(query_result[i][5]);//h.id

            info_contact_output.next_historyid = query_result[i][6].isnull() ? Nullable<unsigned long long>()
            : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][6]));//h.next

            info_contact_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][7]));//h.valid_from

            info_contact_output.history_valid_to = query_result[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][8])));//h.valid_to

            info_contact_output.info_contact_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i][10]);//clr.handle

            info_contact_output.info_contact_data.create_registrar_handle = static_cast<std::string>(query_result[i][12]);//crr.handle

            info_contact_output.info_contact_data.update_registrar_handle = query_result[i][14].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][14]));//upr.handle

            info_contact_output.info_contact_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][15]));//cobr.crdate

            info_contact_output.info_contact_data.transfer_time = query_result[i][16].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][16])));//obj.trdate

            info_contact_output.info_contact_data.update_time = query_result[i][17].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][17])));//obj.update

            info_contact_output.info_contact_data.authinfopw = static_cast<std::string>(query_result[i][18]);//obj.authinfopw

            info_contact_output.info_contact_data.crhistoryid = static_cast<unsigned long long>(query_result[i][19]);//cobr.crhistoryid

            info_contact_output.logd_request_id = query_result[i][20].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][20]));

            info_contact_output.info_contact_data.name = query_result[i][21].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][21]));
            info_contact_output.info_contact_data.organization = query_result[i][22].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][22]));
            info_contact_output.info_contact_data.street1 = query_result[i][23].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][23]));
            info_contact_output.info_contact_data.street2 = query_result[i][24].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][24]));
            info_contact_output.info_contact_data.street3 = query_result[i][25].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][25]));
            info_contact_output.info_contact_data.city = query_result[i][26].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][26]));
            info_contact_output.info_contact_data.stateorprovince = query_result[i][27].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][27]));
            info_contact_output.info_contact_data.postalcode = query_result[i][28].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][28]));
            info_contact_output.info_contact_data.country = query_result[i][29].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][29]));
            info_contact_output.info_contact_data.telephone = query_result[i][30].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][30]));
            info_contact_output.info_contact_data.fax = query_result[i][31].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][31]));
            info_contact_output.info_contact_data.email = query_result[i][32].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][32]));
            info_contact_output.info_contact_data.notifyemail = query_result[i][33].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][33]));
            info_contact_output.info_contact_data.vat = query_result[i][34].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][34]));
            info_contact_output.info_contact_data.ssn = query_result[i][35].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][35]));
            info_contact_output.info_contact_data.ssntype = query_result[i][36].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][36]));
            info_contact_output.info_contact_data.disclosename = static_cast<bool>(query_result[i][37]);
            info_contact_output.info_contact_data.discloseorganization = static_cast<bool>(query_result[i][38]);
            info_contact_output.info_contact_data.discloseaddress = static_cast<bool>(query_result[i][39]);
            info_contact_output.info_contact_data.disclosetelephone = static_cast<bool>(query_result[i][40]);
            info_contact_output.info_contact_data.disclosefax = static_cast<bool>(query_result[i][41]);
            info_contact_output.info_contact_data.discloseemail = static_cast<bool>(query_result[i][42]);
            info_contact_output.info_contact_data.disclosevat = static_cast<bool>(query_result[i][43]);
            info_contact_output.info_contact_data.discloseident = static_cast<bool>(query_result[i][44]);
            info_contact_output.info_contact_data.disclosenotifyemail = static_cast<bool>(query_result[i][45]);

            info_contact_output.utc_timestamp = query_result[i][46].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][46]));// utc timestamp
            info_contact_output.local_timestamp = query_result[i][47].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][47]));//local zone timestamp

            result.push_back(info_contact_output);
        }//for res

        return result;
    }

    std::string InfoContact::explain_analyze(OperationContext& ctx, std::vector<InfoContactOutput>& result
            , const std::string& local_timestamp_pg_time_zone_name)
    {
        result = exec(ctx,local_timestamp_pg_time_zone_name);
        std::pair<std::string, Database::QueryParams> query = make_query(local_timestamp_pg_time_zone_name);
        std::string query_plan("\nEXPLAIN ANALYZE ");
        query_plan += query.first;
        query_plan += "\n\nParams: ";
        query_plan += Util::format_vector(query.second);
        query_plan += "\n\nPlan:\n";

        Database::Result query_result = ctx.get_conn().exec_params(std::string("EXPLAIN ANALYZE ") + query.first,query.second);
        for(Database::Result::size_type i = 0; i < query_result.size(); ++i) query_plan += std::string(query_result[i][0])+"\n";
        return query_plan;
    }

}//namespace Fred

