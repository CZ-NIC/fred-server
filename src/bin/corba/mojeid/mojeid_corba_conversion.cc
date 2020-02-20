/*
 * Copyright (C) 2008-2020  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  implementation for MojeID CORBA conversion
 */

#include "src/backend/buffer.hh"
#include "src/bin/corba/IsoDate.hh"
#include "src/bin/corba/MojeID.hh"
#include "src/bin/corba/NullableIsoDate.hh"
#include "src/bin/corba/mojeid/mojeid_corba_conversion.hh"
#include "src/bin/corba/util/corba_conversions_isodate.hh"
#include "src/bin/corba/util/corba_conversions_isodatetime.hh"
#include "src/bin/corba/util/corba_conversions_nullableisodate.hh"
#include "src/bin/corba/util/corba_conversions_nullableisodatetime.hh"

#include <boost/lexical_cast.hpp>

#include <string>

namespace CorbaConversion {

void unwrap_NullableString(const Registry::MojeID::NullableString* src_ptr, Nullable<std::string>& dst)
{
    if (src_ptr == NULL)
    {
        dst = Nullable<std::string>();
    }
    else
    {
        dst = src_ptr->_boxed_in();
    }
}

namespace {

//Fake NULL value from empty input string
void unwrap_NullableString_fix_frontend_bug(const Registry::MojeID::NullableString* src_ptr, Nullable<std::string>& dst)
{
    if (src_ptr == NULL)
    {
        dst = Nullable<std::string>();
    }
    else
    {
        const std::string dst_value = src_ptr->_boxed_in();
        if (dst_value.empty())
        {
            dst = Nullable<std::string>();
        }
        else
        {
            dst = dst_value;
        }
    }
}
}

Registry::MojeID::NullableString_var wrap_Nullable_string(const Nullable<std::string>& src)
{
    return src.isnull() ? Registry::MojeID::NullableString_var()
                        : Registry::MojeID::NullableString_var(
                                  new Registry::MojeID::NullableString(src.get_value().c_str()));
}

void unwrap_Date_to_Birthdate(const Registry::IsoDate& src, Fred::Backend::MojeIdImplData::Birthdate& dst)
{
    dst.value = src.value.in();
}

void unwrap_NullableIsoDate_to_Birthdate(const Registry::NullableIsoDate* src_ptr, Nullable<Fred::Backend::MojeIdImplData::Birthdate>& dst)
{
    if (src_ptr == NULL)
    {
        dst = Nullable<Fred::Backend::MojeIdImplData::Birthdate>();
    }
    else
    {
        Fred::Backend::MojeIdImplData::Birthdate result;
        unwrap_Date_to_Birthdate(src_ptr->_boxed_in(), result);
        dst = result;
    }
}

void wrap_Birthdate(const Fred::Backend::MojeIdImplData::Birthdate& src, Registry::IsoDate& dst)
{
    dst.value = wrap_string(src.value)._retn();
}

Registry::NullableIsoDate_var wrap_Nullable_Birthdate(const Nullable<Fred::Backend::MojeIdImplData::Birthdate>& src)
{
    if (src.isnull())
    {
        return Registry::NullableIsoDate_var();
    }
    Registry::NullableIsoDate_var result(new Registry::NullableIsoDate());
    wrap_Birthdate(src.get_value(), result.in()->_value());
    return result._retn();
}

void unwrap_Address(const Registry::MojeID::Address& src, Fred::Backend::MojeIdImplData::Address& dst)
{
    dst.street1 = src.street1.in();

    unwrap_NullableString(src.street2.in(), dst.street2);
    unwrap_NullableString(src.street3.in(), dst.street3);

    dst.city = src.city.in();

    unwrap_NullableString(src.state.in(), dst.state);

    dst.postal_code = src.postal_code.in();
    dst.country = src.country.in();
}

void unwrap_NullableAddress(const Registry::MojeID::NullableAddress* src_ptr, Nullable<Fred::Backend::MojeIdImplData::Address>& dst)
{
    if (src_ptr == NULL)
    {
        dst = Nullable<Fred::Backend::MojeIdImplData::Address>();
    }
    else
    {
        Fred::Backend::MojeIdImplData::Address result;
        unwrap_Address(src_ptr->_boxed_in(), result);
        dst = result;
    }
}

void wrap_Address(const Fred::Backend::MojeIdImplData::Address& src, Registry::MojeID::Address& dst)
{
    dst.street1 = wrap_string(src.street1)._retn();

    dst.street2 = wrap_Nullable_string(src.street2);
    dst.street3 = wrap_Nullable_string(src.street3);

    dst.city = wrap_string(src.city)._retn();

    dst.state = wrap_Nullable_string(src.state);

    dst.postal_code = wrap_string(src.postal_code)._retn();
    dst.country = wrap_string(src.country)._retn();
}

Registry::MojeID::NullableAddress_var wrap_Nullable_Address(const Nullable<Fred::Backend::MojeIdImplData::Address>& src)
{
    if (src.isnull())
    {
        return Registry::MojeID::NullableAddress_var();
    }
    Registry::MojeID::NullableAddress_var result(new Registry::MojeID::NullableAddress());
    wrap_Address(src.get_value(), result.in()->_value());
    return result._retn();
}

void unwrap_ShippingAddress(const Registry::MojeID::ShippingAddress& src, Fred::Backend::MojeIdImplData::ShippingAddress& dst)
{
    unwrap_NullableString(src.company_name.in(), dst.company_name);

    dst.street1 = src.street1.in();

    unwrap_NullableString(src.street2.in(), dst.street2);
    unwrap_NullableString(src.street3.in(), dst.street3);

    dst.city = src.city.in();

    unwrap_NullableString(src.state.in(), dst.state);

    dst.postal_code = src.postal_code.in();
    dst.country = src.country.in();
}

void unwrap_NullableShippingAddress(const Registry::MojeID::NullableShippingAddress* src_ptr, Nullable<Fred::Backend::MojeIdImplData::ShippingAddress>& dst)
{
    if (src_ptr == NULL)
    {
        dst = Nullable<Fred::Backend::MojeIdImplData::ShippingAddress>();
    }
    else
    {
        Fred::Backend::MojeIdImplData::ShippingAddress result;
        unwrap_ShippingAddress(src_ptr->_boxed_in(), result);
        dst = result;
    }
}

void wrap_ShippingAddress(const Fred::Backend::MojeIdImplData::ShippingAddress& src, Registry::MojeID::ShippingAddress& dst)
{
    dst.company_name = wrap_Nullable_string(src.company_name);

    dst.street1 = wrap_string(src.street1)._retn();

    dst.street2 = wrap_Nullable_string(src.street2);
    dst.street3 = wrap_Nullable_string(src.street3);

    dst.city = wrap_string(src.city)._retn();

    dst.state = wrap_Nullable_string(src.state);

    dst.postal_code = wrap_string(src.postal_code)._retn();
    dst.country = wrap_string(src.country)._retn();
}

Registry::MojeID::NullableShippingAddress_var wrap_Nullable_ShippingAddress(const Nullable<Fred::Backend::MojeIdImplData::ShippingAddress>& src)
{
    if (src.isnull())
    {
        return Registry::MojeID::NullableShippingAddress_var();
    }
    Registry::MojeID::NullableShippingAddress_var result(new Registry::MojeID::NullableShippingAddress());
    wrap_ShippingAddress(src.get_value(), result.in()->_value());
    return result._retn();
}

ArgumentIsSpecial::ArgumentIsSpecial()
    : std::invalid_argument("argument is special")
{
}

NotEnumValidationResultValue::NotEnumValidationResultValue()
    : std::invalid_argument("argument value is not from enum ValidationResult::Value set")
{
}

ValidationResultWasNotSet::ValidationResultWasNotSet()
    : std::invalid_argument("argument value was never set")
{
}

void wrap_ValidationResult(Fred::Backend::MojeIdImplData::ValidationResult::Value src, Registry::MojeID::ValidationResult& dst)
{
    switch (src)
    {
        case Fred::Backend::MojeIdImplData::ValidationResult::OK:
            dst = Registry::MojeID::OK;
            return;
        case Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE:
            dst = Registry::MojeID::NOT_AVAILABLE;
            return;
        case Fred::Backend::MojeIdImplData::ValidationResult::INVALID:
            dst = Registry::MojeID::INVALID;
            return;
        case Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED:
            dst = Registry::MojeID::REQUIRED;
            return;
        case Fred::Backend::MojeIdImplData::ValidationResult::UNKNOWN:
            throw ValidationResultWasNotSet();
    }
    throw NotEnumValidationResultValue();
}

void wrap_AddressValidationResult(const Fred::Backend::MojeIdImplData::AddressValidationResult& src,
        Registry::MojeID::AddressValidationResult& dst)
{
    wrap_ValidationResult(src.street1, dst.street1);
    wrap_ValidationResult(src.city, dst.city);
    wrap_ValidationResult(src.postal_code, dst.postal_code);
    wrap_ValidationResult(src.country, dst.country);
}

void wrap_MandatoryAddressValidationResult(const Fred::Backend::MojeIdImplData::MandatoryAddressValidationResult& src,
        Registry::MojeID::MandatoryAddressValidationResult& dst)
{
    wrap_ValidationResult(src.address_presence, dst.address_presence);
    wrap_ValidationResult(src.street1, dst.street1);
    wrap_ValidationResult(src.city, dst.city);
    wrap_ValidationResult(src.postal_code, dst.postal_code);
    wrap_ValidationResult(src.country, dst.country);
}

void wrap_RegistrationValidationResult(const Fred::Backend::MojeIdImplData::RegistrationValidationResult& src,
        Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR& dst)
{
    wrap_ValidationResult(src.username, dst.username);
    wrap_ValidationResult(src.name, dst.name);
    wrap_ValidationResult(src.birth_date, dst.birth_date);
    wrap_ValidationResult(src.vat_id_num, dst.vat_id_num);
    wrap_ValidationResult(src.email, dst.email);
    wrap_ValidationResult(src.notify_email, dst.notify_email);
    wrap_ValidationResult(src.phone, dst.phone);
    wrap_ValidationResult(src.fax, dst.fax);

    wrap_MandatoryAddressValidationResult(src.permanent, dst.permanent);

    wrap_AddressValidationResult(src.mailing, dst.mailing);
    wrap_AddressValidationResult(src.billing, dst.billing);
    wrap_AddressValidationResult(src.shipping, dst.shipping);
    wrap_AddressValidationResult(src.shipping2, dst.shipping2);
    wrap_AddressValidationResult(src.shipping3, dst.shipping3);
}

void wrap_UpdateContactPrepareValidationResult(const Fred::Backend::MojeIdImplData::UpdateContactPrepareValidationResult& src,
        Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR& dst)
{
    wrap_ValidationResult(src.name, dst.name);
    wrap_ValidationResult(src.birth_date, dst.birth_date);
    wrap_ValidationResult(src.email, dst.email);
    wrap_ValidationResult(src.notify_email, dst.notify_email);
    wrap_ValidationResult(src.phone, dst.phone);
    wrap_ValidationResult(src.fax, dst.fax);

    wrap_MandatoryAddressValidationResult(src.permanent, dst.permanent);

    wrap_AddressValidationResult(src.mailing, dst.mailing);
    wrap_AddressValidationResult(src.billing, dst.billing);
    wrap_AddressValidationResult(src.shipping, dst.shipping);
    wrap_AddressValidationResult(src.shipping2, dst.shipping2);
    wrap_AddressValidationResult(src.shipping3, dst.shipping3);
}

void wrap_UpdateValidatedContactPrepareValidationResult(
        const Fred::Backend::MojeIdImplData::UpdateValidatedContactPrepareValidationResult& src,
        Registry::MojeID::Server::UPDATE_VALIDATED_CONTACT_PREPARE_VALIDATION_ERROR& dst)
{
    wrap_ValidationResult(src.name, dst.name);
    wrap_ValidationResult(src.birth_date, dst.birth_date);
    wrap_MandatoryAddressValidationResult(src.permanent, dst.permanent);
}

void wrap_CreateValidationRequestValidationResult(const Fred::Backend::MojeIdImplData::CreateValidationRequestValidationResult& src,
        Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR& dst)
{
    wrap_ValidationResult(src.name, dst.name);

    wrap_MandatoryAddressValidationResult(src.permanent, dst.permanent);

    wrap_ValidationResult(src.email, dst.email);
    wrap_ValidationResult(src.phone, dst.phone);
    wrap_ValidationResult(src.notify_email, dst.notify_email);
    wrap_ValidationResult(src.fax, dst.fax);
    wrap_ValidationResult(src.birth_date, dst.birth_date);
    wrap_ValidationResult(src.vat_id_num, dst.vat_id_num);
}

void wrap_ProcessRegistrationValidationResult(const Fred::Backend::MojeIdImplData::ProcessRegistrationValidationResult& src,
        Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR& dst)
{
    wrap_ValidationResult(src.email, dst.email);
    wrap_ValidationResult(src.phone, dst.phone);
}

void unwrap_CreateContact(const Registry::MojeID::CreateContact& src, Fred::Backend::MojeIdImplData::CreateContact& dst)
{
    dst.username = src.username.in();
    dst.name = src.name.in();

    unwrap_NullableString(src.organization.in(), dst.organization);
    unwrap_NullableString(src.vat_reg_num.in(), dst.vat_reg_num);

    unwrap_NullableIsoDate_to_Birthdate(src.birth_date.in(), dst.birth_date);

    unwrap_NullableString_fix_frontend_bug(src.id_card_num.in(), dst.id_card_num);
    unwrap_NullableString_fix_frontend_bug(src.passport_num.in(), dst.passport_num);
    unwrap_NullableString_fix_frontend_bug(src.ssn_id_num.in(), dst.ssn_id_num);
    unwrap_NullableString_fix_frontend_bug(src.vat_id_num.in(), dst.vat_id_num);

    unwrap_Address(src.permanent, dst.permanent);

    unwrap_NullableAddress(src.mailing.in(), dst.mailing);
    unwrap_NullableAddress(src.billing.in(), dst.billing);

    unwrap_NullableShippingAddress(src.shipping.in(), dst.shipping);
    unwrap_NullableShippingAddress(src.shipping2.in(), dst.shipping2);
    unwrap_NullableShippingAddress(src.shipping3.in(), dst.shipping3);

    dst.email = src.email.in();

    unwrap_NullableString(src.notify_email.in(), dst.notify_email);

    dst.telephone = src.telephone.in();

    unwrap_NullableString(src.fax.in(), dst.fax);
}

void unwrap_UpdateTransferContact(const Registry::MojeID::UpdateTransferContact& src, Fred::Backend::MojeIdImplData::UpdateTransferContact& dst)
{
    dst.name = src.name.in();

    unwrap_NullableString(src.organization.in(), dst.organization);
    unwrap_NullableString(src.vat_reg_num.in(), dst.vat_reg_num);

    unwrap_NullableIsoDate_to_Birthdate(src.birth_date.in(), dst.birth_date);

    unwrap_NullableString_fix_frontend_bug(src.vat_id_num.in(), dst.vat_id_num);

    unwrap_Address(src.permanent, dst.permanent);

    unwrap_NullableAddress(src.mailing.in(), dst.mailing);

    dst.email = src.email.in();

    unwrap_NullableString(src.notify_email.in(), dst.notify_email);

    dst.telephone = src.telephone.in();

    unwrap_NullableString(src.fax.in(), dst.fax);
}

void unwrap_UpdateContact(const Registry::MojeID::UpdateContact& src, Fred::Backend::MojeIdImplData::UpdateContact& dst)
{
    dst.name = src.name.in();

    unwrap_NullableString(src.organization.in(), dst.organization);
    unwrap_NullableString(src.vat_reg_num.in(), dst.vat_reg_num);

    unwrap_NullableIsoDate_to_Birthdate(src.birth_date.in(), dst.birth_date);

    unwrap_NullableString_fix_frontend_bug(src.id_card_num.in(), dst.id_card_num);
    unwrap_NullableString_fix_frontend_bug(src.passport_num.in(), dst.passport_num);
    unwrap_NullableString_fix_frontend_bug(src.ssn_id_num.in(), dst.ssn_id_num);
    unwrap_NullableString_fix_frontend_bug(src.vat_id_num.in(), dst.vat_id_num);

    unwrap_Address(src.permanent, dst.permanent);

    unwrap_NullableAddress(src.mailing.in(), dst.mailing);
    unwrap_NullableAddress(src.billing.in(), dst.billing);

    unwrap_NullableShippingAddress(src.shipping.in(), dst.shipping);
    unwrap_NullableShippingAddress(src.shipping2.in(), dst.shipping2);
    unwrap_NullableShippingAddress(src.shipping3.in(), dst.shipping3);

    dst.email = src.email.in();

    unwrap_NullableString(src.notify_email.in(), dst.notify_email);
    unwrap_NullableString(src.telephone.in(), dst.telephone);
    unwrap_NullableString(src.fax.in(), dst.fax);
}

void unwrap_ValidatedContactData(
        const Registry::MojeID::ValidatedContactData& src,
        Fred::Backend::MojeIdImplData::ValidatedContactData& dst)
{
    dst.name = src.name.in();
    unwrap_Date_to_Birthdate(src.birth_date, dst.birth_date);
    unwrap_Address(src.permanent, dst.permanent);
}

Registry::MojeID::InfoContact_var wrap_InfoContact(const Fred::Backend::MojeIdImplData::InfoContact& src)
{
    Registry::MojeID::InfoContact_var result(new Registry::MojeID::InfoContact());

    int_to_int(src.id, result->id);

    result->name = wrap_string(src.name)._retn();

    result->organization = wrap_Nullable_string(src.organization);
    result->vat_reg_num = wrap_Nullable_string(src.vat_reg_num);

    result->birth_date = wrap_Nullable_Birthdate(src.birth_date)._retn();

    result->id_card_num = wrap_Nullable_string(src.id_card_num);
    result->passport_num = wrap_Nullable_string(src.passport_num);
    result->ssn_id_num = wrap_Nullable_string(src.ssn_id_num);
    result->vat_id_num = wrap_Nullable_string(src.vat_id_num);

    wrap_Address(src.permanent, result->permanent);

    result->mailing = wrap_Nullable_Address(src.mailing)._retn();
    result->billing = wrap_Nullable_Address(src.billing)._retn();

    result->shipping = wrap_Nullable_ShippingAddress(src.shipping)._retn();
    result->shipping2 = wrap_Nullable_ShippingAddress(src.shipping2)._retn();
    result->shipping3 = wrap_Nullable_ShippingAddress(src.shipping3)._retn();

    result->email = wrap_string(src.email)._retn();

    result->notify_email = wrap_Nullable_string(src.notify_email);
    result->telephone = wrap_Nullable_string(src.telephone);
    result->fax = wrap_Nullable_string(src.fax);

    return result._retn();
}

void wrap_InfoContactPublishFlags(const Fred::Backend::MojeIdImplData::InfoContactPublishFlags& src,
        Registry::MojeID::InfoContactPublishFlags& dst)
{
    int_to_int(src.name, dst.name);
    int_to_int(src.organization, dst.organization);
    int_to_int(src.vat_reg_num, dst.vat_reg_num);
    int_to_int(src.birth_date, dst.birth_date);
    int_to_int(src.id_card_num, dst.id_card_num);
    int_to_int(src.passport_num, dst.passport_num);
    int_to_int(src.ssn_id_num, dst.ssn_id_num);
    int_to_int(src.vat_id_num, dst.vat_id_num);
    int_to_int(src.email, dst.email);
    int_to_int(src.notify_email, dst.notify_email);
    int_to_int(src.telephone, dst.telephone);
    int_to_int(src.fax, dst.fax);
    int_to_int(src.permanent, dst.permanent);
    int_to_int(src.mailing, dst.mailing);
    int_to_int(src.billing, dst.billing);
    int_to_int(src.shipping, dst.shipping);
    int_to_int(src.shipping2, dst.shipping2);
    int_to_int(src.shipping3, dst.shipping3);
}

void wrap_ContactStateInfo(const Fred::Backend::MojeIdImplData::ContactStateInfo& src, Registry::MojeID::ContactStateInfo& dst)
{
    int_to_int(src.contact_id, dst.contact_id);

    dst.mojeid_activation_datetime = Util::wrap_boost_posix_time_ptime_to_IsoDateTime(src.mojeid_activation_datetime);

    dst.identification_date = Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(src.identification_date)._retn();
    dst.validation_date = Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(src.validation_date)._retn();
    dst.linked_date = Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(src.linked_date)._retn();
}

Registry::MojeID::ContactStateInfo_var wrap_ContactStateInfo(const Fred::Backend::MojeIdImplData::ContactStateInfo& src)
{
    Registry::MojeID::ContactStateInfo_var result(new Registry::MojeID::ContactStateInfo());
    wrap_ContactStateInfo(src, result.inout());
    return result._retn();
}

Registry::MojeID::ContactStateInfoList_var wrap_ContactStateInfoList(const Fred::Backend::MojeIdImplData::ContactStateInfoList& src)
{
    Registry::MojeID::ContactStateInfoList_var result(new Registry::MojeID::ContactStateInfoList());

    result->length(src.size());
    ::size_t dst_idx = 0;
    for (Fred::Backend::MojeIdImplData::ContactStateInfoList::const_iterator src_ptr = src.begin(); src_ptr != src.end();
            ++src_ptr, ++dst_idx)
    {
        wrap_ContactStateInfo(*src_ptr, result->operator[](dst_idx));
    }
    return result._retn();
}

Registry::MojeID::ContactHandleList_var wrap_ContactHandleList(const Fred::Backend::MojeIdImplData::ContactHandleList& src)
{
    Registry::MojeID::ContactHandleList_var result(new Registry::MojeID::ContactHandleList());

    result->length(src.size());
    ::size_t dst_idx = 0;
    for (Fred::Backend::MojeIdImplData::ContactHandleList::const_iterator src_ptr = src.begin(); src_ptr != src.end();
            ++src_ptr, ++dst_idx)
    {
        result->operator[](dst_idx) = wrap_string(*src_ptr)._retn();
    }
    return result._retn();
}

void wrap_MessageLimitExceeded(const Fred::Backend::MojeIdImplData::MessageLimitExceeded& src, Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED& dst)
{
    dst.limit_expire_datetime = Util::wrap_boost_posix_time_ptime_to_IsoDateTime(src.limit_expire_datetime);
}

void raise_REGISTRATION_VALIDATION_ERROR(const Fred::Backend::MojeIdImplData::RegistrationValidationResult& src)
{
    Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR e;
    try
    {
        wrap_RegistrationValidationResult(src, e);
    }
    catch (...)
    {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR(const Fred::Backend::MojeIdImplData::UpdateContactPrepareValidationResult& src)
{
    Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR e;
    try
    {
        wrap_UpdateContactPrepareValidationResult(src, e);
    }
    catch (...)
    {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_UPDATE_VALIDATED_CONTACT_PREPARE_VALIDATION_ERROR(
        const Fred::Backend::MojeIdImplData::UpdateValidatedContactPrepareValidationResult& src)
{
    Registry::MojeID::Server::UPDATE_VALIDATED_CONTACT_PREPARE_VALIDATION_ERROR e;
    try
    {
        wrap_UpdateValidatedContactPrepareValidationResult(src, e);
    }
    catch (...)
    {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_MESSAGE_LIMIT_EXCEEDED(const Fred::Backend::MojeIdImplData::MessageLimitExceeded& src)
{
    Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED e;
    try
    {
        wrap_MessageLimitExceeded(src, e);
    }
    catch (...)
    {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR(const Fred::Backend::MojeIdImplData::CreateValidationRequestValidationResult& src)
{
    Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR e;
    try
    {
        wrap_CreateValidationRequestValidationResult(src, e);
    }
    catch (...)
    {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_PROCESS_REGISTRATION_VALIDATION_ERROR(const Fred::Backend::MojeIdImplData::ProcessRegistrationValidationResult& src)
{
    Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR e;
    try
    {
        wrap_ProcessRegistrationValidationResult(src, e);
    }
    catch (...)
    {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

} // namespace CorbaConversion
