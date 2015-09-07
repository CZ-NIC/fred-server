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

    InfoRegistrar& InfoRegistrar::set_handle(const std::string& registrar_handle)
    {
        registrar_handle_ = registrar_handle;
        return *this;
    }

    InfoRegistrar& InfoRegistrar::set_id(unsigned long long registrar_id)
    {
        registrar_id_ = registrar_id;
        return *this;
    }

    InfoRegistrar& InfoRegistrar::set_lock(bool lock)
    {
        lock_ = lock;
        return *this;
    }

    std::pair<std::string, Database::QueryParams> InfoRegistrar::make_registrar_query(const std::string& local_timestamp_pg_time_zone_name)
    {
        Database::QueryParams params;
        std::ostringstream sql;
        Util::HeadSeparator where_and_separator(" WHERE "," AND ");

        params.push_back(local_timestamp_pg_time_zone_name);//refered as $1

        //query to get info and lock registrar row for update if set
        sql <<"SELECT r.id, r.ico, r.dic, r.varsymb, r.vat AS vat_ " //0-4
            ", r.handle, r.name, r.organization "//5-7
            ", r.street1, r.street2, r.street3, r.city, r.stateorprovince, r.postalcode, r.country " //8-14
            ", r.telephone, r.fax, r.email, r.url, r.system, r.regex "//15-20
            " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp "// utc timestamp 21
            " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp "// local zone timestamp 22
            " FROM registrar r ";

        if(registrar_handle_.isset())
        {
            params.push_back(registrar_handle_);
            sql << where_and_separator.get() << " r.handle = UPPER($"<< params.size() <<"::text) ";
        }

        if(registrar_id_.isset())
        {
            params.push_back(registrar_id_);
            sql << where_and_separator.get() << " r.id = $"<< params.size() <<"::bigint ";
        }

        sql << " ORDER BY r.id ";

        if(lock_)
        {
            sql << " FOR UPDATE of r ";
        }

        return std::make_pair(sql.str(), params);
    }

    std::vector<InfoRegistrarOutput> InfoRegistrar::exec(OperationContext& ctx
            , const std::string& local_timestamp_pg_time_zone_name
        )
    {
        std::vector<InfoRegistrarOutput> result;

        std::pair<std::string, Database::QueryParams> registrar_query = make_registrar_query(local_timestamp_pg_time_zone_name);

        Database::Result registrar_query_result = ctx.get_conn().exec_params(registrar_query.first,registrar_query.second);
        result.reserve(registrar_query_result.size());
        for(Database::Result::size_type i = 0; i < registrar_query_result.size(); ++i)
        {
            InfoRegistrarOutput info_registrar_output;
            info_registrar_output.info_registrar_data.id = static_cast<unsigned long long>(registrar_query_result[i][0]);//r.id
            info_registrar_output.info_registrar_data.ico = registrar_query_result[i][1].isnull() ? Nullable<std::string>()
                : Nullable<std::string>(static_cast<std::string>(registrar_query_result[i][1]));//r.ico
            info_registrar_output.info_registrar_data.dic = registrar_query_result[i][2].isnull() ? Nullable<std::string>()
                : Nullable<std::string>(static_cast<std::string>(registrar_query_result[i][2]));//r.dic
            info_registrar_output.info_registrar_data.variable_symbol = registrar_query_result[i][3].isnull() ? Nullable<std::string>()
                : Nullable<std::string>(static_cast<std::string>(registrar_query_result[i][3]));//r.varsymb
            info_registrar_output.info_registrar_data.vat_payer = static_cast<bool>(registrar_query_result[i]["vat_"]);
            info_registrar_output.info_registrar_data.handle = static_cast<std::string>(registrar_query_result[i][5]);//r.handle
            info_registrar_output.info_registrar_data.name = registrar_query_result[i][6].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][6]));
            info_registrar_output.info_registrar_data.organization = registrar_query_result[i][7].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][7]));
            info_registrar_output.info_registrar_data.street1 = registrar_query_result[i][8].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][8]));
            info_registrar_output.info_registrar_data.street2 = registrar_query_result[i][9].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][9]));
            info_registrar_output.info_registrar_data.street3 = registrar_query_result[i][10].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][10]));
            info_registrar_output.info_registrar_data.city = registrar_query_result[i][11].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][11]));
            info_registrar_output.info_registrar_data.stateorprovince = registrar_query_result[i][12].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][12]));
            info_registrar_output.info_registrar_data.postalcode = registrar_query_result[i][13].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][13]));
            info_registrar_output.info_registrar_data.country = registrar_query_result[i][14].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][14]));
            info_registrar_output.info_registrar_data.telephone = registrar_query_result[i][15].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][15]));
            info_registrar_output.info_registrar_data.fax = registrar_query_result[i][16].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][16]));
            info_registrar_output.info_registrar_data.email = registrar_query_result[i][17].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][17]));
            info_registrar_output.info_registrar_data.url = registrar_query_result[i][18].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][18]));
            info_registrar_output.info_registrar_data.system = registrar_query_result[i][19].isnull() ? Nullable<bool>()
                : Nullable<bool> (static_cast<bool>(registrar_query_result[i][19]));
            info_registrar_output.info_registrar_data.payment_memo_regex = registrar_query_result[i][20].isnull() ? Nullable<std::string>()
                : Nullable<std::string> (static_cast<std::string>(registrar_query_result[i][20]));
            info_registrar_output.utc_timestamp = registrar_query_result[i][21].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(registrar_query_result[i][21]));// utc timestamp
            info_registrar_output.local_timestamp = registrar_query_result[i][22].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(registrar_query_result[i][22]));//local zone timestamp

            result.push_back(info_registrar_output);
        }//for res

        return result;
    }

}//namespace Fred

