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
 *  nsset info implementation
 */

#ifndef INFO_NSSET_IMPL_H_
#define INFO_NSSET_IMPL_H_

#include <string>
#include <vector>
#include <utility>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/printable.h"
#include "util/db/param_query_composition.h"
#include "info_nsset_output.h"

namespace Fred
{
    /**
    * Nsset info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    */
    class InfoNsset
    {
        bool history_query_;/**< flag to query history records of the nsset */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<Database::ParamQuery> info_nsset_inline_view_filter_expr_;/**< where clause of the info nsset query where projection is inline view sub-select */
        Optional<Database::ParamQuery> info_nsset_id_filter_cte_;/**< CTE query returning set of nsset id */

        Database::ParamQuery make_info_nsset_projection_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator*/

        Database::ParamQuery make_tech_contact_query(unsigned long long id, unsigned long long historyid);/**< nsset technical contacts query generator @return pair of query string with query params*/
        Database::ParamQuery make_dns_host_query(unsigned long long nssetid, unsigned long long historyid);/**< nsset DNS hosts query generator @return pair of query string with query params*/
        Database::ParamQuery make_dns_ip_query(unsigned long long hostid, unsigned long long historyid);/**< nsset DNS hosts query generator @return pair of query string with query params*/
    public:

        /**
        * Default constructor.
        * Sets @ref history_query_ and @ref lock_ to false
        */
        InfoNsset();

        /**
         * Sets nsset selection criteria.
         * Filter expression, which is optional WHERE clause, has access to following info nsset projection aliases:
         * info_nsset_id, info_nsset_roid, info_nsset_handle, info_nsset_delete_time, info_nsset_historyid,
         * info_nsset_next_historyid, info_nsset_history_valid_from, info_nsset_history_valid_to,
         * info_nsset_sponsoring_registrar_id, info_nsset_sponsoring_registrar_handle, info_nsset_creating_registrar_id,
         * info_nsset_creating_registrar_handle, info_nsset_last_updated_by_registrar_id, info_nsset_last_updated_by_registrar_handle,
         * info_nsset_creation_time, info_nsset_transfer_time, info_nsset_update_time, info_nsset_tech_check_level,
         * info_nsset_authinfopw, info_nsset_first_historyid, info_nsset_logd_request_id, info_nsset_utc_timestamp, info_nsset_local_timestamp
         */
        InfoNsset& set_inline_view_filter(const Database::ParamQuery& filter_expr);

        /**
         * Sets CTE query, that returns set of nsset id.
         */
        InfoNsset& set_cte_id_filter(const Database::ParamQuery& filter_expr);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_history_query(bool history_query);


        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoNsset& set_lock();

        /**
        * Executes getting info about the nsset.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the nsset descendingly ordered by nsset historyid
        */
        std::vector<InfoNssetOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

    };

}//namespace Fred

#endif//INFO_NSSET_IMPL_H_
