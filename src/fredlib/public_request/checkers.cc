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

#include "src/fredlib/public_request/checkers.h"
#include "src/fredlib/contact_verification/django_email_format.h"
#include "util/idn_utils.h"

#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Fred {
namespace PublicRequest {

namespace {

bool nothing_else_whitespaces(const std::string &_data)
{
    return boost::algorithm::all(_data, boost::algorithm::is_space());
}

bool absent_or_empty(const Nullable< std::string > &_data)
{
    return _data.isnull() || nothing_else_whitespaces(_data.get_value());
}

bool absents_or_matches_pattern(const std::string &_str, const boost::regex &_pattern)
{
    return nothing_else_whitespaces(_str) ||
           boost::regex_match(_str, _pattern);
}

bool absents_or_matches_pattern(const Nullable< std::string > &_str, const boost::regex &_pattern)
{
    return _str.isnull() ||
           absents_or_matches_pattern(_str.get_value(), _pattern);
}

}//Fred::PublicRequest::{anonymous}

check_contact_name::check_contact_name(const Contact::Verification::Contact &_data)
{
    const std::string name = boost::algorithm::trim_copy(_data.name);
    first_name_absents = name.empty();
    last_name_absents = name.find_last_of(' ') == std::string::npos;
}

check_contact_mailing_address::check_contact_mailing_address(const Contact::Verification::Contact &_data)
{
    street1_absents = absent_or_empty(_data.street1);
    city_absents = absent_or_empty(_data.city);
    postalcode_absents = absent_or_empty(_data.postalcode);
    country_absents = absent_or_empty(_data.country);
}

check_contact_email_presence::check_contact_email_presence(const Contact::Verification::Contact &_data)
:   absents(absent_or_empty(_data.email))
{
}

check_contact_email_validity::check_contact_email_validity(const Contact::Verification::Contact &_data)
{
    enum { MAX_MOJEID_EMAIL_LENGTH = 200 };
    const std::string email = _data.email.get_value_or_default();

    invalid = email.empty() ||
              (MAX_MOJEID_EMAIL_LENGTH < Util::get_utf8_char_len(email)) ||
              DjangoEmailFormat().check(email);
}

check_contact_email_availability::check_contact_email_availability(
    const Contact::Verification::Contact &_data,
    OperationContext &_ctx)
:   check_contact_email_presence(_data)
{
    if (!this->check_contact_email_presence::success()) {
        used_recently = false;
        return;
    }
    const Database::Result ucheck = _ctx.get_conn().exec_params(
        "SELECT EXISTS("
            "SELECT 1 "
            "FROM contact_history ch "
            "JOIN object_state os ON os.ohid_from=ch.historyid "
            "JOIN enum_object_states eos ON eos.id=os.state_id "
            "WHERE eos.name='conditionallyIdentifiedContact' AND "
                  "(NOW()-$1::INTERVAL)<os.valid_from AND "
                  "TRIM(LOWER(ch.email))=TRIM(LOWER($2::TEXT)) AND "
                  "ch.id!=$3::BIGINT) AS used_recently",
        Database::query_param_list(email_phone_protection_period())
                                  (_data.email.get_value_or_default())
                                  (_data.id));
    used_recently = static_cast< bool >(ucheck[0][0]);
}

check_contact_phone_presence::check_contact_phone_presence(const Contact::Verification::Contact &_data)
:   absents(absent_or_empty(_data.telephone))
{
}

check_contact_phone_validity::check_contact_phone_validity(const Contact::Verification::Contact &_data)
:   invalid(!absents_or_matches_pattern(_data.telephone, phone_pattern()))
{
}

check_contact_phone_availability::check_contact_phone_availability(
    const Contact::Verification::Contact &_data,
    OperationContext &_ctx)
:   check_contact_phone_presence(_data)
{
    if (!this->check_contact_phone_presence::success()) {
        used_recently = false;
        return;
    }
    const Database::Result ucheck = _ctx.get_conn().exec_params(
        "SELECT EXISTS("
            "SELECT 1 "
            "FROM contact_history ch "
            "JOIN object_state os ON os.ohid_from=ch.historyid "
            "JOIN enum_object_states eos ON eos.id=os.state_id "
            "WHERE eos.name='conditionallyIdentifiedContact' AND "
                  "(NOW()-$1::INTERVAL)<os.valid_from AND "
                  "TRIM(LOWER(ch.telephone))=TRIM(LOWER($2::TEXT)) AND "
                  "ch.id!=$3::BIGINT) AS used_recently",
        Database::query_param_list(email_phone_protection_period())
                                  (_data.telephone.get_value_or_default())
                                  (_data.id));
    used_recently = static_cast< bool >(ucheck[0][0]);
}

check_contact_fax_validity::check_contact_fax_validity(const Contact::Verification::Contact &_data)
:   invalid(!absents_or_matches_pattern(_data.fax, phone_pattern()))
{
}

namespace MojeID {

check_contact_username::check_contact_username(const Contact::Verification::Contact &_data)
:   absents(nothing_else_whitespaces(_data.handle)),
    invalid(!absents_or_matches_pattern(_data.handle, username_pattern()))
{
}

}//Fred::PublicRequest::MojeID

}//Fred::PublicRequest
}//Fred
