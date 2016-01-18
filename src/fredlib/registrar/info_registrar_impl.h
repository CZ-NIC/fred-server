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

#ifndef INFO_REGISTRAR_IMPL_H_
#define INFO_REGISTRAR_IMPL_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"
#include "util/db/param_query_composition.h"
#include "info_registrar_output.h"

namespace Fred
{
    /**
    * Registrar info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoRegistrar
    {
        bool lock_;/**< lock row for registrar */
        Optional<Database::ParamQuery> info_registrar_inline_view_filter_expr_;/**< where clause of the info registrar query where projection is inline view sub-select */
        Optional<Database::ParamQuery> info_registrar_id_filter_cte_;/**< CTE query returning set of registrar id */

        Database::ParamQuery make_registrar_query(const std::string& local_timestamp_pg_time_zone_name);/**< registrar query generator @return pair of query string with query params*/

    public:
        /**
         * Default constructor.
         * Sets @ref lock_ to false
         */
        InfoRegistrar();

        /**
         * Registrar info query projection aliases.
         * Set of constants for building inline view filter expressions.
         */
        struct GetAlias
        {
            static const char* id(){return "info_registrar_id";}
            static const char* ico(){return "info_registrar_ico";}
            static const char* dic(){return "info_registrar_dic";}
            static const char* variable_symbol(){return "info_registrar_variable_symbol";}
            static const char* vat_payer(){return "info_registrar_vat_payer";}
            static const char* handle(){return "info_registrar_handle";}
            static const char* name(){return "info_registrar_name";}
            static const char* organization(){return "info_registrar_organization";}
            static const char* street1(){return "info_registrar_street1";}
            static const char* street2(){return "info_registrar_street2";}
            static const char* street3(){return "info_registrar_street3";}
            static const char* city(){return "info_registrar_city";}
            static const char* stateorprovince(){return "info_registrar_stateorprovince";}
            static const char* postalcode(){return "info_registrar_postalcode";}
            static const char* country(){return "info_registrar_country";}
            static const char* telephone(){return "info_registrar_telephone";}
            static const char* fax(){return "info_registrar_fax";}
            static const char* email(){return "info_registrar_email";}
            static const char* url(){return "info_registrar_url";}
            static const char* system(){return "info_registrar_system";}
            static const char* memo_regex(){return "info_registrar_memo_regex";}
            static const char* utc_timestamp(){return "info_registrar_utc_timestamp";}
            static const char* local_timestamp(){return "info_registrar_local_timestamp";}
        };

        /**
         * Sets registrar selection criteria.
         * Filter expression, which is optional WHERE clause, has access to @ref GetAlias info registrar projection aliases.
         * Simple usage example: .set_inline_view_filter(Database::ParamQuery(InfoRegistrar::GetAlias::id())(" = ").param_bigint(id_))
         */
        InfoRegistrar& set_inline_view_filter(const Database::ParamQuery& filter_expr);

        /**
         * Sets CTE query, that returns set of registrar id.
         */
        InfoRegistrar& set_cte_id_filter(const Database::ParamQuery& filter_expr);


        /**
        * Sets the registrar lock flag.
        * @param lock sets lock registrar flag into @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoRegistrar& set_lock(bool lock = true);

        /**
        * Executes getting info about the registrar.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the registrar
        */
        std::vector<InfoRegistrarOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");

    };//classInfoRegistrar
}//namespace Fred

#endif//INFO_REGISTRAR_IMPL_H_
