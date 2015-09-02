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
 *  domain info implementation
 */

#ifndef INFO_DOMAIN_IMPL_H_
#define INFO_DOMAIN_IMPL_H_

#include <algorithm>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/printable.h"
#include "util/db/param_query_composition.h"
#include "info_domain_output.h"

namespace Fred
{
    /**
    * Domain info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    */
    class InfoDomain
    {

        bool history_query_;/**< flag to query history records of the domain */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<Database::ParamQuery> info_domain_inline_view_filter_expr_;/**< where clause of the info domain query where projection is inline view sub-select */
        Optional<Database::ParamQuery> info_domain_id_filter_cte_;/**< CTE query returning set of domain id */

        Database::ParamQuery make_domain_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator @return pair of query string with query params*/
        Database::ParamQuery make_admin_query(unsigned long long id, unsigned long long historyid);/**< info query generator @return pair of query string with query params*/
    public:

        /**
        * Default constructor.
        * Sets @ref history_query_ and @ref lock_ to false
        */
        InfoDomain();

        /**
         * Sets domain selection criteria.
         * Filter expression, which is optional WHERE clause, has access to following info domain projection aliases:
         * info_domain_id, info_domain_roid, info_domain_fqdn, info_domain_delete_time,
         * info_domain_historyid, info_domain_next_historyid, info_domain_history_valid_from, info_domain_history_valid_to,
         * info_domain_registrant_id, info_domain_registrant_handle, info_domain_nsset_id, info_domain_nsset_handle,
         * info_domain_keyset_id, info_domain_keyset_handle, info_domain_sponsoring_registrar_id, info_domain_sponsoring_registrar_handle,
         * info_domain_creating_registrar_id, info_domain_creating_registrar_handle, info_domain_last_updated_by_registrar_id,
         * info_domain_last_updated_by_registrar_handle, info_domain_creation_time, info_domain_transfer_time, info_domain_update_time,
         * info_domain_expiration_date, info_domain_authinfopw, info_domain_enum_validation_expiration, info_domain_enum_publish,
         * info_domain_first_historyid, info_domain_logd_request_id,
         * info_domain_utc_timestamp, info_domain_local_timestamp, info_domain_is_enum, info_domain_zone_id, info_domain_zone_fqdn
         */
        InfoDomain& set_inline_view_filter(const Database::ParamQuery& filter_expr);

        /**
         * Sets CTE query, that returns set of domain id.
         */
        InfoDomain& set_cte_id_filter(const Database::ParamQuery& filter_expr);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoDomain& set_history_query(bool history_query);

        /**
         * Sets lock for update.
         * Default, if not set, is lock for share.
         * Sets true to lock flag in @ref lock_ attribute
         * @return operation instance reference to allow method chaining
         */
        InfoDomain& set_lock();

        /**
        * Executes getting info about the domain.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the domain descendingly ordered by domain historyid
        */
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");//return data

        /**
        * Executes explain analyze and getting info about the domain for testing purposes.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data and history_timestamp
        * @param result info data about the domain
        * @return query and plan
        */
        std::string explain_analyze(OperationContext& ctx, std::vector<InfoDomainOutput>& result, const std::string& local_timestamp_pg_time_zone_name = "Europe/Prague");//return query plan
    };

}//namespace Fred

#endif//INFO_DOMAIN_IMPL_H_
