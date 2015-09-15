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

    InfoContact& InfoContact::set_inline_view_filter(const Database::ParamQuery& filter_expr)
    {
        info_contact_inline_view_filter_expr_ = filter_expr;
        return *this;
    }

    InfoContact& InfoContact::set_cte_id_filter(const Database::ParamQuery& cte_id_filter_query)
    {
        info_contact_id_filter_cte_ = cte_id_filter_query;
        return *this;
    }


    Database::ParamQuery InfoContact::make_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        const Database::ReusableParameter p_local_zone(local_timestamp_pg_time_zone_name, "text");
        Database::ParamQuery info_contact_query;

        if(info_contact_id_filter_cte_.isset())
        {
            info_contact_query("WITH id_filter(id) as (")(info_contact_id_filter_cte_.get_value())(") ");
        }

        info_contact_query(
        "SELECT * FROM ("
        "SELECT cobr.id AS ")(GetAlias::id())(
        " , cobr.roid AS ")(GetAlias::roid())(
        " , cobr.name AS ")(GetAlias::handle())(
        " , (cobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::delete_time())(
        " , h.id AS ")(GetAlias::historyid())(
        " , h.next AS ")(GetAlias::next_historyid())(
        " , (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::history_valid_from())(
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::history_valid_to())(
        " , obj.clid AS ")(GetAlias::sponsoring_registrar_id())(
        " , clr.handle AS ")(GetAlias::sponsoring_registrar_handle())(
        " , cobr.crid AS ")(GetAlias::creating_registrar_id())(
        " , crr.handle AS ")(GetAlias::creating_registrar_handle())(
        " , obj.upid AS ")(GetAlias::last_updated_by_registrar_id())(
        " , upr.handle AS ")(GetAlias::last_updated_by_registrar_handle())(
        " , (cobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::creation_time())(
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::transfer_time())(
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE ").param(p_local_zone)(" AS ")(GetAlias::update_time())(
        " , obj.authinfopw AS ")(GetAlias::authinfopw())(
        " , cobr.crhistoryid AS ")(GetAlias::first_historyid())(
        " , h.request_id AS ")(GetAlias::logd_request_id())(
        " , ct.name AS ")(GetAlias::name())(
        " , ct.organization AS ")(GetAlias::organization())(
        " , ct.street1 AS ")(GetAlias::street1())(
        " , ct.street2 AS ")(GetAlias::street2())(
        " , ct.street3 AS ")(GetAlias::street3())(
        " , ct.city AS ")(GetAlias::city())(
        " , ct.stateorprovince AS ")(GetAlias::stateorprovince())(
        " , ct.postalcode AS ")(GetAlias::postalcode())(
        " , ct.country AS ")(GetAlias::country())(
        " , ct.telephone AS ")(GetAlias::telephone())(
        " , ct.fax AS ")(GetAlias::fax())(
        " , ct.email AS ")(GetAlias::email())(
        " , ct.notifyemail AS ")(GetAlias::notifyemail())(
        " , ct.vat AS ")(GetAlias::vat())(
        " , ct.ssn AS ")(GetAlias::ssn())(
        " , est.type AS ")(GetAlias::ssntype())(
        " , ct.disclosename AS ")(GetAlias::disclosename())(
        " , ct.discloseorganization AS ")(GetAlias::discloseorganization())(
        " , ct.discloseaddress AS ")(GetAlias::discloseaddress())(
        " , ct.disclosetelephone AS ")(GetAlias::disclosetelephone())(
        " , ct.disclosefax AS ")(GetAlias::disclosefax())(
        " , ct.discloseemail AS ")(GetAlias::discloseemail())(
        " , ct.disclosevat AS ")(GetAlias::disclosevat())(
        " , ct.discloseident AS ")(GetAlias::discloseident())(
        " , ct.disclosenotifyemail  AS ")(GetAlias::disclosenotifyemail())(
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS ")(GetAlias::utc_timestamp())(
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE ").param(p_local_zone)(")::timestamp AS ")(GetAlias::local_timestamp())(
        " , ct.warning_letter AS ")(GetAlias::domain_expiration_letter_preference())(
        " FROM object_registry cobr ");
        if(history_query_)
        {
            info_contact_query(
            " JOIN object_history obj ON obj.id = cobr.id "
            " JOIN contact_history ct ON ct.historyid = obj.historyid "
            " JOIN history h ON h.id = ct.historyid ");
        }
        else
        {
            info_contact_query(
            " JOIN object obj ON obj.id = cobr.id "
            " JOIN contact ct ON ct.id = obj.id "
            " JOIN history h ON h.id = cobr.historyid ");
        }

        info_contact_query(
        " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = cobr.crid "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " LEFT JOIN enum_ssntype est ON est.id = ct.ssntype "
        " WHERE "
        " cobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) ");

        if(info_contact_id_filter_cte_.isset())
        {
            info_contact_query(" AND cobr.id IN (SELECT id FROM id_filter) ");
        }

        if(!history_query_)
        {
            info_contact_query(" AND cobr.erdate IS NULL ");
        }

        if(lock_)
        {
            info_contact_query(" FOR UPDATE of cobr ");
        }
        else
        {
            info_contact_query(" FOR SHARE of cobr ");
        }
        info_contact_query(") as tmp");

        //inline view sub-select locking example at:
        //http://www.postgresql.org/docs/9.1/static/sql-select.html#SQL-FOR-UPDATE-SHARE
        if(info_contact_inline_view_filter_expr_.isset())
        {
            info_contact_query(" WHERE ")(info_contact_inline_view_filter_expr_.get_value());
        }

        info_contact_query(
            " ORDER BY ")(GetAlias::historyid())(" DESC ");

        return info_contact_query;
    }

    std::vector<InfoContactOutput> InfoContact::exec(OperationContext& ctx
            , const std::string& local_timestamp_pg_time_zone_name
        )
    {
        std::vector<InfoContactOutput> result;

        Database::ParamQuery contact_param_query = make_query(local_timestamp_pg_time_zone_name);
        std::pair<std::string, Database::QueryParams> query = contact_param_query.get_query();

        Database::Result query_result = ctx.get_conn().exec_params(query.first,query.second);
        result.reserve(query_result.size());
        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoContactOutput info_contact_output;

            info_contact_output.info_contact_data.id = static_cast<unsigned long long>(query_result[i][GetAlias::id()]);
            info_contact_output.info_contact_data.roid = static_cast<std::string>(query_result[i][GetAlias::roid()]);

            info_contact_output.info_contact_data.handle = static_cast<std::string>(query_result[i][GetAlias::handle()]);

            info_contact_output.info_contact_data.delete_time = query_result[i][GetAlias::delete_time()].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::delete_time()])));

            info_contact_output.info_contact_data.historyid = static_cast<unsigned long long>(query_result[i][GetAlias::historyid()]);

            info_contact_output.next_historyid = query_result[i][GetAlias::next_historyid()].isnull() ? Nullable<unsigned long long>()
            : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][GetAlias::next_historyid()]));

            info_contact_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::history_valid_from()]));

            info_contact_output.history_valid_to = query_result[i][GetAlias::history_valid_to()].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::history_valid_to()])));

            info_contact_output.info_contact_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i][GetAlias::sponsoring_registrar_handle()]);

            info_contact_output.info_contact_data.create_registrar_handle = static_cast<std::string>(query_result[i][GetAlias::creating_registrar_handle()]);

            info_contact_output.info_contact_data.update_registrar_handle = query_result[i][GetAlias::last_updated_by_registrar_handle()].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::last_updated_by_registrar_handle()]));

            info_contact_output.info_contact_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::creation_time()]));

            info_contact_output.info_contact_data.transfer_time = query_result[i][GetAlias::transfer_time()].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::transfer_time()])));

            info_contact_output.info_contact_data.update_time = query_result[i][GetAlias::update_time()].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::update_time()])));

            info_contact_output.info_contact_data.authinfopw = static_cast<std::string>(query_result[i][GetAlias::authinfopw()]);

            info_contact_output.info_contact_data.crhistoryid = static_cast<unsigned long long>(query_result[i][GetAlias::first_historyid()]);

            info_contact_output.logd_request_id = query_result[i][GetAlias::logd_request_id()].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][GetAlias::logd_request_id()]));

            info_contact_output.info_contact_data.name = query_result[i][GetAlias::name()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::name()]));
            info_contact_output.info_contact_data.organization = query_result[i][GetAlias::organization()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::organization()]));
            Contact::PlaceAddress place;
            place.street1 = static_cast<std::string>(query_result[i][GetAlias::street1()]);
            place.street2 = query_result[i][GetAlias::street2()].isnull() ? Optional<std::string>()
                    : Optional<std::string> (static_cast<std::string>(query_result[i][GetAlias::street2()]));
            place.street3 = query_result[i][GetAlias::street3()].isnull() ? Optional<std::string>()
                    : Optional<std::string> (static_cast<std::string>(query_result[i][GetAlias::street3()]));
            place.city = static_cast<std::string>(query_result[i][GetAlias::city()]);
            place.stateorprovince = query_result[i][GetAlias::stateorprovince()].isnull() ? Optional<std::string>()
                    : Optional<std::string> (static_cast<std::string>(query_result[i][GetAlias::stateorprovince()]));
            place.postalcode = static_cast<std::string>(query_result[i][GetAlias::postalcode()]);
            place.country = static_cast<std::string>(query_result[i][GetAlias::country()]);
            info_contact_output.info_contact_data.place = place;
            info_contact_output.info_contact_data.telephone = query_result[i][GetAlias::telephone()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::telephone()]));
            info_contact_output.info_contact_data.fax = query_result[i][GetAlias::fax()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::fax()]));
            info_contact_output.info_contact_data.email = query_result[i][GetAlias::email()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::email()]));
            info_contact_output.info_contact_data.notifyemail = query_result[i][GetAlias::notifyemail()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::notifyemail()]));
            info_contact_output.info_contact_data.vat = query_result[i][GetAlias::vat()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::vat()]));
            info_contact_output.info_contact_data.ssn = query_result[i][GetAlias::ssn()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::ssn()]));
            info_contact_output.info_contact_data.ssntype = query_result[i][GetAlias::ssntype()].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][GetAlias::ssntype()]));
            info_contact_output.info_contact_data.disclosename = static_cast<bool>(query_result[i][GetAlias::disclosename()]);
            info_contact_output.info_contact_data.discloseorganization = static_cast<bool>(query_result[i][GetAlias::discloseorganization()]);
            info_contact_output.info_contact_data.discloseaddress = static_cast<bool>(query_result[i][GetAlias::discloseaddress()]);
            info_contact_output.info_contact_data.disclosetelephone = static_cast<bool>(query_result[i][GetAlias::disclosetelephone()]);
            info_contact_output.info_contact_data.disclosefax = static_cast<bool>(query_result[i][GetAlias::disclosefax()]);
            info_contact_output.info_contact_data.discloseemail = static_cast<bool>(query_result[i][GetAlias::discloseemail()]);
            info_contact_output.info_contact_data.disclosevat = static_cast<bool>(query_result[i][GetAlias::disclosevat()]);
            info_contact_output.info_contact_data.discloseident = static_cast<bool>(query_result[i][GetAlias::discloseident()]);
            info_contact_output.info_contact_data.disclosenotifyemail = static_cast<bool>(query_result[i][GetAlias::disclosenotifyemail()]);

            info_contact_output.utc_timestamp = query_result[i][GetAlias::utc_timestamp()].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::utc_timestamp()]));
            info_contact_output.local_timestamp = query_result[i][GetAlias::local_timestamp()].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][GetAlias::local_timestamp()]));

            info_contact_output.info_contact_data.warning_letter = query_result[i][GetAlias::domain_expiration_letter_preference()].isnull() ? Nullable<bool>()
                    : Nullable<bool>(static_cast<bool>(query_result[i][GetAlias::domain_expiration_letter_preference()]));

            Database::QueryParams params;
            params.push_back(info_contact_output.info_contact_data.id);
            std::string sql;
            if(history_query_) {
                sql = "SELECT type,company_name,street1,street2,street3,"
                             "city,stateorprovince,postalcode,country "
                      "FROM contact_address_history "
                      "WHERE contactid=$1::bigint AND "
                            "historyid=$2::bigint";
                params.push_back(info_contact_output.info_contact_data.historyid);
            }
            else {
                sql = "SELECT type,company_name,street1,street2,street3,"
                             "city,stateorprovince,postalcode,country "
                      "FROM contact_address "
                      "WHERE contactid=$1::bigint";
            }
            Database::Result subquery_result = ctx.get_conn().exec_params(sql, params);
            ContactAddressList addresses;
            for(::size_t idx = 0; idx < subquery_result.size(); ++idx) {
                const struct ContactAddressType type(ContactAddressType::from_string(
                     static_cast< std::string >(subquery_result[idx]["type"])));
                struct ContactAddress address;
                if (!subquery_result[idx]["company_name"].isnull()) {
                    address.company_name = static_cast< std::string >(subquery_result[idx]["company_name"]);
                }
                if (!subquery_result[idx]["street1"].isnull()) {
                    address.street1 = static_cast< std::string >(subquery_result[idx]["street1"]);
                }
                if (!subquery_result[idx]["street2"].isnull()) {
                    address.street2 = static_cast< std::string >(subquery_result[idx]["street2"]);
                }
                if (!subquery_result[idx]["street3"].isnull()) {
                    address.street3 = static_cast< std::string >(subquery_result[idx]["street3"]);
                }
                if (!subquery_result[idx]["city"].isnull()) {
                    address.city = static_cast< std::string >(subquery_result[idx]["city"]);
                }
                if (!subquery_result[idx]["stateorprovince"].isnull()) {
                    address.stateorprovince = static_cast< std::string >(subquery_result[idx]["stateorprovince"]);
                }
                if (!subquery_result[idx]["postalcode"].isnull()) {
                    address.postalcode = static_cast< std::string >(subquery_result[idx]["postalcode"]);
                }
                if (!subquery_result[idx]["country"].isnull()) {
                    address.country = static_cast< std::string >(subquery_result[idx]["country"]);
                }
                addresses[type] = address;
            }
            if (!addresses.empty()) {
                info_contact_output.info_contact_data.addresses = addresses;
            }

            result.push_back(info_contact_output);
        }//for res

        return result;
    }

}//namespace Fred

