/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/bin/corba/epp/contact/contact_corba_conversions.hh"
#include "src/backend/epp/contact/contact_data.hh"

#include "src/bin/corba/EPP.hh"

#include "src/bin/corba/epp/corba_conversions.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/util/corba_conversion.hh"
#include "src/util/db/nullable.hh"
#include "src/util/map_at.hh"
#include "src/util/optional_value.hh"

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

boost::optional< Nullable<std::string> > convert_contact_update_or_delete_string(const char* src)
{
    const bool src_has_special_meaning_to_delete = does_contact_change_string_mean_to_delete(src);
    if (src_has_special_meaning_to_delete)
    {
        return Nullable<std::string>();
    }
    const bool src_has_special_meaning_not_to_touch = does_contact_change_string_mean_not_to_touch(src);
    if (src_has_special_meaning_not_to_touch)
    {
        return boost::optional< Nullable<std::string> >();
    }
    const std::string value_to_set = LibFred::Corba::unwrap_string(src);
    const bool value_to_set_means_not_to_touch = value_to_set.empty();
    if (value_to_set_means_not_to_touch)
    {
        return boost::optional< Nullable<std::string> >();
    }
    return Nullable<std::string>(value_to_set);
}

boost::optional<std::string> convert_contact_update_string(const char* src)
{
    const bool src_has_special_meaning_not_to_touch = does_contact_change_string_mean_not_to_touch(src);
    if (src_has_special_meaning_not_to_touch)
    {
        return boost::optional<std::string>();
    }
    const std::string value_to_set = LibFred::Corba::unwrap_string(src);
    const bool value_to_set_means_not_to_touch = value_to_set.empty();
    if (value_to_set_means_not_to_touch)
    {
        return boost::optional<std::string>();
    }
    return boost::optional<std::string>(value_to_set);
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

Epp::Contact::ContactDisclose convert_ContactChange_to_ContactDisclose(
        const ccReg::ContactChange& src,
        Epp::Contact::ContactDisclose::Flag::Enum meaning)
{
    Epp::Contact::ContactDisclose result(meaning);
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseName))
    {
        result.add<Epp::Contact::ContactDisclose::Item::name>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseOrganization))
    {
        result.add<Epp::Contact::ContactDisclose::Item::organization>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseAddress))
    {
        result.add<Epp::Contact::ContactDisclose::Item::address>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseTelephone))
    {
        result.add<Epp::Contact::ContactDisclose::Item::telephone>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseFax))
    {
        result.add<Epp::Contact::ContactDisclose::Item::fax>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseEmail))
    {
        result.add<Epp::Contact::ContactDisclose::Item::email>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseVAT))
    {
        result.add<Epp::Contact::ContactDisclose::Item::vat>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseIdent))
    {
        result.add<Epp::Contact::ContactDisclose::Item::ident>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseNotifyEmail))
    {
        result.add<Epp::Contact::ContactDisclose::Item::notify_email>();
    }

    return result;
}

Epp::Contact::ContactDisclose convert_ContactData_to_ContactDisclose(
        const ccReg::ContactData& src,
        Epp::Contact::ContactDisclose::Flag::Enum meaning)
{
    Epp::Contact::ContactDisclose result(meaning);
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseName))
    {
        result.add<Epp::Contact::ContactDisclose::Item::name>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseOrganization))
    {
        result.add<Epp::Contact::ContactDisclose::Item::organization>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseAddress))
    {
        result.add<Epp::Contact::ContactDisclose::Item::address>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseTelephone))
    {
        result.add<Epp::Contact::ContactDisclose::Item::telephone>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseFax))
    {
        result.add<Epp::Contact::ContactDisclose::Item::fax>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseEmail))
    {
        result.add<Epp::Contact::ContactDisclose::Item::email>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseVAT))
    {
        result.add<Epp::Contact::ContactDisclose::Item::vat>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseIdent))
    {
        result.add<Epp::Contact::ContactDisclose::Item::ident>();
    }
    if (LibFred::Corba::wrap_int<bool>(src.DiscloseNotifyEmail))
    {
        result.add<Epp::Contact::ContactDisclose::Item::notify_email>();
    }

    return result;
}

