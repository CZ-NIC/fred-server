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
 *  implementation for MojeID CORBA conversion
 */
#include "src/corba/mojeid/mojeid_corba_conversion.h"
#include <string>
#include <boost/lexical_cast.hpp>

namespace CorbaConversion {

CORBA::String_var wrap_string(const std::string &src)
{
    return src.c_str();
}

void unwrap_NullableString(const Registry::MojeID::NullableString *src_ptr, Nullable< std::string > &dst)
{
    if (src_ptr == NULL) {
        dst = Nullable< std::string >();
    }
    else {
        dst = src_ptr->_boxed_in();
    }
}

Registry::MojeID::NullableString_var wrap_Nullable_string(const Nullable< std::string > &src)
{
    return src.isnull() ? Registry::MojeID::NullableString_var()
                        : Registry::MojeID::NullableString_var(
                              new Registry::MojeID::NullableString(src.get_value().c_str()));
}

void unwrap_Date(const Registry::MojeID::Date &src, Registry::MojeIDImplData::Date &dst)
{
    dst.value = src.value.in();
}

void unwrap_NullableDate(const Registry::MojeID::NullableDate *src_ptr, Nullable< Registry::MojeIDImplData::Date > &dst)
{
    if (src_ptr == NULL) {
        dst = Nullable< Registry::MojeIDImplData::Date >();
    }
    else {
        Registry::MojeIDImplData::Date result;
        unwrap_Date(src_ptr->_boxed_in(), result);
        dst = result;
    }
}

void wrap_boost_gregorian_date(const boost::gregorian::date &src, Registry::MojeID::Date &dst)
{
    if (src.is_special()) {
        throw ArgumentIsSpecial();
    }

    dst.value = wrap_string(boost::gregorian::to_iso_extended_string(src))._retn();
}

void wrap_Date(const Registry::MojeIDImplData::Date &src, Registry::MojeID::Date &dst)
{
    dst.value = wrap_string(src.value)._retn();
}

Registry::MojeID::NullableDate_var wrap_Nullable_Date(const Nullable< Registry::MojeIDImplData::Date > &src)
{
    if (src.isnull()) {
        return Registry::MojeID::NullableDate_var();
    }
    Registry::MojeID::NullableDate_var result(new Registry::MojeID::NullableDate());
    wrap_Date(src.get_value(), result.in()->_value());
    return result._retn();
}

Registry::MojeID::NullableDate_var wrap_Nullable_boost_gregorian_date(const Nullable< boost::gregorian::date > &src)
{
    if (src.isnull()) {
        return Registry::MojeID::NullableDate_var();
    }
    Registry::MojeID::NullableDate_var result(new Registry::MojeID::NullableDate());
    wrap_boost_gregorian_date(src.get_value(), result.in()->_value());
    return result._retn();
}

void unwrap_Address(const Registry::MojeID::Address &src, Registry::MojeIDImplData::Address &dst)
{
    dst.street1 = src.street1.in();

    unwrap_NullableString(src.street2.in(), dst.street2);
    unwrap_NullableString(src.street3.in(), dst.street3);

    dst.city = src.city.in();

    unwrap_NullableString(src.state.in(), dst.state);

    dst.postal_code = src.postal_code.in();
    dst.country     = src.country.in();
}

void unwrap_NullableAddress(const Registry::MojeID::NullableAddress *src_ptr, Nullable< Registry::MojeIDImplData::Address > &dst)
{
    if (src_ptr == NULL) {
        dst = Nullable< Registry::MojeIDImplData::Address >();
    }
    else {
        Registry::MojeIDImplData::Address result;
        unwrap_Address(src_ptr->_boxed_in(), result);
        dst = result;
    }
}

void wrap_Address(const Registry::MojeIDImplData::Address &src, Registry::MojeID::Address &dst)
{
    dst.street1 = wrap_string(src.street1)._retn();

    dst.street2 = wrap_Nullable_string(src.street2);
    dst.street3 = wrap_Nullable_string(src.street3);

    dst.city = wrap_string(src.city)._retn();

    dst.state = wrap_Nullable_string(src.state);

    dst.postal_code = wrap_string(src.postal_code)._retn();
    dst.country     = wrap_string(src.country)._retn();
}

Registry::MojeID::NullableAddress_var wrap_Nullable_Address(const Nullable< Registry::MojeIDImplData::Address > &src)
{
    if (src.isnull()) {
        return Registry::MojeID::NullableAddress_var();
    }
    Registry::MojeID::NullableAddress_var result(new Registry::MojeID::NullableAddress());
    wrap_Address(src.get_value(), result.in()->_value());
    return result._retn();
}

void unwrap_ShippingAddress(const Registry::MojeID::ShippingAddress &src, Registry::MojeIDImplData::ShippingAddress &dst)
{
    unwrap_NullableString(src.company_name.in(), dst.company_name);

    dst.street1 = src.street1.in();

    unwrap_NullableString(src.street2.in(), dst.street2);
    unwrap_NullableString(src.street3.in(), dst.street3);

    dst.city = src.city.in();

    unwrap_NullableString(src.state.in(), dst.state);

    dst.postal_code = src.postal_code.in();
    dst.country     = src.country.in();
}

void unwrap_NullableShippingAddress(const Registry::MojeID::NullableShippingAddress *src_ptr, Nullable< Registry::MojeIDImplData::ShippingAddress > &dst)
{
    if (src_ptr == NULL) {
        dst = Nullable< Registry::MojeIDImplData::ShippingAddress >();
    }
    else {
        Registry::MojeIDImplData::ShippingAddress result;
        unwrap_ShippingAddress(src_ptr->_boxed_in(), result);
        dst = result;
    }
}

void wrap_ShippingAddress(const Registry::MojeIDImplData::ShippingAddress &src, Registry::MojeID::ShippingAddress &dst)
{
    dst.company_name = wrap_Nullable_string(src.company_name);

    dst.street1 = wrap_string(src.street1)._retn();

    dst.street2 = wrap_Nullable_string(src.street2);
    dst.street3 = wrap_Nullable_string(src.street3);

    dst.city = wrap_string(src.city)._retn();

    dst.state = wrap_Nullable_string(src.state);

    dst.postal_code = wrap_string(src.postal_code)._retn();
    dst.country     = wrap_string(src.country)._retn();
}

Registry::MojeID::NullableShippingAddress_var wrap_Nullable_ShippingAddress(const Nullable< Registry::MojeIDImplData::ShippingAddress > &src)
{
    if (src.isnull()) {
        return Registry::MojeID::NullableShippingAddress_var();
    }
    Registry::MojeID::NullableShippingAddress_var result(new Registry::MojeID::NullableShippingAddress());
    wrap_ShippingAddress(src.get_value(), result.in()->_value());
    return result._retn();
}

ArgumentIsSpecial::ArgumentIsSpecial()
:   std::invalid_argument("argument is special")
{
}

void unwrap_DateTime(const Registry::MojeID::DateTime &src, boost::posix_time::ptime &dst)
{
    const std::string value = src.value.in();
    try {
        dst = boost::date_time::parse_delimited_time< ptime >(value, 'T');
    }
    catch (const boost::bad_lexical_cast &e) {//special values usually lead to bad_lexical_cast exception
        throw ArgumentIsSpecial();//really special :-)
    }
    if (dst.is_special()) {
        throw ArgumentIsSpecial();
    }
}

Registry::MojeID::DateTime_var wrap_DateTime(const boost::posix_time::ptime &src)
{
    if (src.is_special()) {
        throw ArgumentIsSpecial();
    }

    Registry::MojeID::DateTime_var result(new Registry::MojeID::DateTime());
    result.inout().value = wrap_string(boost::posix_time::to_iso_extended_string(src))._retn();
    return result._retn();
}

NotEnumValidationResultValue::NotEnumValidationResultValue()
:   std::invalid_argument("argument value is not from enum ValidationResult::Value set")
{
}

void wrap_ValidationResult(Registry::MojeIDImplData::ValidationResult::Value src, Registry::MojeID::ValidationResult &dst)
{
    switch (src) {
    case Registry::MojeIDImplData::ValidationResult::OK:
        dst = Registry::MojeID::OK;
        return;
    case Registry::MojeIDImplData::ValidationResult::NOT_AVAILABLE:
        dst = Registry::MojeID::NOT_AVAILABLE;
        return;
    case Registry::MojeIDImplData::ValidationResult::INVALID:
        dst = Registry::MojeID::INVALID;
        return;
    case Registry::MojeIDImplData::ValidationResult::REQUIRED:
        dst = Registry::MojeID::REQUIRED;
        return;
    }
    throw NotEnumValidationResultValue();
}

void wrap_AddressValidationResult(const Registry::MojeIDImplData::AddressValidationResult &src,
                                  Registry::MojeID::AddressValidationResult &dst)
{
    wrap_ValidationResult(src.street1,     dst.street1);
    wrap_ValidationResult(src.city,        dst.city);
    wrap_ValidationResult(src.postal_code, dst.postal_code);
    wrap_ValidationResult(src.country,     dst.country);
}

void wrap_MandatoryAddressValidationResult(const Registry::MojeIDImplData::MandatoryAddressValidationResult &src,
                                           Registry::MojeID::MandatoryAddressValidationResult &dst)
{
    wrap_ValidationResult(src.address_presence, dst.address_presence);
    wrap_ValidationResult(src.street1,          dst.street1);
    wrap_ValidationResult(src.city,             dst.city);
    wrap_ValidationResult(src.postal_code,      dst.postal_code);
    wrap_ValidationResult(src.country,          dst.country);
}

void wrap_RegistrationValidationResult(const Registry::MojeIDImplData::RegistrationValidationResult &src,
                                       Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR &dst)
{
    wrap_ValidationResult(src.username,     dst.username);
    wrap_ValidationResult(src.first_name,   dst.first_name);
    wrap_ValidationResult(src.last_name,    dst.last_name);
    wrap_ValidationResult(src.birth_date,   dst.birth_date);
    wrap_ValidationResult(src.email,        dst.email);
    wrap_ValidationResult(src.notify_email, dst.notify_email);
    wrap_ValidationResult(src.phone,        dst.phone);
    wrap_ValidationResult(src.fax,          dst.fax);

    wrap_MandatoryAddressValidationResult(src.permanent, dst.permanent);

    wrap_AddressValidationResult(src.mailing,   dst.mailing);
    wrap_AddressValidationResult(src.billing,   dst.billing);
    wrap_AddressValidationResult(src.shipping,  dst.shipping);
    wrap_AddressValidationResult(src.shipping2, dst.shipping2);
    wrap_AddressValidationResult(src.shipping3, dst.shipping3);
}

void wrap_UpdateContactPrepareValidationResult(const Registry::MojeIDImplData::UpdateContactPrepareValidationResult &src,
                                               Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR &dst)
{
    wrap_ValidationResult(src.first_name,   dst.first_name);
    wrap_ValidationResult(src.last_name,    dst.last_name);
    wrap_ValidationResult(src.birth_date,   dst.birth_date);
    wrap_ValidationResult(src.email,        dst.email);
    wrap_ValidationResult(src.notify_email, dst.notify_email);
    wrap_ValidationResult(src.phone,        dst.phone);
    wrap_ValidationResult(src.fax,          dst.fax);

    wrap_MandatoryAddressValidationResult(src.permanent, dst.permanent);

    wrap_AddressValidationResult(src.mailing,   dst.mailing);
    wrap_AddressValidationResult(src.billing,   dst.billing);
    wrap_AddressValidationResult(src.shipping,  dst.shipping);
    wrap_AddressValidationResult(src.shipping2, dst.shipping2);
    wrap_AddressValidationResult(src.shipping3, dst.shipping3);
}

void wrap_CreateValidationRequestValidationResult(const Registry::MojeIDImplData::CreateValidationRequestValidationResult &src,
                                                  Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR &dst)
{
    wrap_ValidationResult(src.first_name, dst.first_name);
    wrap_ValidationResult(src.last_name,  dst.last_name);

    wrap_MandatoryAddressValidationResult(src.permanent, dst.permanent);

    wrap_ValidationResult(src.email,        dst.email);
    wrap_ValidationResult(src.phone,        dst.phone);
    wrap_ValidationResult(src.notify_email, dst.notify_email);
    wrap_ValidationResult(src.fax,          dst.fax);
    wrap_ValidationResult(src.birth_date,   dst.birth_date);
    wrap_ValidationResult(src.vat_id_num,   dst.vat_id_num);
}

void wrap_ProcessRegistrationValidationResult(const Registry::MojeIDImplData::ProcessRegistrationValidationResult &src,
                                              Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR &dst)
{
    wrap_ValidationResult(src.email, dst.email);
    wrap_ValidationResult(src.phone, dst.phone);
}

void unwrap_CreateContact(const Registry::MojeID::CreateContact &src, Registry::MojeIDImplData::CreateContact &dst)
{
    dst.username   = src.username.in();
    dst.first_name = src.first_name.in();
    dst.last_name  = src.last_name.in();

    unwrap_NullableString(src.organization.in(), dst.organization);
    unwrap_NullableString(src.vat_reg_num.in(),  dst.vat_reg_num);

    unwrap_NullableDate(src.birth_date.in(), dst.birth_date);

    unwrap_NullableString(src.id_card_num.in(),  dst.id_card_num);
    unwrap_NullableString(src.passport_num.in(), dst.passport_num);
    unwrap_NullableString(src.ssn_id_num.in(),   dst.ssn_id_num);
    unwrap_NullableString(src.vat_id_num.in(),   dst.vat_id_num);

    unwrap_Address(src.permanent, dst.permanent);

    unwrap_NullableAddress(src.mailing.in(), dst.mailing);
    unwrap_NullableAddress(src.billing.in(), dst.billing);

    unwrap_NullableShippingAddress(src.shipping.in(),  dst.shipping);
    unwrap_NullableShippingAddress(src.shipping2.in(), dst.shipping2);
    unwrap_NullableShippingAddress(src.shipping3.in(), dst.shipping3);

    dst.email = src.email.in();

    unwrap_NullableString(src.notify_email.in(), dst.notify_email);

    dst.telephone = src.telephone.in();

    unwrap_NullableString(src.fax.in(), dst.fax);
}

void unwrap_UpdateTransferContact(const Registry::MojeID::UpdateTransferContact &src, Registry::MojeIDImplData::UpdateTransferContact &dst)
{
    unwrap_NullableString(src.organization.in(), dst.organization);
    unwrap_NullableString(src.vat_reg_num.in(),  dst.vat_reg_num);

    unwrap_NullableDate(src.birth_date.in(), dst.birth_date);

    unwrap_NullableString(src.vat_id_num.in(), dst.vat_id_num);

    unwrap_Address(src.permanent, dst.permanent);

    unwrap_NullableAddress(src.mailing.in(), dst.mailing);

    dst.email = src.email.in();

    unwrap_NullableString(src.notify_email.in(), dst.notify_email);

    dst.telephone = src.telephone.in();

    unwrap_NullableString(src.fax.in(), dst.fax);
}

void unwrap_UpdateContact(const Registry::MojeID::UpdateContact &src, Registry::MojeIDImplData::UpdateContact &dst)
{
    dst.first_name = src.first_name.in();
    dst.last_name  = src.last_name.in();

    unwrap_NullableString(src.organization.in(), dst.organization);
    unwrap_NullableString(src.vat_reg_num.in(),  dst.vat_reg_num);

    unwrap_NullableDate(src.birth_date.in(), dst.birth_date);

    unwrap_NullableString(src.id_card_num.in(),  dst.id_card_num);
    unwrap_NullableString(src.passport_num.in(), dst.passport_num);
    unwrap_NullableString(src.ssn_id_num.in(),   dst.ssn_id_num);
    unwrap_NullableString(src.vat_id_num.in(),   dst.vat_id_num);

    unwrap_Address(src.permanent, dst.permanent);

    unwrap_NullableAddress(src.mailing.in(), dst.mailing);
    unwrap_NullableAddress(src.billing.in(), dst.billing);

    unwrap_NullableShippingAddress(src.shipping.in(),  dst.shipping);
    unwrap_NullableShippingAddress(src.shipping2.in(), dst.shipping2);
    unwrap_NullableShippingAddress(src.shipping3.in(), dst.shipping3);

    dst.email = src.email.in();

    unwrap_NullableString(src.notify_email.in(), dst.notify_email);
    unwrap_NullableString(src.telephone.in(),    dst.telephone);
    unwrap_NullableString(src.fax.in(),          dst.fax);
}

Registry::MojeID::InfoContact_var wrap_InfoContact(const Registry::MojeIDImplData::InfoContact &src)
{
    Registry::MojeID::InfoContact_var result(new Registry::MojeID::InfoContact());

    int_to_int(src.id, result->id);

    result->first_name = wrap_string(src.first_name)._retn();
    result->last_name  = wrap_string(src.last_name)._retn();

    result->organization = wrap_Nullable_string(src.organization);
    result->vat_reg_num  = wrap_Nullable_string(src.vat_reg_num);

    result->birth_date = wrap_Nullable_Date(src.birth_date)._retn();

    result->id_card_num  = wrap_Nullable_string(src.id_card_num);
    result->passport_num = wrap_Nullable_string(src.passport_num);
    result->ssn_id_num   = wrap_Nullable_string(src.ssn_id_num);
    result->vat_id_num   = wrap_Nullable_string(src.vat_id_num);

    wrap_Address(src.permanent, result->permanent);

    result->mailing = wrap_Nullable_Address(src.mailing)._retn();
    result->billing = wrap_Nullable_Address(src.billing)._retn();

    result->shipping  = wrap_Nullable_ShippingAddress(src.shipping)._retn();
    result->shipping2 = wrap_Nullable_ShippingAddress(src.shipping2)._retn();
    result->shipping3 = wrap_Nullable_ShippingAddress(src.shipping3)._retn();

    result->email = wrap_string(src.email)._retn();

    result->notify_email = wrap_Nullable_string(src.notify_email);
    result->telephone    = wrap_Nullable_string(src.telephone);
    result->fax          = wrap_Nullable_string(src.fax);

    return result._retn();
}

void wrap_ContactStateInfo(const Registry::MojeIDImplData::ContactStateInfo &src, Registry::MojeID::ContactStateInfo &dst)
{
    int_to_int(src.contact_id, dst.contact_id);

    dst.mojeid_activation_datetime = wrap_DateTime(src.mojeid_activation_datetime);

    dst.identification_date = wrap_Nullable_boost_gregorian_date(src.identification_date)._retn();
    dst.validation_date     = wrap_Nullable_boost_gregorian_date(src.validation_date)._retn();
    dst.linked_date         = wrap_Nullable_boost_gregorian_date(src.linked_date)._retn();
}

Registry::MojeID::ContactStateInfo_var wrap_ContactStateInfo(const Registry::MojeIDImplData::ContactStateInfo &src)
{
    Registry::MojeID::ContactStateInfo_var result(new Registry::MojeID::ContactStateInfo());
    wrap_ContactStateInfo(src, result.inout());
    return result._retn();
}

Registry::MojeID::ContactStateInfoList_var wrap_ContactStateInfoList(const Registry::MojeIDImplData::ContactStateInfoList &src)
{
    Registry::MojeID::ContactStateInfoList_var result(new Registry::MojeID::ContactStateInfoList());

    result->length(src.size());
    ::size_t dst_idx = 0;
    for (Registry::MojeIDImplData::ContactStateInfoList::const_iterator src_ptr = src.begin() ; src_ptr != src.end();
         ++src_ptr, ++dst_idx)
    {
        wrap_ContactStateInfo(*src_ptr, result->operator[](dst_idx));
    }
    return result._retn();
}

Registry::MojeID::Buffer_var wrap_Buffer(const Registry::MojeIDImplData::Buffer &src)
{
    Registry::MojeID::Buffer_var result(new Registry::MojeID::Buffer());
    wrap_string(src.value, result->value);
    return result._retn();
}

AllocbufFailed::AllocbufFailed()
:   std::invalid_argument("cannot allocate the requested memory")
{
}

void wrap_string(const std::string &src, Registry::MojeID::BufferValue &dst)
{
    try {
        dst.length(src.size());
        if (!src.empty()) {
            std::memcpy(dst.get_buffer(), &(src[0]), src.size());
        }
    }
    catch (...) {
        throw AllocbufFailed();
    }
}

Registry::MojeID::ContactHandleList_var wrap_ContactHandleList(const Registry::MojeIDImplData::ContactHandleList &src)
{
    Registry::MojeID::ContactHandleList_var result(new Registry::MojeID::ContactHandleList());

    result->length(src.size());
    ::size_t dst_idx = 0;
    for (Registry::MojeIDImplData::ContactHandleList::const_iterator src_ptr = src.begin() ; src_ptr != src.end();
         ++src_ptr, ++dst_idx)
    {
        result->operator[](dst_idx) = wrap_string(*src_ptr)._retn();
    }
    return result._retn();
}

void wrap_MessageLimitExceeded(const Registry::MojeIDImplData::MessageLimitExceeded &src, Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED &dst)
{
    dst.limit_expire_datetime = wrap_DateTime(src.limit_expire_datetime);
}

void raise_REGISTRATION_VALIDATION_ERROR(const Registry::MojeIDImplData::RegistrationValidationResult &src)
{
    Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR e;
    try {
        wrap_RegistrationValidationResult(src, e);
    }
    catch (...) {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR(const Registry::MojeIDImplData::UpdateContactPrepareValidationResult &src)
{
    Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR e;
    try {
        wrap_UpdateContactPrepareValidationResult(src, e);
    }
    catch (...) {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_MESSAGE_LIMIT_EXCEEDED(const Registry::MojeIDImplData::MessageLimitExceeded &src)
{
    Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED e;
    try {
        wrap_MessageLimitExceeded(src, e);
    }
    catch (...) {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

void raise_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR(const Registry::MojeIDImplData::CreateValidationRequestValidationResult &src)
{
    Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR e;
    try {
        wrap_CreateValidationRequestValidationResult(src, e);
    }
    catch (...) {
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

}//namespace CorbaConversion