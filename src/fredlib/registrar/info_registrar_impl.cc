/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  registrar info implementation
 */

#include <algorithm>
#include <string>
#include <sstream>

#include <boost/date_time/posix_time/ptime.hpp>

#include "info_registrar_impl.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"

namespace Fred
{
    InfoRegistrar::InfoRegistrar()
        : lock_(false)
        {}


    InfoRegistrar& InfoRegistrar::set_lock(bool lock)
    {
        lock_ = lock;
        return *this;
    }

    InfoRegistrar& InfoRegistrar::set_inline_view_filter(const Database::ParamQuery& filter_expr)
    {
        info_registrar_inline_view_filter_expr_ = filter_expr;
        return *this;
    }

    InfoRegistrar& InfoRegistrar::set_cte_id_filter(const Database::ParamQuery& cte_id_filter_query)
    {
        info_registrar_id_filter_cte_ = cte_id_filter_query;
        return *this;
    }


    Database::ParamQuery InfoRegistrar::make_registrar_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        Database::ParamQuery info_registrar_query;

        if(info_registrar_id_filter_cte_.isset())
        {
            info_registrar_query("WITH id_filter(id) as (")(info_registrar_id_filter_cte_.get_value())(") ");
        }

        info_registrar_query(
        "SELECT * FROM ("
        "SELECT r.id AS ")(GetAlias::id())(
        " , r.ico AS ")(GetAlias::ico())(
        " , r.dic AS ")(GetAlias::dic())(
        " , btrim(r.varsymb) AS ")(GetAlias::variable_symbol())(
        " , r.vat AS ")(GetAlias::vat_payer())(
        " , r.handle AS ")(GetAlias::handle())(
        " , r.name AS ")(GetAlias::name())(
        " , r.organization AS ")(GetAlias::organization())(
        " , r.street1 AS ")(GetAlias::street1())(
        " , r.street2 AS ")(GetAlias::street2())(
        " , r.street3 AS ")(GetAlias::street3())(
        " , r.city AS ")(GetAlias::city())(
        " , r.stateorprovince AS ")(GetAlias::stateorprovince())(
        " , r.postalcode AS ")(GetAlias::postalcode())(
        " , r.country AS ")(GetAlias::country())(
        " , r.telephone AS ")(GetAlias::telephone())(
        " , r.fax AS ")(GetAlias::fax())(
        " , r.email AS ")(GetAlias::email())(
        " , r.url AS ")(GetAlias::url())(
        " , r.system AS ")(GetAlias::system())(
        " , r.regex AS ")(GetAlias::memo_regex())(
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS ")(GetAlias::utc_timestamp())(
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE ").param_text(local_timestamp_pg_time_zone_name)(")::timestamp AS ")(GetAlias::local_timestamp())(
        " FROM registrar r ");

        if(info_registrar_id_filter_cte_.isset())
        {
            info_registrar_query(" WHERE r.id IN (SELECT id FROM id_filter) ");
        }

        if(lock_)
        {
            info_registrar_query(" FOR UPDATE of r ");
        }

        info_registrar_query(") as tmp");

        //inline view sub-select locking example at:
        //http://www.postgresql.org/docs/9.1/static/sql-select.html#SQL-FOR-UPDATE-SHARE
        if(info_registrar_inline_view_filter_expr_.isset())
        {
            info_registrar_query(" WHERE ")(info_registrar_inline_view_filter_expr_.get_value());
        }

        info_registrar_query(
            " ORDER BY ")(GetAlias::id());

        return info_registrar_query;

    }

    std::vector<InfoRegistrarOutput> InfoRegistrar::exec(OperationContext& ctx
            , const std::string& local_timestamp_pg_time_zone_name
        )
    {
        std::vector<InfoRegistrarOutput> result;

        Database::Result registrar_query_result = ctx.get_conn().exec_params(make_registrar_query(local_timestamp_pg_time_zone_name));
        result.reserve(registrar_query_result.size());
        for(Database::Result::size_type i = 0; i < registrar_query_result.size(); ++i)
        {
            InfoRegistrarOutput info_registrar_output;
            info_registrar_output.info_registrar_data.id = static_cast<unsigned long long>(registrar_query_result[i][GetAlias::id()]);
            info_registrar_output.info_registrar_data.ico = registrar_query_result[i][GetAlias::ico()].isnull() ? Nullable<std::string>()
                : Nullable<std::string>(static_cast<std::string>(registrar_query_result[i][GetAlias::ico()]));
            info_registrar_output.info_registrar_data.dic = registrar_query_result[i][GetAlias::dic()].isnull() ? Nullable<std::string>()
                : Nullable<std::string>(static_cast<std::string>(registrar_query_result[i][GetAlias::dic()]));
            info_registrar_output.info_registrar_data.variable_symbol = registrar_query_result[i][GetAlias::variable_symbol()].isnull() ? Nullable<std::string>()
                : Nullable<std::string>(static_cast<std::string>(registrar_query_result[i][GetAlias::variable_symbol()]));
            info_registrar_output.info_registrar_data.vat_payer = static_cast<bool>(registrar_query_result[i][GetAlias::vat_payer()]);
            info_registrar_output.info_registrar_data.handle = static_cast<std::string>(registrar_query_result[i][GetAlias::handle()]);
            info_registrar_output.info_registrar_data.name = registrar_query_result[i][GetAlias::name()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::name()]));
            info_registrar_output.info_registrar_data.organization = registrar_query_result[i][GetAlias::organization()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::organization()]));
            info_registrar_output.info_registrar_data.street1 = registrar_query_result[i][GetAlias::street1()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::street1()]));
            info_registrar_output.info_registrar_data.street2 = registrar_query_result[i][GetAlias::street2()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::street2()]));
            info_registrar_output.info_registrar_data.street3 = registrar_query_result[i][GetAlias::street3()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::street3()]));
            info_registrar_output.info_registrar_data.city = registrar_query_result[i][GetAlias::city()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::city()]));
            info_registrar_output.info_registrar_data.stateorprovince = registrar_query_result[i][GetAlias::stateorprovince()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::stateorprovince()]));
            info_registrar_output.info_registrar_data.postalcode = registrar_query_result[i][GetAlias::postalcode()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::postalcode()]));
            info_registrar_output.info_registrar_data.country = registrar_query_result[i][GetAlias::country()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::country()]));
            info_registrar_output.info_registrar_data.telephone = registrar_query_result[i][GetAlias::telephone()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::telephone()]));
            info_registrar_output.info_registrar_data.fax = registrar_query_result[i][GetAlias::fax()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::fax()]));
            info_registrar_output.info_registrar_data.email = registrar_query_result[i][GetAlias::email()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::email()]));
            info_registrar_output.info_registrar_data.url = registrar_query_result[i][GetAlias::url()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::url()]));
            info_registrar_output.info_registrar_data.system = registrar_query_result[i][GetAlias::system()].isnull() ? Nullable<bool>()
                : Nullable<bool> (static_cast<bool>(registrar_query_result[i][GetAlias::system()]));
            info_registrar_output.info_registrar_data.payment_memo_regex = registrar_query_result[i][GetAlias::memo_regex()].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][GetAlias::memo_regex()]));
            info_registrar_output.utc_timestamp = registrar_query_result[i][GetAlias::utc_timestamp()].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(registrar_query_result[i][GetAlias::utc_timestamp()]));// utc timestamp
            info_registrar_output.local_timestamp = registrar_query_result[i][GetAlias::local_timestamp()].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(registrar_query_result[i][GetAlias::local_timestamp()]));//local zone timestamp

            result.push_back(info_registrar_output);
        }//for res

        return result;
    }

}//namespace Fred

