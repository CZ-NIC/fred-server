/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
 *  declaration for MojeID CORBA conversion
 */

#ifndef MOJEID_CORBA_CONVERSION_HH_02D1309914E14D58B682DF7CFD19A17A
#define MOJEID_CORBA_CONVERSION_HH_02D1309914E14D58B682DF7CFD19A17A

#include "src/backend/buffer.hh"
#include "src/backend/mojeid/mojeid_impl_data.hh"
#include "src/bin/corba/IsoDate.hh"
#include "src/bin/corba/MojeID.hh"
#include "src/bin/corba/NullableIsoDate.hh"
#include "src/util/corba_conversion.hh"
#include "util/db/nullable.hh"

namespace CorbaConversion {

void unwrap_NullableString(
        const Registry::MojeID::NullableString* src_ptr,
        Nullable<std::string>& dst);


Registry::MojeID::NullableString_var wrap_Nullable_string(const Nullable<std::string>& src);


void unwrap_Date_to_Birthdate(
        const Registry::IsoDate& src,
        Fred::Backend::MojeIdImplData::Birthdate& dst);


void unwrap_NullableIsoDate_to_Birthdate(
        const Registry::NullableIsoDate* src_ptr,
        Nullable<Fred::Backend::MojeIdImplData::Birthdate>& dst);


void wrap_boost_gregorian_date(
        const boost::gregorian::date& src,
        Registry::IsoDate& dst);


void wrap_Date(
        const Fred::Backend::MojeIdImplData::Birthdate& src,
        Registry::IsoDate& dst);


Registry::NullableIsoDate_var wrap_Nullable_Date(
        const Nullable<Fred::Backend::MojeIdImplData::Birthdate>& src);


Registry::NullableIsoDate_var wrap_Nullable_boost_gregorian_date(const Nullable<boost::gregorian::date>& src);


void unwrap_Address(
        const Registry::MojeID::Address& src,
        Fred::Backend::MojeIdImplData::Address& dst);


void unwrap_NullableAddress(
        const Registry::MojeID::NullableAddress* src_ptr,
        Nullable<Fred::Backend::MojeIdImplData::Address>& dst);


void wrap_Address(
        const Fred::Backend::MojeIdImplData::Address& src,
        Registry::MojeID::Address& dst);


Registry::MojeID::NullableAddress_var wrap_Nullable_Address(
        const Nullable<Fred::Backend::MojeIdImplData::Address>& src);


void unwrap_ShippingAddress(
        const Registry::MojeID::ShippingAddress& src,
        Fred::Backend::MojeIdImplData::ShippingAddress& dst);


void unwrap_NullableShippingAddress(
        const Registry::MojeID::NullableShippingAddress* src_ptr,
        Nullable<Fred::Backend::MojeIdImplData::ShippingAddress>& dst);


void wrap_ShippingAddress(
        const Fred::Backend::MojeIdImplData::ShippingAddress& src,
        Registry::MojeID::ShippingAddress& dst);


Registry::MojeID::NullableShippingAddress_var wrap_Nullable_ShippingAddress(
        const Nullable<Fred::Backend::MojeIdImplData::ShippingAddress>& src);


/**
 * Exception if argument is special
 */
struct ArgumentIsSpecial
    : std::invalid_argument
{


    ArgumentIsSpecial();


    virtual ~ArgumentIsSpecial()
    {
    }

};

/**
 * Exception if argument value is not enum ValidationResult value
 */
struct NotEnumValidationResultValue
    : std::invalid_argument
{


    NotEnumValidationResultValue();


    virtual ~NotEnumValidationResultValue()
    {
    }

};

/**
 * Exception if argument value indicate that its value was never set
 */
struct ValidationResultWasNotSet
    : std::invalid_argument
{


    ValidationResultWasNotSet();


    virtual ~ValidationResultWasNotSet()
    {
    }

};

void wrap_ValidationResult(
        Fred::Backend::MojeIdImplData::ValidationResult::Value src,
        Registry::MojeID::ValidationResult& dst);


void wrap_AddressValidationResult(
        const Fred::Backend::MojeIdImplData::AddressValidationResult& src,
        Registry::MojeID::AddressValidationResult& dst);


void wrap_MandatoryAddressValidationResult(
        const Fred::Backend::MojeIdImplData::MandatoryAddressValidationResult& src,
        Registry::MojeID::MandatoryAddressValidationResult& dst);


void wrap_RegistrationValidationResult(
        const Fred::Backend::MojeIdImplData::RegistrationValidationResult& src,
        Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR& dst);


void wrap_UpdateContactPrepareValidationResult(
        const Fred::Backend::MojeIdImplData::UpdateContactPrepareValidationResult& src,
        Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR& dst);


void wrap_CreateValidationRequestValidationResult(
        const Fred::Backend::MojeIdImplData::CreateValidationRequestValidationResult& src,
        Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR& dst);


void wrap_ProcessRegistrationValidationResult(
        const Fred::Backend::MojeIdImplData::ProcessRegistrationValidationResult& src,
        Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR& dst);


void unwrap_CreateContact(
        const Registry::MojeID::CreateContact& src,
        Fred::Backend::MojeIdImplData::CreateContact& dst);


void unwrap_UpdateContact(
        const Registry::MojeID::UpdateContact& src,
        Fred::Backend::MojeIdImplData::UpdateContact& dst);


void unwrap_UpdateTransferContact(
        const Registry::MojeID::UpdateTransferContact& src,
        Fred::Backend::MojeIdImplData::UpdateTransferContact& dst);


Registry::MojeID::InfoContact_var wrap_InfoContact(const Fred::Backend::MojeIdImplData::InfoContact& src);


void wrap_InfoContactPublishFlags(
        const Fred::Backend::MojeIdImplData::InfoContactPublishFlags& src,
        Registry::MojeID::InfoContactPublishFlags& dst);


void wrap_ContactStateInfo(
        const Fred::Backend::MojeIdImplData::ContactStateInfo& src,
        Registry::MojeID::ContactStateInfo& dst);


Registry::MojeID::ContactStateInfo_var wrap_ContactStateInfo(
        const Fred::Backend::MojeIdImplData::ContactStateInfo& src);


Registry::MojeID::ContactStateInfoList_var wrap_ContactStateInfoList(
        const Fred::Backend::MojeIdImplData::ContactStateInfoList& src);


Registry::MojeID::ContactHandleList_var wrap_ContactHandleList(
        const Fred::Backend::MojeIdImplData::ContactHandleList& src);


void wrap_MessageLimitExceeded(
        const Fred::Backend::MojeIdImplData::MessageLimitExceeded& src,
        Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED& dst);


void raise_REGISTRATION_VALIDATION_ERROR(
        const Fred::Backend::MojeIdImplData::RegistrationValidationResult& src);


void raise_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR(
        const Fred::Backend::MojeIdImplData::UpdateContactPrepareValidationResult& src);


void raise_MESSAGE_LIMIT_EXCEEDED(const Fred::Backend::MojeIdImplData::MessageLimitExceeded& src);


void raise_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR(
        const Fred::Backend::MojeIdImplData::CreateValidationRequestValidationResult& src);


void raise_PROCESS_REGISTRATION_VALIDATION_ERROR(
        const Fred::Backend::MojeIdImplData::ProcessRegistrationValidationResult& src);

} // namespace CorbaConversion

#endif