boost::optional<Epp::Contact::ContactDisclose> unwrap_ContactChange_to_ContactDisclose(
        const ccReg::ContactChange& src)
{
    switch (src.DiscloseFlag)
    {
        case ccReg::DISCL_EMPTY:
            return boost::optional<Epp::Contact::ContactDisclose>();
        case ccReg::DISCL_HIDE:
            return convert_ContactChange_to_ContactDisclose(src, Epp::Contact::ContactDisclose::Flag::hide);
        case ccReg::DISCL_DISPLAY:
            return convert_ContactChange_to_ContactDisclose(src, Epp::Contact::ContactDisclose::Flag::disclose);
    }
    throw std::runtime_error("Invalid DiscloseFlag value");
}

boost::optional<Epp::Contact::ContactDisclose> unwrap_ContactData_to_ContactDisclose(
        const ccReg::ContactData& src)
{
    switch (src.DiscloseFlag)
    {
        case ccReg::DISCL_EMPTY:
            return boost::optional<Epp::Contact::ContactDisclose>();
        case ccReg::DISCL_HIDE:
            return convert_ContactData_to_ContactDisclose(src, Epp::Contact::ContactDisclose::Flag::hide);
        case ccReg::DISCL_DISPLAY:
            return convert_ContactData_to_ContactDisclose(src, Epp::Contact::ContactDisclose::Flag::disclose);
    }
    throw std::runtime_error("Invalid DiscloseFlag value");
}

void unwrap_ident(ccReg::identtyp type, const char* value, boost::optional<Epp::Contact::ContactIdent>& dst)
{
    switch (type)
    {
        case ccReg::EMPTY:
            dst = boost::none;
            return;
        case ccReg::OP:
            dst = Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Op>(Corba::unwrap_string(value));
            return;
        case ccReg::PASS:
            dst = Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Pass>(Corba::unwrap_string(value));
            return;
        case ccReg::ICO:
            dst = Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Ico>(Corba::unwrap_string(value));
            return;
        case ccReg::MPSV:
            dst = Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Mpsv>(Corba::unwrap_string(value));
            return;
        case ccReg::BIRTHDAY:
            dst = Epp::Contact::ContactIdentValueOf<Epp::Contact::ContactIdentType::Birthday>(Corba::unwrap_string(value));
            return;
    }
    throw std::runtime_error("Invalid identtyp value.");
}

void unwrap_ident(
        ccReg::identtyp type,
        const char* value,
        boost::optional< boost::optional<Epp::Contact::ContactIdent> >& dst)
{
    if (does_contact_change_string_mean_to_delete(value))
    {
        dst = boost::optional<Epp::Contact::ContactIdent>();
        return;
    }
    if (does_contact_change_string_mean_not_to_touch(value))
    {
        dst = boost::none;
        return;
    }
    boost::optional<Epp::Contact::ContactIdent> ident;
    unwrap_ident(type, value, ident);
    dst = ident;
}

template <class T>
void unwrap_address(const ccReg::AddressData& src, T& dst)
{
    unwrap_contact_create_string(src.Street1, dst.street1);
    unwrap_contact_create_string(src.Street2, dst.street2);
    unwrap_contact_create_string(src.Street3, dst.street3);
    unwrap_contact_create_string(src.City, dst.city);
    unwrap_contact_create_string(src.StateOrProvince, dst.state_or_province);
    unwrap_contact_create_string(src.PostalCode, dst.postal_code);
    unwrap_contact_create_string(src.CountryCode, dst.country_code);
}

template <class T>
void unwrap_optional_address(
        const ccReg::OptionalAddressData& src,
        boost::optional<T>& dst)
{
    bool is_address_set;
    CorbaConversion::unwrap_int(src.is_set, is_address_set);
    if (is_address_set)
    {
        T address;
        unwrap_address(src.data, address);
        dst = address;
    }
    else
    {
        dst = boost::none;
    }
}

void unwrap_address_change(
        const ccReg::AddressChange& src,
        boost::optional< Nullable<Epp::Contact::ContactChange::Address> >& dst)
{
    switch (src.action)
    {
        case ccReg::set:
            {
                Epp::Contact::ContactChange::Address address;
                unwrap_address(src.data, address);
                dst = Nullable<Epp::Contact::ContactChange::Address>(address);
                return;
            }
        case ccReg::remove:
            dst = Nullable<Epp::Contact::ContactChange::Address>();
            return;
        case ccReg::no_change:
            dst = boost::none;
            return;
    }
    throw std::runtime_error("unexpected value of AddressAction");
}

} // namespace LibFred::Corba::{anonymous}

