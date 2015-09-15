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
 *  keyset info implementation
 */

#ifndef INFO_KEYSET_IMPL_H_
#define INFO_KEYSET_IMPL_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/db/param_query_composition.h"
#include "info_keyset_output.h"
#include "util/printable.h"
namespace Fred
{
    /**
    * Keyset info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoKeyset
    {

        bool history_query_;/**< flag to query history records of the keyset */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<Database::ParamQuery> info_keyset_inline_view_filter_expr_;/**< where clause of the info keyset query where projection is inline view sub-select */
        Optional<Database::ParamQuery> info_keyset_id_filter_cte_;/**< CTE query returning set of keyset id */

        Database::ParamQuery make_info_keyset_projection_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator*/

        Database::ParamQuery make_tech_contact_query(unsigned long long id, unsigned long long historyid);/**< keyset technical contacts query generator @return pair of query string with query params*/
        Database::ParamQuery make_dns_keys_query(unsigned long long id, unsigned long long historyid);/**< keyset DNS keys query generator @return pair of query string with query params*/
    public:

        /**
        * Default constructor.
        * Sets @ref history_query_ and @ref lock_ to false
        */
        InfoKeyset();

        /**
         * Keyset info query projection aliases.
         * Set of constants for building inline view filter expressions.
         */
        struct GetAlias
        {
            static const char* id(){return "info_keyset_id";}
            static const char* roid(){return "info_keyset_roid";}
            static const char* handle(){return "info_keyset_handle";}
            static const char* delete_time(){return "info_keyset_delete_time";}
            static const char* historyid(){return "info_keyset_historyid";}
            static const char* next_historyid(){return "info_keyset_next_historyid";}
            static const char* history_valid_from(){return "info_keyset_history_valid_from";}
            static const char* history_valid_to(){return "info_keyset_history_valid_to";}
            static const char* sponsoring_registrar_id(){return "info_keyset_sponsoring_registrar_id";}
            static const char* sponsoring_registrar_handle(){return "info_keyset_sponsoring_registrar_handle";}
            static const char* creating_registrar_id(){return "info_keyset_creating_registrar_id";}
            static const char* creating_registrar_handle(){return "info_keyset_creating_registrar_handle";}
            static const char* last_updated_by_registrar_id(){return "info_keyset_last_updated_by_registrar_id";}
            static const char* last_updated_by_registrar_handle(){return "info_keyset_last_updated_by_registrar_handle";}
            static const char* creation_time(){return "info_keyset_creation_time";}
            static const char* transfer_time(){return "info_keyset_transfer_time";}
            static const char* update_time(){return "info_keyset_update_time";}
            static const char* authinfopw(){return "info_keyset_authinfopw";}
            static const char* first_historyid(){return "info_keyset_first_historyid";}
            static const char* logd_request_id(){return "info_keyset_logd_request_id";}
            static const char* utc_timestamp(){return "info_keyset_utc_timestamp";}
            static const char* local_timestamp(){return "info_keyset_local_timestamp";}
        };

        /**
         * Sets keyset selection criteria.
         * Filter expression, which is optional WHERE clause, has access to @ref GetAlias info keyset projection aliases.
         * Simple usage example: .set_inline_view_filter(Database::ParamQuery(InfoKeyset::GetAlias::id())(" = ").param_bigint(id_))
         */
        InfoKeyset& set_inline_view_filter(const Database::ParamQuery& filter_expr);

        /**
         * Sets CTE query, that returns set of keyset id.
         */
        InfoKeyset& set_cte_id_filter(const Database::ParamQuery& filter_expr);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoKeyset& set_history_query(bool history_query);


        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoKeyset& set_lock();

        /**
        * Executes getting info about the keyset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the keyset descendingly ordered by keyset historyid
        */
        std::vector<InfoKeysetOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

    };

}//namespace Fred

#endif//INFO_KEYSET_IMPL_H_
