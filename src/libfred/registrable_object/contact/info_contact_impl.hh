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

#ifndef INFO_CONTACT_IMPL_H_
#define INFO_CONTACT_IMPL_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "src/libfred/opexception.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"
#include "src/util/printable.hh"
#include "src/util/db/param_query_composition.hh"
#include "src/libfred/registrable_object/contact/info_contact_output.hh"

namespace LibFred
{
    /**
    * Contact info implementation.
    * It's executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    */
    class InfoContact
    {
        bool history_query_;/**< flag to query history records of the contact */
        bool lock_;/**< if set to true lock object_registry row for update, if set to false lock for share */
        Optional<Database::ParamQuery> info_contact_inline_view_filter_expr_;/**< where clause of the info contact query where projection is inline view sub-select */
        Optional<Database::ParamQuery> info_contact_id_filter_cte_;/**< CTE query returning set of contact id */

        Database::ParamQuery make_query(const std::string& local_timestamp_pg_time_zone_name);/**< info query generator @return query string with query params*/

    public:
        /**
         * Default constructor.
         * Sets @ref history_query_ and @ref lock_ to false
         */
        InfoContact();

        /**
         * Contact info query projection aliases.
         * Set of constants for building inline view filter expressions.
         */
        struct GetAlias
        {
            static const char* id(){return "info_contact_id";}
            static const char* roid(){return "info_contact_roid";}
            static const char* handle(){return "info_contact_handle";}
            static const char* delete_time(){return "info_contact_delete_time";}
            static const char* historyid(){return "info_contact_historyid";}
            static const char* next_historyid(){return "info_contact_next_historyid";}
            static const char* history_valid_from(){return "info_contact_history_valid_from";}
            static const char* history_valid_to(){return "info_contact_history_valid_to";}
            static const char* sponsoring_registrar_id(){return "info_contact_sponsoring_registrar_id";}
            static const char* sponsoring_registrar_handle(){return "info_contact_sponsoring_registrar_handle";}
            static const char* creating_registrar_id(){return "info_contact_creating_registrar_id";}
            static const char* creating_registrar_handle(){return "info_contact_creating_registrar_handle";}
            static const char* last_updated_by_registrar_id(){return "info_contact_last_updated_by_registrar_id";}
            static const char* last_updated_by_registrar_handle(){return "info_contact_last_updated_by_registrar_handle";}
            static const char* creation_time(){return "info_contact_creation_time";}
            static const char* transfer_time(){return "info_contact_transfer_time";}
            static const char* update_time(){return "info_contact_update_time";}
            static const char* authinfopw(){return "info_contact_authinfopw";}
            static const char* first_historyid(){return "info_contact_first_historyid";}
            static const char* logd_request_id(){return "info_contact_logd_request_id";}
            static const char* name(){return "info_contact_name";}
            static const char* organization(){return "info_contact_organization";}
            static const char* street1(){return "info_contact_street1";}
            static const char* street2(){return "info_contact_street2";}
            static const char* street3(){return "info_contact_street3";}
            static const char* city(){return "info_contact_city";}
            static const char* stateorprovince(){return "info_contact_stateorprovince";}
            static const char* postalcode(){return "info_contact_postalcode";}
            static const char* country(){return "info_contact_country";}
            static const char* telephone(){return "info_contact_telephone";}
            static const char* fax(){return "info_contact_fax";}
            static const char* email(){return "info_contact_email";}
            static const char* notifyemail(){return "info_contact_notifyemail";}
            static const char* vat(){return "info_contact_vat";}
            static const char* ssn(){return "info_contact_ssn";}
            static const char* ssntype(){return "info_contact_ssntype";}
            static const char* disclosename(){return "info_contact_disclosename";}
            static const char* discloseorganization(){return "info_contact_discloseorganization";}
            static const char* discloseaddress(){return "info_contact_discloseaddress";}
            static const char* disclosetelephone(){return "info_contact_disclosetelephone";}
            static const char* disclosefax(){return "info_contact_disclosefax";}
            static const char* discloseemail(){return "info_contact_discloseemail";}
            static const char* disclosevat(){return "info_contact_disclosevat";}
            static const char* discloseident(){return "info_contact_discloseident";}
            static const char* disclosenotifyemail(){return "info_contact_disclosenotifyemail";}
            static const char* utc_timestamp(){return "info_contact_utc_timestamp";}
            static const char* local_timestamp(){return "info_contact_local_timestamp";}
            static const char* domain_expiration_letter_preference(){return "info_contact_domain_expiration_letter_preference";}
        };

        /**
         * Sets contact selection criteria.
         * Filter expression, which is optional WHERE clause, has access to @ref GetAlias info contact projection aliases.
         * Simple usage example: .set_inline_view_filter(Database::ParamQuery(InfoContact::GetAlias::id())(" = ").param_bigint(id_))
         */
        InfoContact& set_inline_view_filter(const Database::ParamQuery& filter_expr);

        /**
         * Sets CTE query, that returns set of contact id.
         */
        InfoContact& set_cte_id_filter(const Database::ParamQuery& filter_expr);

        /**
        * Sets history query flag.
        * @param history_query sets history query flag into @ref history query_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_history_query(bool history_query);

        /**
        * Sets lock for update.
        * Default, if not set, is lock for share.
        * Sets true to lock flag in @ref lock_ attribute
        * @return operation instance reference to allow method chaining
        */
        InfoContact& set_lock();

        /**
        * Executes getting info about the contact.
        * @param ctx contains reference to database and logging interface
        * @param local_timestamp_pg_time_zone_name is postgresql time zone name of the returned data
        * @return info data about the contact
        */
        std::vector<InfoContactOutput> exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name = "UTC");

    };

} // namespace LibFred

#endif//INFO_CONTACT_IMPL_H_
