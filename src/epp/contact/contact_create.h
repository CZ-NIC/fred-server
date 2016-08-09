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

#ifndef EPP_CONTACT_CREATE_H_548537357434
#define EPP_CONTACT_CREATE_H_548537357434

#include "src/epp/contact/disclose.h"
#include "src/epp/contact/ident_type.h"
#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <set>
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct ContactCreateInputData
{
    template < ContactDisclose::Enum ITEM >
    bool compute_disclose_flag(bool default_policy_is_to_disclose)const
    {
        const bool disclose_preference_is_set = !to_hide.empty() || !to_disclose.empty();
        if (!disclose_preference_is_set) {
            const bool item_should_be_disclosed = default_policy_is_to_disclose;
            return item_should_be_disclosed;
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

    std::string name;
    std::string organization;
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string state_or_province;
    std::string postal_code;
    std::string country_code;
    std::string telephone;
    std::string fax;
    std::string email;
    std::string notify_email;
    std::string VAT;
    std::string ident;
    Nullable< IdentType::Enum > identtype;
    std::string authinfo;
    std::set< ContactDisclose::Enum > to_hide;
    std::set< ContactDisclose::Enum > to_disclose;
};

struct LocalizedCreateContactResponse {
    const LocalizedSuccessResponse ok_response;
    const boost::posix_time::ptime crdate;

    LocalizedCreateContactResponse(
        const LocalizedSuccessResponse& _ok_response,
        const boost::posix_time::ptime& _crdate
    ) :
        ok_response(_ok_response),
        crdate(_crdate)
    { }
};

LocalizedCreateContactResponse contact_create(
    const std::string &_contact_handle,
    const ContactCreateInputData &_data,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const std::string &_dont_notify_client_transaction_handles_with_this_prefix);

}

#endif