void unwrap_ContactChange(const ccReg::ContactChange& src, Epp::Contact::ContactChange& dst)
{
    dst.name = convert_contact_update_or_delete_string(src.Name);
    dst.organization = convert_contact_update_or_delete_string(src.Organization);
    const bool street_change_requested = 0 < src.Streets.length();
    if (street_change_requested)
    {
        dst.streets = std::vector<std::string>();
        dst.streets->reserve(src.Streets.length());
        for (unsigned idx = 0; idx < src.Streets.length(); ++idx)
        {
            dst.streets->push_back(Corba::unwrap_string(src.Streets[idx]));
        }
    }
    else
    {
        dst.streets = boost::none;
    }
    dst.city = convert_contact_update_or_delete_string(src.City);
    dst.state_or_province = convert_contact_update_or_delete_string(src.StateOrProvince);
    dst.postal_code = convert_contact_update_or_delete_string(src.PostalCode);
    dst.country_code = convert_contact_update_string(src.CC);
    unwrap_address_change(src.MailingAddress, dst.mailing_address);
    dst.telephone = convert_contact_update_or_delete_string(src.Telephone);
    dst.fax = convert_contact_update_or_delete_string(src.Fax);
    dst.email = convert_contact_update_or_delete_string(src.Email);
    dst.notify_email = convert_contact_update_or_delete_string(src.NotifyEmail);
    dst.vat = convert_contact_update_or_delete_string(src.VAT);
    unwrap_ident(src.identtype, src.ident, dst.ident);
    dst.authinfopw = convert_contact_update_or_delete_string(src.AuthInfoPw);
    dst.disclose = unwrap_ContactChange_to_ContactDisclose(src);
}

void unwrap_ContactData(const ccReg::ContactData& src, Epp::Contact::ContactData& dst)
{
    unwrap_contact_create_string(src.Name, dst.name);
    unwrap_contact_create_string(src.Organization, dst.organization);
    dst.streets.reserve(src.Streets.length());
    for (unsigned idx = 0; idx < src.Streets.length(); ++idx)
    {
        boost::optional<std::string> street;
        unwrap_contact_create_string(src.Streets[idx], street);
        if (!static_cast<bool>(street))
        {
            street = std::string();
        }
        dst.streets.push_back(*street);
    }
    unwrap_contact_create_string(src.City, dst.city);
    unwrap_contact_create_string(src.StateOrProvince, dst.state_or_province);
    unwrap_contact_create_string(src.PostalCode, dst.postal_code);
    unwrap_contact_create_string(src.CC, dst.country_code);
    unwrap_optional_address(src.MailingAddress, dst.mailing_address);
    unwrap_contact_create_string(src.Telephone, dst.telephone);
    unwrap_contact_create_string(src.Fax, dst.fax);
    unwrap_contact_create_string(src.Email, dst.email);
    unwrap_contact_create_string(src.NotifyEmail, dst.notify_email);
    unwrap_contact_create_string(src.VAT, dst.vat);
    unwrap_ident(src.identtype, src.ident, dst.ident);
    unwrap_contact_create_string(src.AuthInfoPw, dst.authinfopw);
    dst.disclose = unwrap_ContactData_to_ContactDisclose(src);
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

struct GetIdentFromContactIdent:boost::static_visitor<Ident>
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
                boost::optional<Epp::Contact::ContactHandleRegistrationObstructionLocalized> >& contact_handle_check_results)
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
    dst.data.Street1 = wrap_optional_string_to_corba_string(src->street1);
    dst.data.Street2 = wrap_optional_string_to_corba_string(src->street2);
    dst.data.Street3 = wrap_optional_string_to_corba_string(src->street3);
    dst.data.City = wrap_optional_string_to_corba_string(src->city);
    dst.data.StateOrProvince = wrap_optional_string_to_corba_string(src->state_or_province);
    dst.data.PostalCode = wrap_optional_string_to_corba_string(src->postal_code);
    dst.data.CountryCode = wrap_optional_string_to_corba_string(src->country_code);
    return dst;
}

template <Epp::Contact::ContactDisclose::Item::Enum item>
CORBA::Boolean presents(const Epp::Contact::ContactDisclose& _src)
{
    return wrap_int<CORBA::Boolean>(_src.presents<item>());
}

} // namespace LibFred::Corba::{anonymous}

