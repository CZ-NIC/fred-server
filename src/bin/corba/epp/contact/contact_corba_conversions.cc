/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/corba/epp/contact/contact_corba_conversions.hh"
#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/update_operation.hh"

#include "src/bin/corba/EPP.hh"

#include "src/bin/corba/epp/corba_conversions.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/util/corba_conversion.hh"
#include "util/db/nullable.hh"
#include "util/map_at.hh"
#include "util/optional_value.hh"

#include <boost/algorithm/string/trim.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace LibFred {
namespace Corba {

namespace {

bool does_contact_change_string_mean_to_delete(const char* value)
{
    return value[0] == '\b';
}

bool does_contact_change_string_mean_not_to_touch(const char* value)
{
    return value[0] == '\0';
}

bool does_contact_create_string_mean_not_to_present(const char* value)
{
    return value[0] == '\0';
}

Epp::Deletable<std::string> make_deletable_string(const char* src)
{
    const bool src_has_special_meaning_to_delete = does_contact_change_string_mean_to_delete(src);
    if (src_has_special_meaning_to_delete)
    {
        return Epp::Deletable<std::string>(Epp::UpdateOperation::delete_value());
    }
    const bool src_has_special_meaning_not_to_touch = does_contact_change_string_mean_not_to_touch(src);
    if (src_has_special_meaning_not_to_touch)
    {
        return Epp::Deletable<std::string>(Epp::UpdateOperation::no_operation());
    }
    const std::string value_to_set = LibFred::Corba::unwrap_string(src);
    const bool value_to_set_means_not_to_touch = value_to_set.empty();
    if (value_to_set_means_not_to_touch)
    {
        return Epp::Deletable<std::string>(Epp::UpdateOperation::no_operation());
    }
    return Epp::Deletable<std::string>(Epp::UpdateOperation::set_value(value_to_set));
}

Epp::Updateable<std::string> make_updateable_string(const char* src)
{
    const bool src_has_special_meaning_not_to_touch = does_contact_change_string_mean_not_to_touch(src);
    if (src_has_special_meaning_not_to_touch)
    {
        return Epp::Updateable<std::string>(Epp::UpdateOperation::no_operation());
    }
    const std::string value_to_set = LibFred::Corba::unwrap_string(src);
    const bool value_to_set_means_not_to_touch = value_to_set.empty();
    if (value_to_set_means_not_to_touch)
    {
        return Epp::Updateable<std::string>(Epp::UpdateOperation::no_operation());
    }
    return Epp::Updateable<std::string>(Epp::UpdateOperation::set_value(value_to_set));
}

Epp::Contact::HideableOptional<std::string> make_hideable_optional_string(
        const char* value,
        ccReg::PrivacyPolicy publishability)
{
    const bool src_has_special_meaning_not_to_present = does_contact_create_string_mean_not_to_present(value);
    boost::optional<std::string> dst_value;
    if (src_has_special_meaning_not_to_present)
    {
        dst_value = boost::none;
    }
    else
    {
        dst_value = Corba::unwrap_string(value);
    }
    switch (publishability)
    {
        case ccReg::public_data:
            return Epp::Contact::make_public_data(dst_value);
        case ccReg::private_data:
            return Epp::Contact::make_private_data(dst_value);
        case ccReg::unused_privacy_policy:
            return Epp::Contact::make_data_with_unspecified_privacy(dst_value);
    }
    throw std::logic_error("unexpected privacy policy");
}

Epp::Contact::ContactChange::MainAddress make_contact_change_main_address(
        const ccReg::Lists& streets,
        const char* city,
        const char* state_or_province,
        const char* postal_code,
        const char* country_code)
{
    Epp::Contact::ContactChange::MainAddress value;
    value.city = make_deletable_string(city);
    value.state_or_province = make_deletable_string(state_or_province);
    value.postal_code = make_deletable_string(postal_code);
    value.country_code = make_updateable_string(country_code);
    const bool street_change_requested = 0 < streets.length();
    if (street_change_requested)
    {
        for (unsigned idx = 0; idx < value.street.size(); ++idx)
        {
            if (idx < streets.length())
            {
                value.street[idx] = Epp::UpdateOperation::set_value(Corba::unwrap_string(streets[idx]));
            }
            else
            {
                value.street[idx] = Epp::UpdateOperation::no_operation();
            }
        }
    }
    else
    {
        const bool address_has_to_be_changed =
                !((value.city == Epp::UpdateOperation::Action::no_operation) &&
                  (value.state_or_province == Epp::UpdateOperation::Action::no_operation) &&
                  (value.postal_code == Epp::UpdateOperation::Action::no_operation) &&
                  (value.country_code == Epp::UpdateOperation::Action::no_operation));
        if (address_has_to_be_changed)
        {
            //case "one of city, state, province, postal code or country code are set and no street is set" has
            //very special meaning: clean all "streets"
            value.street[0] = Epp::UpdateOperation::delete_value();
            value.street[1] = Epp::UpdateOperation::delete_value();
            value.street[2] = Epp::UpdateOperation::delete_value();
        }
        else
        {
            value.street[0] = Epp::UpdateOperation::no_operation();
            value.street[1] = Epp::UpdateOperation::no_operation();
            value.street[2] = Epp::UpdateOperation::no_operation();
        }
    }
    return value;
}

void unwrap_contact_create_string(const char* src, boost::optional<std::string>& dst)
{
    const bool src_has_special_meaning_not_to_present = does_contact_create_string_mean_not_to_present(src);
    if (src_has_special_meaning_not_to_present)
    {
        dst = boost::none;
    }
    else
    {
        dst = Corba::unwrap_string(src);
    }
}

template <typename T>
Epp::Contact::ContactIdent make_contact_ident(const char* ident_value)
{
    return Epp::Contact::ContactIdent(Epp::Contact::ContactIdentValueOf<T>(Corba::unwrap_string(ident_value)));
}

Epp::Deletable<Epp::Contact::ContactIdent> make_deletable_contact_ident(
        ccReg::identtyp ident_type,
        const char* ident_value)
{
    if (does_contact_change_string_mean_to_delete(ident_value))
    {
        return Epp::Deletable<Epp::Contact::ContactIdent>(Epp::UpdateOperation::delete_value());
    }
    if (does_contact_change_string_mean_not_to_touch(ident_value))
    {
        if (ident_type != ccReg::EMPTY)//set ident type without setting of ident value is prohibited
        {//this case is not accessible through the EPP XML interface stimulation
            return Epp::UpdateOperation::operation_not_specified;
        }
        return Epp::Deletable<Epp::Contact::ContactIdent>(Epp::UpdateOperation::no_operation());
    }
    switch (ident_type)
    {
        case ccReg::EMPTY://set value of unspecified ident type is prohibited
            return Epp::UpdateOperation::operation_not_specified;
        case ccReg::OP:
            return Epp::Deletable<Epp::Contact::ContactIdent>(
                    Epp::UpdateOperation::set_value(
                            make_contact_ident<Epp::Contact::ContactIdentType::Op>(ident_value)));
        case ccReg::PASS:
            return Epp::Deletable<Epp::Contact::ContactIdent>(
                    Epp::UpdateOperation::set_value(
                            make_contact_ident<Epp::Contact::ContactIdentType::Pass>(ident_value)));
        case ccReg::ICO:
            return Epp::Deletable<Epp::Contact::ContactIdent>(
                    Epp::UpdateOperation::set_value(
                            make_contact_ident<Epp::Contact::ContactIdentType::Ico>(ident_value)));
        case ccReg::MPSV:
            return Epp::Deletable<Epp::Contact::ContactIdent>(
                    Epp::UpdateOperation::set_value(
                            make_contact_ident<Epp::Contact::ContactIdentType::Mpsv>(ident_value)));
            break;
        case ccReg::BIRTHDAY:
            return Epp::Deletable<Epp::Contact::ContactIdent>(
                    Epp::UpdateOperation::set_value(
                            make_contact_ident<Epp::Contact::ContactIdentType::Birthday>(ident_value)));
    }
    throw std::runtime_error("Invalid identtyp value.");
}

void unwrap_contact_change_address(const ccReg::AddressData& src, Epp::Contact::ContactChange::Address& dst)
{
    unwrap_contact_create_string(src.Street1, dst.street[0]);
    unwrap_contact_create_string(src.Street2, dst.street[1]);
    unwrap_contact_create_string(src.Street3, dst.street[2]);
    unwrap_contact_create_string(src.City, dst.city);
    unwrap_contact_create_string(src.StateOrProvince, dst.state_or_province);
    unwrap_contact_create_string(src.PostalCode, dst.postal_code);
    unwrap_contact_create_string(src.CountryCode, dst.country_code);
}

Epp::Deletable<Epp::Contact::ContactChange::Address> make_deletable_contact_change_address(
        const ccReg::AddressChange& src)
{
    switch (src.action)
    {
    case ccReg::set:
        {
            Epp::Contact::ContactChange::Address address;
            unwrap_contact_change_address(src.data, address);
            return Epp::Deletable<Epp::Contact::ContactChange::Address>(Epp::UpdateOperation::set_value(address));
        }
    case ccReg::remove:
        return Epp::Deletable<Epp::Contact::ContactChange::Address>(Epp::UpdateOperation::delete_value());
    case ccReg::no_change:
        return Epp::Deletable<Epp::Contact::ContactChange::Address>(Epp::UpdateOperation::no_operation());
    }
    throw std::runtime_error("unexpected value of ccReg::AddressAction");
}

void unwrap_contact_data_address(const ccReg::AddressData& src, Epp::Contact::ContactData::Address& dst)
{
    unwrap_contact_create_string(src.Street1, dst.street[0]);
    unwrap_contact_create_string(src.Street2, dst.street[1]);
    unwrap_contact_create_string(src.Street3, dst.street[2]);
    unwrap_contact_create_string(src.City, dst.city);
    unwrap_contact_create_string(src.StateOrProvince, dst.state_or_province);
    unwrap_contact_create_string(src.PostalCode, dst.postal_code);
    unwrap_contact_create_string(src.CountryCode, dst.country_code);
}

void unwrap_optional_contact_data_address(
        const ccReg::OptionalAddressData& src,
        boost::optional<Epp::Contact::ContactData::Address>& dst)
{
    if (src.is_set)
    {
        Epp::Contact::ContactData::Address address;
        unwrap_contact_data_address(src.data, address);
        dst = address;
    }
    else
    {
        dst = boost::none;
    }
}

using HideableOptionalContactDataAddress = Epp::Contact::HideableOptional<Epp::Contact::ContactData::Address>;

HideableOptionalContactDataAddress make_hideable_optional_contact_data_address(
        const ccReg::Lists& streets,
        const char* city,
        const char* state_or_province,
        const char* postal_code,
        const char* country_code,
        ccReg::PrivacyPolicy publishability)
{
    Epp::Contact::ContactData::Address value;
    for (unsigned idx = 0; idx < value.street.size(); ++idx)
    {
        if (idx < streets.length())
        {
            unwrap_contact_create_string(streets[idx], value.street[idx]);
        }
        else
        {
            value.street[idx] = boost::none;
        }
    }
    unwrap_contact_create_string(city, value.city);
    unwrap_contact_create_string(state_or_province, value.state_or_province);
    unwrap_contact_create_string(postal_code, value.postal_code);
    unwrap_contact_create_string(country_code, value.country_code);
    const bool address_is_set =
            (value.street[0] != boost::none) ||
            (value.street[1] != boost::none) ||
            (value.street[2] != boost::none) ||
            (value.city != boost::none) ||
            (value.state_or_province != boost::none) ||
            (value.postal_code != boost::none) ||
            (value.country_code != boost::none);
    const auto optional_value = address_is_set ? boost::optional<Epp::Contact::ContactData::Address>(value)
                                               : boost::optional<Epp::Contact::ContactData::Address>();
    switch (publishability)
    {
        case ccReg::public_data:
            return Epp::Contact::make_public_data(optional_value);
        case ccReg::private_data:
            return Epp::Contact::make_private_data(optional_value);
        case ccReg::unused_privacy_policy:
            return Epp::Contact::make_data_with_unspecified_privacy(optional_value);
    }
    throw std::logic_error("unexpected privacy policy");
}

boost::optional<Epp::Contact::ContactIdent> make_optional_contact_ident(
        ccReg::identtyp ident_type,
        const char* ident_value)
{
    switch (ident_type)
    {
        case ccReg::EMPTY:
            return boost::none;
        case ccReg::OP:
            return make_contact_ident<Epp::Contact::ContactIdentType::Op>(ident_value);
        case ccReg::PASS:
            return make_contact_ident<Epp::Contact::ContactIdentType::Pass>(ident_value);
        case ccReg::ICO:
            return make_contact_ident<Epp::Contact::ContactIdentType::Ico>(ident_value);
        case ccReg::MPSV:
            return make_contact_ident<Epp::Contact::ContactIdentType::Mpsv>(ident_value);
        case ccReg::BIRTHDAY:
            return make_contact_ident<Epp::Contact::ContactIdentType::Birthday>(ident_value);
    }
    throw std::runtime_error("Invalid identtyp value.");
}

Epp::Contact::HideableOptional<Epp::Contact::ContactIdent> make_hideable_optional_contact_ident(
        ccReg::identtyp ident_type,
        const char* ident_value,
        ccReg::PrivacyPolicy publishability)
{
    const auto optional_ident = make_optional_contact_ident(ident_type, ident_value);
    switch (publishability)
    {
        case ccReg::public_data:
            return Epp::Contact::make_public_data(optional_ident);
        case ccReg::private_data:
            return Epp::Contact::make_private_data(optional_ident);
        case ccReg::unused_privacy_policy:
            return Epp::Contact::make_data_with_unspecified_privacy(optional_ident);
    }
    throw std::logic_error("unknown privacy policy");
}

boost::optional<::Epp::Contact::PrivacyPolicy> unwrap_privacy_policy(ccReg::PrivacyPolicy publishability)
{
    switch (publishability)
    {
        case ccReg::public_data:
            return ::Epp::Contact::PrivacyPolicy::show;
        case ccReg::private_data:
            return ::Epp::Contact::PrivacyPolicy::hide;
        case ccReg::unused_privacy_policy:
            return boost::none;
    }
    throw std::logic_error("unknown privacy policy");
}

::Epp::Contact::ContactChange::Publishability make_publishability(
        const ccReg::ControlledPrivacyData& disclose_flags)
{
    ::Epp::Contact::ContactChange::Publishability result;
    result.name = unwrap_privacy_policy(disclose_flags.Name);
    result.organization = unwrap_privacy_policy(disclose_flags.Organization);
    result.address = unwrap_privacy_policy(disclose_flags.Address);
    result.telephone = unwrap_privacy_policy(disclose_flags.Telephone);
    result.fax = unwrap_privacy_policy(disclose_flags.Fax);
    result.email = unwrap_privacy_policy(disclose_flags.Email);
    result.vat = unwrap_privacy_policy(disclose_flags.VAT);
    result.ident = unwrap_privacy_policy(disclose_flags.Ident);
    result.notify_email = unwrap_privacy_policy(disclose_flags.NotifyEmail);
    return result;
}

::Epp::Updateable<::Epp::Contact::ContactChange::Publishability> make_updateable_publishability(
        const ccReg::ControlledPrivacyDataChange& disclose_flags_change)
{
    ::Epp::Updateable<::Epp::Contact::ContactChange::Publishability> result;
    if (disclose_flags_change.update_data)
    {
        result = ::Epp::UpdateOperation::set_value(make_publishability(disclose_flags_change.data));
    }
    else
    {
        result = ::Epp::UpdateOperation::no_operation();
    }
    return result;
}

}//namespace LibFred::Corba::{anonymous}

void unwrap_ContactChange(const ccReg::ContactChange& src, Epp::Contact::ContactChange& dst)
{
    dst.name = make_deletable_string(src.Name);
    dst.organization = make_deletable_string(src.Organization);
    dst.address = make_contact_change_main_address(
            src.Streets,
            src.City,
            src.StateOrProvince,
            src.PostalCode,
            src.CC);
    dst.mailing_address = make_deletable_contact_change_address(src.MailingAddress);
    dst.telephone = make_deletable_string(src.Telephone);
    dst.fax = make_deletable_string(src.Fax);
    dst.email = make_deletable_string(src.Email);
    dst.notify_email = make_deletable_string(src.NotifyEmail);
    dst.vat = make_deletable_string(src.VAT);
    dst.ident = make_deletable_contact_ident(src.identtype, src.ident);
    dst.authinfopw = make_deletable_string(src.AuthInfoPw);
    dst.disclose = make_updateable_publishability(src.DiscloseFlags);
}

void unwrap_ContactData(const ccReg::ContactData& src, Epp::Contact::ContactData& dst)
{
    dst.name = make_hideable_optional_string(src.Name, src.DiscloseFlags.Name);
    dst.organization = make_hideable_optional_string(src.Organization, src.DiscloseFlags.Organization);
    dst.address = make_hideable_optional_contact_data_address(
            src.Streets,
            src.City,
            src.StateOrProvince,
            src.PostalCode,
            src.CC,
            src.DiscloseFlags.Address);
    unwrap_optional_contact_data_address(src.MailingAddress, dst.mailing_address);
    dst.telephone = make_hideable_optional_string(src.Telephone, src.DiscloseFlags.Telephone);
    dst.fax = make_hideable_optional_string(src.Fax, src.DiscloseFlags.Fax);
    dst.email = make_hideable_optional_string(src.Email, src.DiscloseFlags.Email);
    dst.notify_email = make_hideable_optional_string(src.NotifyEmail, src.DiscloseFlags.NotifyEmail);
    dst.vat = make_hideable_optional_string(src.VAT, src.DiscloseFlags.VAT);
    dst.ident = make_hideable_optional_contact_ident(src.identtype, src.ident, src.DiscloseFlags.Ident);
    unwrap_contact_create_string(src.AuthInfoPw, dst.authinfopw);
}

namespace {

ccReg::CheckAvail wrap_contact_handle_check_result(
        const boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized>& _obstruction)
{
    if (!_obstruction.is_initialized())
    {
        return ccReg::NotExist;
    }

    switch (_obstruction.get().state)
    {
        case Epp::Contact::ContactHandleRegistrationObstruction::invalid_handle:
            return ccReg::BadFormat;

        case Epp::Contact::ContactHandleRegistrationObstruction::protected_handle:
            return ccReg::DelPeriod; // strange but correct

        case Epp::Contact::ContactHandleRegistrationObstruction::registered_handle:
            return ccReg::Exist;
    }

    throw std::runtime_error("unknown_contact_state");
}

struct Ident
{
    ccReg::identtyp type;
    CORBA::String_var value;
};

struct GetIdentFromContactIdent : boost::static_visitor<Ident>
{
    Ident operator()(const Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Op>& src)const
    {
        Ident ident;
        ident.type = ccReg::OP;
        ident.value = LibFred::Corba::wrap_string_to_corba_string(src.value);
        return ident;
    }
    Ident operator()(const Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Pass>& src)const
    {
        Ident ident;
        ident.type = ccReg::PASS;
        ident.value = LibFred::Corba::wrap_string_to_corba_string(src.value);
        return ident;
    }
    Ident operator()(const Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Ico>& src)const
    {
        Ident ident;
        ident.type = ccReg::ICO;
        ident.value = LibFred::Corba::wrap_string_to_corba_string(src.value);
        return ident;
    }
    Ident operator()(const Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Mpsv>& src)const
    {
        Ident ident;
        ident.type = ccReg::MPSV;
        ident.value = LibFred::Corba::wrap_string_to_corba_string(src.value);
        return ident;
    }
    Ident operator()(const Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Birthday>& src)const
    {
        Ident ident;
        ident.type = ccReg::BIRTHDAY;
        ident.value = LibFred::Corba::wrap_string_to_corba_string(src.value);
        return ident;
    }
};

Ident wrap_ident(const boost::optional<Epp::Contact::ContactIdent>& src)
{
    Ident ident;
    if (!static_cast<bool>(src))
    {
        ident.type = ccReg::EMPTY;
        ident.value = "";
        return ident;
    }
    ident = boost::apply_visitor(GetIdentFromContactIdent(), *src);
    return ident;
}

} // namespace LibFred::Corba::{anonymous}

ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map<std::string,
                boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized>>& contact_handle_check_results)
{
    ccReg::CheckResp result;
    result.length(contact_handles.size());

    CORBA::ULong i = 0;
    for (std::vector<std::string>::const_iterator it = contact_handles.begin();
         it != contact_handles.end();
         ++it, ++i)
    {
        const boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized> check_result =
                map_at(contact_handle_check_results, *it);

        result[i].avail = wrap_contact_handle_check_result(check_result);
        result[i].reason = wrap_string_to_corba_string(
                check_result.is_initialized() ? check_result.get().description : "");
    }

    return result;
}

