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

#include "src/backend/mojeid/checkers.hh"
#include "libfred/contact_verification/django_email_format.hh"
#include "libfred/object/object_type.hh"
#include "src/deprecated/libfred/registrable_object/contact/ssntype.hh"
#include "util/idn_utils.hh"
#include "src/util/types/birthdate.hh"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <cstdlib>
#include <iostream>

namespace Fred {
namespace Backend {

namespace {

const boost::regex phone_pattern("[[:space:]]*\\+[0-9]{1,3}\\.[0-9]{1,14}[[:space:]]*");

bool nothing_else_whitespaces(const std::string& _data)
{
    return boost::algorithm::all(
            _data,
            boost::algorithm::is_space());
}


bool absent_or_empty(const Nullable<std::string>& _data)
{
    return _data.isnull() || nothing_else_whitespaces(_data.get_value());
}


bool match(
        const std::string& _str,
        const boost::regex& _pattern)
{
    return boost::regex_match(_str, _pattern);
}


bool absent_or_match(
        const std::string& _str,
        const boost::regex& _pattern)
{
    return nothing_else_whitespaces(_str)
           ||
           match(_str, _pattern);
}


bool absent_or_match(
        const Nullable<std::string>& _str,
        const boost::regex& _pattern)
{
    return _str.isnull()
           ||
           absent_or_match(
                   _str.get_value(),
                   _pattern);
}


bool email_absent_or_valid(const Nullable<std::string>& _email)
{
    enum
    {
        MAX_MOJEID_EMAIL_LENGTH = 200

    };

    const std::string email = boost::algorithm::trim_copy(_email.get_value_or_default());

    return email.empty() ||
           ((Util::get_utf8_char_len(email) <= MAX_MOJEID_EMAIL_LENGTH) && DjangoEmailFormat().check(email));
}


} // namespace Fred::Backend::{anonymous}

namespace GeneralCheck {


contact_name::contact_name(const Nullable<std::string>& _name)
{
    const std::string name = boost::algorithm::trim_copy(_name.get_value_or_default());
    first_name_absent = name.empty();
    last_name_absent = name.find_last_of(' ') == std::string::npos;
}


contact_name::contact_name(
        const std::string& _first_name,
        const std::string& _last_name)
    : first_name_absent(nothing_else_whitespaces(_first_name)),
      last_name_absent(nothing_else_whitespaces(_last_name))
{
}


contact_optional_address::contact_optional_address(bool _success)
    : street1_absent(!_success),
      city_absent(!_success),
      postalcode_absent(!_success),
      country_absent(!_success)
{
}


contact_optional_address& contact_optional_address::operator()(
        const std::string& _street1,
        const std::string& _city,
        const std::string& _postalcode,
        const std::string& _country)
{
    street1_absent = nothing_else_whitespaces(_street1);
    city_absent = nothing_else_whitespaces(_city);
    postalcode_absent = nothing_else_whitespaces(_postalcode);
    country_absent = nothing_else_whitespaces(_country);
    return *this;
}


contact_address::contact_address(
        const std::string& _street1,
        const std::string& _city,
        const std::string& _postalcode,
        const std::string& _country)
    : contact_optional_address(true)
{
    this->contact_optional_address::operator()(
            _street1,
            _city,
            _postalcode,
            _country);
}


contact_email_presence::contact_email_presence(const Nullable<std::string>& _email)
    : absent(absent_or_empty(_email))
{
}


contact_email_validity::contact_email_validity(const Nullable<std::string>& _email)
    : invalid(!email_absent_or_valid(_email))
{
}


contact_email_availability::contact_email_availability(
        const Nullable<std::string>& _email,
        unsigned long long _id,
        LibFred::OperationContext& _ctx)
    : absent(contact_email_presence(_email).absent),
      used_recently(!absent)
{
    if (absent)
    {
        return;
    }
    const Database::Result ucheck =
        _ctx.get_conn().exec_params(
        // clang-format off
        "SELECT EXISTS("
            "SELECT 1 "
            "FROM contact_history ch "
            "JOIN object_state os ON os.ohid_from=ch.historyid "
            "JOIN enum_object_states eos ON eos.id=os.state_id "
            "WHERE eos.name='conditionallyIdentifiedContact' AND "
                  "(NOW()-$1::INTERVAL)<os.valid_from AND "
                  "TRIM(LOWER(ch.email))=TRIM(LOWER($2::TEXT)) AND "
                  "ch.id!=$3::BIGINT) AS used_recently",
        // clang-format on
        Database::query_param_list(email_phone_protection_period())(_email.get_value_or_default())(
                        _id));
    used_recently = static_cast<bool>(ucheck[0][0]);
}


contact_notifyemail_validity::contact_notifyemail_validity(const Nullable<std::string>& _notifyemail)
    : invalid(!email_absent_or_valid(_notifyemail))
{
}


contact_phone_presence::contact_phone_presence(const Nullable<std::string>& _telephone)
    : absent(absent_or_empty(_telephone))
{
}


contact_phone_validity::contact_phone_validity(const Nullable<std::string>& _telephone)
    : invalid(!absent_or_match(_telephone, phone_pattern))
{
}


contact_phone_availability::contact_phone_availability(
        const Nullable<std::string>& _telephone,
        unsigned long long _id,
        LibFred::OperationContext& _ctx)
    : absent(contact_phone_presence(_telephone).absent),
      used_recently(!absent)
{
    if (absent)
    {
        return;
    }
    const Database::Result ucheck = _ctx.get_conn().exec_params(
        // clang-format off
        "SELECT EXISTS("
            "SELECT 1 "
            "FROM contact_history ch "
            "JOIN object_state os ON os.ohid_from=ch.historyid "
            "JOIN enum_object_states eos ON eos.id=os.state_id "
            "WHERE eos.name='conditionallyIdentifiedContact' AND "
                  "(NOW()-$1::INTERVAL)<os.valid_from AND "
                  "TRIM(LOWER(ch.telephone))=TRIM(LOWER($2::TEXT)) AND "
                  "ch.id!=$3::BIGINT) AS used_recently",
        // clang-format on
        Database::query_param_list(email_phone_protection_period())(_telephone.get_value_or_default())(_id));
    used_recently = static_cast<bool>(ucheck[0][0]);
}


contact_fax_validity::contact_fax_validity(const Nullable<std::string>& _fax)
    : invalid(!absent_or_match(_fax, phone_pattern))
{
}


namespace MojeId {

const boost::regex username_pattern("[0-9A-Za-z](-?[0-9A-Za-z])*");


contact_username::contact_username(const std::string& _handle)
    : absent(nothing_else_whitespaces(_handle))
{
    if (absent)
    {
        invalid = false;
    }
    else
    {
        invalid = (USERNAME_LENGTH_LIMIT < _handle.length()) ||
                  !match(
                _handle,
                username_pattern);
    }
}


contact_username_availability::contact_username_availability(
        const std::string& _handle,
        LibFred::OperationContext& _ctx)
{
    const Database::Result dbres =
        _ctx.get_conn().exec_params(
        // clang-format off
        "SELECT erdate IS NULL AS taken,"
               "(SELECT (NOW()-(val::TEXT||'MONTHS')::INTERVAL)<erdate FROM enum_parameters "
                "WHERE name='handle_registration_protection_period') AS protected "
        "FROM object_registry "
        "WHERE type=get_object_type_id($2::TEXT) AND "
              "UPPER(name)=UPPER($1::TEXT) "//use index (UPPER(name)) WHERE type=1
        "ORDER BY erdate IS NULL DESC,"//prefer erdate=NULL
                 "erdate DESC "        //otherwise the newest erdate
        "LIMIT 1",
        // clang-format on
        Database::query_param_list(_handle)(
                Conversion::Enums::to_db_handle(
                        LibFred::Object_Type::
                        contact)));
    taken         = (0 < dbres.size()) && static_cast<bool>(dbres[0][0]);
    used_recently = (0 < dbres.size()) && static_cast<bool>(dbres[0][1]);
}


namespace {

bool ssntype_present(
        const Nullable<std::string>& _ssntype_current,
        LibFred::SSNType::Enum _ssntype_required)
{
    return !_ssntype_current.isnull() &&
           (_ssntype_current.get_value() == Conversion::Enums::to_db_handle(_ssntype_required));
}


} // Fred::Backend::GeneralCheck::MojeId::{anonymous}


contact_birthday::contact_birthday(
        const Nullable<std::string>& _ssntype,
        const Nullable<std::string>& _ssn)
    : absent(!ssntype_present(_ssntype, LibFred::SSNType::birthday)),
      invalid(!contact_birthday_validity(_ssntype, _ssn).success())
{
}


contact_birthday_validity::contact_birthday_validity(
        const Nullable<std::string>& _ssntype,
        const Nullable<std::string>& _ssn)
{
    try
    {
        invalid = ssntype_present(
                _ssntype,
                LibFred::SSNType::birthday)
                  && birthdate_from_string_to_date(_ssn.get_value()).is_special();
    }
    catch (...)
    {
        invalid = true;
    }
}


contact_vat_id_presence::contact_vat_id_presence(
        const Nullable<std::string>& _ssntype,
        const Nullable<std::string>& _ssn)
    : absent(!ssntype_present(_ssntype, LibFred::SSNType::ico) || absent_or_empty(_ssn))
{
}


} // namespace Fred::GeneralCheck::MojeId
} // namespace Fred::GeneralCheck


check_contact_place_address::check_contact_place_address(const LibFred::InfoContactData& _data)
    : GeneralCheck::contact_optional_address(_data.place.isnull()),
      absent(_data.place.isnull())
{
    if (!absent)
    {
        this->GeneralCheck::contact_optional_address::operator()(
                _data.place.get_value().street1,
                _data.place.get_value().city,
                _data.place.get_value().postalcode,
                _data.place.get_value().country);


    }
}


check_contact_addresses::check_contact_addresses(
        const LibFred::InfoContactData& _data,
        LibFred::ContactAddressType _address_type)
    : GeneralCheck::contact_optional_address(true)
{
    LibFred::ContactAddressList::const_iterator addr_ptr = _data.addresses.find(_address_type);
    if (addr_ptr != _data.addresses.end())
    {
        this->GeneralCheck::contact_optional_address::operator()(
                addr_ptr->second.street1,
                addr_ptr->second.city,
                addr_ptr->second.postalcode,
                addr_ptr->second.country);


    }
}


namespace MojeId {


check_contact_ssn::check_contact_ssn(const LibFred::InfoContactData& _data)
{
    const bool ssn_has_to_be_birthday = _data.organization.isnull() ||
                                        _data.organization.get_value().empty();
    if (ssn_has_to_be_birthday)
    {
        const check_contact_birthday check(_data);
        birthdate_absent = check.absent;
        birthdate_invalid = check.invalid;
        vat_id_num_absent = false;
        vat_id_num_invalid = false;
    }
    else
    {
        const check_contact_vat_id_presence check(_data);
        birthdate_absent = false;
        birthdate_invalid = false;
        vat_id_num_absent = check.absent;
        vat_id_num_invalid = false;
    }
}


} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred
