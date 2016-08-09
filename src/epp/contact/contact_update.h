/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 */

#ifndef EPP_CONTACT_UPDATE_H_456451554674
#define EPP_CONTACT_UPDATE_H_456451554674

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/contact/disclose.h"
#include "src/epp/contact/ident_type.h"

#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>
#include <set>

namespace Epp {
/**
 * members of type Nullable<Optional<std::string> > means:
 * * if Isnull() - change is delete (set to empty string)
 * * if NOT Isnull() and Isset() - change to string content
 * * if NOT Isnull() NOT Isset() - no change at all
 */
// TODO XXX later string members should be more idiot proof
// TODO XXX later explore relations between ident and identtype data
// TODO XXX handle snad neni vymazatelny
// TODO XXX authinfo snad neni vymazatelne
struct ContactUpdateInputData
{
    bool should_discloseflags_be_changed()const
    {
        return !to_hide.empty() || !to_disclose.empty();
    }
    template < ContactDisclose::Enum ITEM >
    bool compute_disclose_flag(bool default_policy_is_to_disclose)const
    {
        if (!this->should_discloseflags_be_changed()) {
            throw std::runtime_error("Don't touch discloseflags.");
        }
        const bool disclose_preference_is_to_hide = to_hide.find(ITEM) != to_hide.end();
        const bool disclose_preference_is_to_disclose = to_disclose.find(ITEM) != to_disclose.end();
        if (disclose_preference_is_to_hide != disclose_preference_is_to_disclose) {
            const bool item_should_be_disclosed = disclose_preference_is_to_disclose;
            return item_should_be_disclosed;
        }
        if (!disclose_preference_is_to_hide && !disclose_preference_is_to_disclose) {
            const bool item_should_be_disclosed = default_policy_is_to_disclose;
            return item_should_be_disclosed;
        }
        throw std::runtime_error("The same entry can't be simultaneously hidden and disclosed.");
    }

    Optional<std::string> name;
    Optional<std::string> organization;
    Optional<std::string> street1;
    Optional<std::string> street2;
    Optional<std::string> street3;
    Optional<std::string> city;
    Optional<std::string> state_or_province;
    Optional<std::string> postal_code;
    Optional<std::string> country_code;
    Optional<std::string> telephone;
    Optional<std::string> fax;
    Optional<std::string> email;
    Optional<std::string> notify_email;
    Optional<std::string> VAT;
    Optional<std::string> ident;
    Nullable<IdentType::Enum> identtype;
    Optional<std::string> authinfo;
    std::set< ContactDisclose::Enum > to_hide;
    std::set< ContactDisclose::Enum > to_disclose;
};

LocalizedSuccessResponse contact_update(
    const std::string &_contact_handle,
    const ContactUpdateInputData &_data,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    bool _epp_update_contact_enqueue_check,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const std::string &_client_transaction_handles_prefix_not_to_nofify);

}

#endif