namespace {

CORBA::String_var wrap_optional_string_to_corba_string(const boost::optional<std::string>& src)
{
    return LibFred::Corba::wrap_string_to_corba_string(!static_cast<bool>(src) ? std::string() : *src);
}

CORBA::String_var wrap_optional_ptime_to_string(const boost::optional<boost::posix_time::ptime>& src)
{
    return static_cast<bool>(src) ? wrap_boost_posix_time_ptime_to_string(*src) : "";
}


ccReg::OptionalAddressData wrap_OptionalAddressData(
        const boost::optional<Epp::Contact::ContactData::Address>& src)
{
    ccReg::OptionalAddressData dst;
    if (!static_cast<bool>(src))
    {
        dst.is_set = wrap_int<CORBA::Boolean>(false);
        return dst;
    }
    dst.is_set = wrap_int<CORBA::Boolean>(true);
    dst.data.Street1 = wrap_optional_string_to_corba_string(src->street[0]);
    dst.data.Street2 = wrap_optional_string_to_corba_string(src->street[1]);
    dst.data.Street3 = wrap_optional_string_to_corba_string(src->street[2]);
    dst.data.City = wrap_optional_string_to_corba_string(src->city);
    dst.data.StateOrProvince = wrap_optional_string_to_corba_string(src->state_or_province);
    dst.data.PostalCode = wrap_optional_string_to_corba_string(src->postal_code);
    dst.data.CountryCode = wrap_optional_string_to_corba_string(src->country_code);
    return dst;
}

template <typename T>
ccReg::PrivacyPolicy wrap_privacy_policy(const Epp::Contact::Hideable<T>& src)
{
    if (!src.is_publishability_specified())
    {
        return ccReg::unused_privacy_policy;
    }
    if (src.is_public())
    {
        return ccReg::public_data;
    }
    if (src.is_private())
    {
        return ccReg::private_data;
    }
    throw std::runtime_error("unexpected PrivacyPolicy value");
}

} // namespace LibFred::Corba::{anonymous}