void wrap_InfoContactLocalizedOutputData(
        const Epp::Contact::InfoContactLocalizedOutputData& _src,
        ccReg::Contact& _dst)
{
    _dst.handle = wrap_string_to_corba_string(_src.handle);
    _dst.ROID = wrap_string_to_corba_string(_src.roid);
    _dst.ClID = wrap_string_to_corba_string(_src.sponsoring_registrar_handle);
    _dst.CrID = wrap_string_to_corba_string(_src.creating_registrar_handle);
    // XXX IDL nonsense
    _dst.UpID = wrap_Nullable_string_to_string(_src.last_update_registrar_handle);
    wrap_Epp_ObjectStatesLocalized< ::Epp::Contact::StatusValue>(_src.localized_external_states, _dst.stat);
    _dst.CrDate = wrap_boost_posix_time_ptime_to_string(_src.crdate);
    // XXX IDL nonsense
    _dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_update);
    // XXX IDL nonsense
    _dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_transfer);
    _dst.Name = wrap_Nullable_string_to_string(_src.name);
    _dst.Organization = wrap_Nullable_string_to_string(_src.organization);

    class Value
    {
    public:
        explicit Value(const Nullable<std::string>& _src):src_(_src) { }
        bool does_present()const
        {
            return !src_.isnull() && !src_.get_value().empty();
        }
    private:
        const Nullable<std::string>& src_;
    };
    const unsigned number_of_streets =
            Value(_src.street3).does_present() ? 3
                : Value(_src.street2).does_present() ? 2
                    : Value(_src.street1).does_present() ? 1 : 0;
    _dst.Streets.length(number_of_streets);
    if (0 < number_of_streets)
    {
        _dst.Streets[0] = wrap_Nullable_string_to_string(_src.street1);
    }
    if (1 < number_of_streets)
    {
        _dst.Streets[1] = wrap_Nullable_string_to_string(_src.street2);
    }
    if (2 < number_of_streets)
    {
        _dst.Streets[2] = wrap_Nullable_string_to_string(_src.street3);
    }

    _dst.City = wrap_Nullable_string_to_string(_src.city);
    _dst.StateOrProvince = wrap_Nullable_string_to_string(_src.state_or_province);
    _dst.PostalCode = wrap_Nullable_string_to_string(_src.postal_code);
    _dst.CountryCode = wrap_Nullable_string_to_string(_src.country_code);
    _dst.Telephone = wrap_Nullable_string_to_string(_src.telephone);
    _dst.MailingAddress = wrap_OptionalAddressData(_src.mailing_address);
    _dst.Fax = wrap_Nullable_string_to_string(_src.fax);
    _dst.Email = wrap_Nullable_string_to_string(_src.email);
    _dst.NotifyEmail = wrap_Nullable_string_to_string(_src.notify_email);
    _dst.VAT = wrap_Nullable_string_to_string(_src.VAT);
    const Ident ident = wrap_ident(_src.ident);
    _dst.ident = ident.value;
    _dst.identtype = ident.type;
    _dst.AuthInfoPw = wrap_optional_string_to_corba_string(_src.authinfopw);

    if (!static_cast<bool>(_src.disclose))
    {
        _dst.DiscloseFlag = ccReg::DISCL_EMPTY;
        _dst.DiscloseName = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseOrganization = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseAddress = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseTelephone = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseFax = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseEmail = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseVAT = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseIdent = wrap_int<CORBA::Boolean>(false);
        _dst.DiscloseNotifyEmail = wrap_int<CORBA::Boolean>(false);
    }
    else
    {
        _dst.DiscloseFlag = _src.disclose->does_present_item_mean_to_disclose()
                                ? ccReg::DISCL_DISPLAY
                                : ccReg::DISCL_HIDE;
        _dst.DiscloseName = presents<Epp::Contact::ContactDisclose::Item::name>(*_src.disclose);
        _dst.DiscloseOrganization = presents<Epp::Contact::ContactDisclose::Item::organization>(*_src.disclose);
        _dst.DiscloseAddress = presents<Epp::Contact::ContactDisclose::Item::address>(*_src.disclose);
        _dst.DiscloseTelephone = presents<Epp::Contact::ContactDisclose::Item::telephone>(*_src.disclose);
        _dst.DiscloseFax = presents<Epp::Contact::ContactDisclose::Item::fax>(*_src.disclose);
        _dst.DiscloseEmail = presents<Epp::Contact::ContactDisclose::Item::email>(*_src.disclose);
        _dst.DiscloseVAT = presents<Epp::Contact::ContactDisclose::Item::vat>(*_src.disclose);
        _dst.DiscloseIdent = presents<Epp::Contact::ContactDisclose::Item::ident>(*_src.disclose);
        _dst.DiscloseNotifyEmail = presents<Epp::Contact::ContactDisclose::Item::notify_email>(*_src.disclose);
    }
}

} // namespace LibFred::Corba
} // namespace LibFred
