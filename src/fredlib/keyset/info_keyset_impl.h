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
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
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
         * Sets keyset selection criteria.
         * Filter expression, which is optional WHERE clause, has access to following info keyset projection aliases:
         * info_keyset_id, info_keyset_roid, info_keyset_handle, info_keyset_delete_time, info_keyset_historyid,
         * info_keyset_next_historyid, info_keyset_history_valid_from, info_keyset_history_valid_to,
         * info_keyset_sponsoring_registrar_id, info_keyset_sponsoring_registrar_handle, info_keyset_creating_registrar_id,
         * info_keyset_creating_registrar_handle, info_keyset_last_updated_by_registrar_id, info_keyset_last_updated_by_registrar_handle,
         * info_keyset_creation_time, info_keyset_transfer_time, info_keyset_update_time, info_keyset_authinfopw,
         * info_keyset_first_historyid, info_keyset_logd_request_id, info_keyset_utc_timestamp, info_keyset_local_timestamp
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
        * @return info data about the keyset
        */
        std::vector<InfoKeysetOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

        /**
        * Executes explain analyze and getting info about the keyset for testing purposes.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @param result info data about the keyset
        * @return query and plan
        */
        std::string explain_analyze(OperationContext& ctx, std::vector<InfoKeysetOutput>& result, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return query plan
    };

}//namespace Fred

#endif//INFO_KEYSET_IMPL_H_