void wrap_InfoContactLocalizedOutputData(
        const Epp::Contact::InfoContactLocalizedOutputData& src,
        ccReg::Contact& dst)
{
    dst.handle = wrap_string_to_corba_string(src.handle);
    dst.ROID = wrap_string_to_corba_string(src.roid);
    dst.ClID = wrap_string_to_corba_string(src.sponsoring_registrar_handle);
    dst.CrID = wrap_string_to_corba_string(src.creating_registrar_handle);
    // XXX IDL nonsense
    dst.UpID = wrap_optional_string_to_corba_string(src.last_update_registrar_handle);
    wrap_Epp_ObjectStatesLocalized<::Epp::Contact::StatusValue>(src.localized_external_states, dst.stat);
    dst.CrDate = wrap_boost_posix_time_ptime_to_string(src.crdate);
    // XXX IDL nonsense
    dst.UpDate = wrap_optional_ptime_to_string(src.last_update);
    // XXX IDL nonsense
    dst.TrDate = wrap_optional_ptime_to_string(src.last_transfer);
    dst.Name = wrap_optional_string_to_corba_string(*src.name);
    dst.Organization = wrap_optional_string_to_corba_string(*src.organization);

    class Value
    {
    public:
        explicit Value(const boost::optional<std::string>& _src) : src_(_src) { }
        bool does_present()const
        {
            return static_cast<bool>(src_) && !src_->empty();
        }
    private:
        const boost::optional<std::string>& src_;
    };
    const auto number_of_streets =
            Value(src.address->street[2]).does_present() ? 3
                : Value(src.address->street[1]).does_present() ? 2
                    : Value(src.address->street[0]).does_present() ? 1 : 0;
    dst.Streets.length(number_of_streets);
    if (0 < number_of_streets)
    {
        dst.Streets[0] = wrap_optional_string_to_corba_string(src.address->street[0]);
    }
    if (1 < number_of_streets)
    {
        dst.Streets[1] = wrap_optional_string_to_corba_string(src.address->street[1]);
    }
    if (2 < number_of_streets)
    {
        dst.Streets[2] = wrap_optional_string_to_corba_string(src.address->street[2]);
    }

    dst.City = wrap_optional_string_to_corba_string(src.address->city);
    dst.StateOrProvince = wrap_optional_string_to_corba_string(src.address->state_or_province);
    dst.PostalCode = wrap_optional_string_to_corba_string(src.address->postal_code);
    dst.CountryCode = wrap_optional_string_to_corba_string(src.address->country_code);
    dst.Telephone = wrap_optional_string_to_corba_string(*src.telephone);
    dst.MailingAddress = wrap_OptionalAddressData(src.mailing_address);
    dst.Fax = wrap_optional_string_to_corba_string(*src.fax);
    dst.Email = wrap_optional_string_to_corba_string(*src.email);
    dst.NotifyEmail = wrap_optional_string_to_corba_string(*src.notify_email);
    dst.VAT = wrap_optional_string_to_corba_string(*src.VAT);
    const Ident ident = wrap_ident(*src.ident);
    dst.ident = ident.value;
    dst.identtype = ident.type;
    dst.AuthInfoPw = wrap_optional_string_to_corba_string(src.authinfopw);
    dst.DiscloseFlags.Name = wrap_privacy_policy(src.name);
    dst.DiscloseFlags.Organization = wrap_privacy_policy(src.organization);
    dst.DiscloseFlags.Address = wrap_privacy_policy(src.address);
    dst.DiscloseFlags.Telephone = wrap_privacy_policy(src.telephone);
    dst.DiscloseFlags.Fax = wrap_privacy_policy(src.fax);
    dst.DiscloseFlags.Email = wrap_privacy_policy(src.email);
    dst.DiscloseFlags.VAT = wrap_privacy_policy(src.VAT);
    dst.DiscloseFlags.Ident = wrap_privacy_policy(src.ident);
    dst.DiscloseFlags.NotifyEmail = wrap_privacy_policy(src.notify_email);
}

} // namespace LibFred::Corba
} // namespace LibFred
