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

#ifndef INFO_DOMAIN_IMPL_HH_79C22AA95B8C49119F0ED08FBB5158F5
#define INFO_DOMAIN_IMPL_HH_79C22AA95B8C49119F0ED08FBB5158F5

#include <algorithm>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"
#include "src/util/printable.hh"
#include "src/util/db/param_query_composition.hh"
#include "src/libfred/registrable_object/domain/info_domain_output.hh"

namespace LibFred
{
    /**
    * Domain info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
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
         * Domain info query projection aliases.
         * Set of constants for building inline view filter expressions.
         */
        struct GetAlias
        {
            static const char* id(){return "info_domain_id";}
            static const char* roid(){return "info_domain_roid";}
            static const char* fqdn(){return "info_domain_fqdn";}
            static const char* delete_time(){return "info_domain_delete_time";}
            static const char* historyid(){return "info_domain_historyid";}
            static const char* next_historyid(){return "info_domain_next_historyid";}
            static const char* history_valid_from(){return "info_domain_history_valid_from";}
            static const char* history_valid_to(){return "info_domain_history_valid_to";}
            static const char* registrant_id(){return "info_domain_registrant_id";}
            static const char* registrant_handle(){return "info_domain_registrant_handle";}
            static const char* nsset_id(){return "info_domain_nsset_id";}
            static const char* nsset_handle(){return "info_domain_nsset_handle";}
            static const char* keyset_id(){return "info_domain_keyset_id";}
            static const char* keyset_handle(){return "info_domain_keyset_handle";}
            static const char* sponsoring_registrar_id(){return "info_domain_sponsoring_registrar_id";}
            static const char* sponsoring_registrar_handle(){return "info_domain_sponsoring_registrar_handle";}
            static const char* creating_registrar_id(){return "info_domain_creating_registrar_id";}
            static const char* creating_registrar_handle(){return "info_domain_creating_registrar_handle";}
            static const char* last_updated_by_registrar_id(){return "info_domain_last_updated_by_registrar_id";}
            static const char* last_updated_by_registrar_handle(){return "info_domain_last_updated_by_registrar_handle";}
            static const char* creation_time(){return "info_domain_creation_time";}
            static const char* transfer_time(){return "info_domain_transfer_time";}
            static const char* update_time(){return "info_domain_update_time";}
            static const char* expiration_date(){return "info_domain_expiration_date";}
            static const char* authinfopw(){return "info_domain_authinfopw";}
            static const char* enum_validation_expiration(){return "info_domain_enum_validation_expiration";}
            static const char* enum_publish(){return "info_domain_enum_publish";}
            static const char* first_historyid(){return "info_domain_first_historyid";}
            static const char* logd_request_id(){return "info_domain_logd_request_id";}
            static const char* utc_timestamp(){return "info_domain_utc_timestamp";}
            static const char* local_timestamp(){return "info_domain_local_timestamp";}
            static const char* is_enum(){return "info_domain_is_enum";}
            static const char* zone_id(){return "info_domain_zone_id";}
            static const char* zone_fqdn(){return "info_domain_zone_fqdn";}
        };

        /**
         * Sets domain selection criteria.
         * Filter expression, which is optional WHERE clause, has access to @ref GetAlias info domain projection aliases.
         * Simple usage example: .set_inline_view_filter(Database::ParamQuery(InfoDomain::GetAlias::id())(" = ").param_bigint(id_))
         */
        InfoDomain& set_inline_view_filter(const Database::ParamQuery& filter_expr);

        /**
         * Sets CTE query, that returns set of domain id.
         */
        InfoDomain& set_cte_id_filter(const Database::ParamQuery& filter_expr);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history_query_ attribute
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
        std::vector<InfoDomainOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");

    };

} // namespace LibFred

#endif
