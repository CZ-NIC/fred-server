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

#include "src/fredlib/contact/checkers.h"
#include "src/fredlib/contact_verification/django_email_format.h"
#include "util/idn_utils.h"
#include "util/types/birthdate.h"

#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Fred {

namespace {

bool nothing_else_whitespaces(const std::string &_data)
{
    return boost::algorithm::all(_data, boost::algorithm::is_space());
}

bool absent_or_empty(const Nullable< std::string > &_data)
{
    return _data.isnull() || nothing_else_whitespaces(_data.get_value());
}

bool match(const std::string &_str, const boost::regex &_pattern)
{
    return boost::regex_match(_str, _pattern);
}

bool absent_or_match(const std::string &_str, const boost::regex &_pattern)
{
    return nothing_else_whitespaces(_str) || match(_str, _pattern);
}

bool absent_or_match(const Nullable< std::string > &_str, const boost::regex &_pattern)
{
    return _str.isnull() || absent_or_match(_str.get_value(), _pattern);
}

bool email_absent_or_valid(const Nullable< std::string > &_email)
{
    enum { MAX_MOJEID_EMAIL_LENGTH = 200 };
    const std::string email = boost::algorithm::trim_copy(_email.get_value_or_default());

    return email.empty() ||
           ((Util::get_utf8_char_len(email) <= MAX_MOJEID_EMAIL_LENGTH) && DjangoEmailFormat().check(email));
}

}//Fred::{anonymous}

namespace GeneralCheck
{

contact_name::contact_name(const Nullable< std::string > &_name)
{
    const std::string name = boost::algorithm::trim_copy(_name.get_value_or_default());
    first_name_absent = name.empty();
    last_name_absent = name.find_last_of(' ') == std::string::npos;
}

contact_mailing_address::contact_mailing_address(
    const std::string &_street1,
    const std::string &_city,
    const std::string &_postalcode,
    const std::string &_country)
:   street1_absent(nothing_else_whitespaces(_street1)),
    city_absent(nothing_else_whitespaces(_city)),
    postalcode_absent(nothing_else_whitespaces(_postalcode)),
    country_absent(nothing_else_whitespaces(_country))
{ }

contact_email_presence::contact_email_presence(const Nullable< std::string > &_email)
:   absent(absent_or_empty(_email))
{ }

contact_email_validity::contact_email_validity(const Nullable< std::string > &_email)
:   invalid(!email_absent_or_valid(_email))
{ }

contact_email_availability::contact_email_availability(
    const Nullable< std::string > &_email,
    unsigned long long _id,
    OperationContext &_ctx)
:   absent(contact_email_presence(_email).absent),
    used_recently(!absent)
{
    if (absent) {
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
                                  (_email.get_value_or_default())
                                  (_id));
    used_recently = static_cast< bool >(ucheck[0][0]);
}

contact_notifyemail_validity::contact_notifyemail_validity(const Nullable< std::string > &_notifyemail)
:   invalid(!email_absent_or_valid(_notifyemail))
{ }

}//Fred::GeneralCheck

check_contact_phone_presence::check_contact_phone_presence(const InfoContactData &_data)
:   absent(absent_or_empty(_data.telephone))
{
}

check_contact_phone_validity::check_contact_phone_validity(const InfoContactData &_data)
:   invalid(!absent_or_match(_data.telephone, phone_pattern()))
{
}

check_contact_phone_availability::check_contact_phone_availability(
    const InfoContactData &_data,
    OperationContext &_ctx)
:   absent(check_contact_phone_presence(_data).absent),
    used_recently(!absent)
{
    if (absent) {
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

check_contact_fax_validity::check_contact_fax_validity(const InfoContactData &_data)
:   invalid(!absent_or_match(_data.fax, phone_pattern()))
{
}

namespace MojeID {

check_contact_username::check_contact_username(const InfoContactData &_data)
:   absent(nothing_else_whitespaces(_data.handle))
{
    if (absent) {
        invalid = false;
    }
    else {
        invalid = (USERNAME_LENGTH_LIMIT < _data.handle.length()) ||
                  !match(_data.handle, username_pattern());
    }
}

namespace {
const char *const ssntype_birthday = "BIRTHDAY";
const char *const ssntype_vat_id = "ICO";
bool ssntype_present(const InfoContactData &_data, const char *_ssntype)
{
    return !_data.ssntype.isnull() && (_data.ssntype.get_value() == _ssntype);
}
}

check_contact_birthday::check_contact_birthday(const InfoContactData &_data)
:   absent(!ssntype_present(_data, ssntype_birthday)),
    invalid(!check_contact_birthday_validity(_data).success())
{
}

check_contact_birthday_validity::check_contact_birthday_validity(const InfoContactData &_data)
try:invalid(ssntype_present(_data, ssntype_birthday) &&
            birthdate_from_string_to_date(_data.ssn.get_value()).is_special())
{
}
catch (...) {
    invalid = true;
}

check_contact_vat_id_presence::check_contact_vat_id_presence(const InfoContactData &_data)
:   absent(!ssntype_present(_data, ssntype_vat_id) || absent_or_empty(_data.ssn))
{
}

}//Fred::MojeID
}//Fred
