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
#include "src/util/db/nullable.hh"

namespace CorbaConversion
{

void unwrap_NullableString(const Registry::MojeID::NullableString *src_ptr, Nullable< std::string > &dst);
Registry::MojeID::NullableString_var wrap_Nullable_string(const Nullable< std::string > &src);

void unwrap_Date(const Registry::IsoDate &src, Registry::MojeIDImplData::Birthdate &dst);
void unwrap_NullableIsoDate(const Registry::NullableIsoDate *src_ptr, Nullable< Registry::MojeIDImplData::Birthdate > &dst);
void wrap_boost_gregorian_date(const boost::gregorian::date &src, Registry::IsoDate &dst);
void wrap_Date(const Registry::MojeIDImplData::Birthdate &src, Registry::IsoDate &dst);
Registry::NullableIsoDate_var wrap_Nullable_Date(const Nullable< Registry::MojeIDImplData::Birthdate > &src);
Registry::NullableIsoDate_var wrap_Nullable_boost_gregorian_date(const Nullable< boost::gregorian::date > &src);

void unwrap_Address(const Registry::MojeID::Address &src, Registry::MojeIDImplData::Address &dst);
void unwrap_NullableAddress(const Registry::MojeID::NullableAddress *src_ptr, Nullable< Registry::MojeIDImplData::Address > &dst);
void wrap_Address(const Registry::MojeIDImplData::Address &src, Registry::MojeID::Address &dst);
Registry::MojeID::NullableAddress_var wrap_Nullable_Address(const Nullable< Registry::MojeIDImplData::Address > &src);

void unwrap_ShippingAddress(const Registry::MojeID::ShippingAddress &src, Registry::MojeIDImplData::ShippingAddress &dst);
void unwrap_NullableShippingAddress(const Registry::MojeID::NullableShippingAddress *src_ptr, Nullable< Registry::MojeIDImplData::ShippingAddress > &dst);
void wrap_ShippingAddress(const Registry::MojeIDImplData::ShippingAddress &src, Registry::MojeID::ShippingAddress &dst);
Registry::MojeID::NullableShippingAddress_var wrap_Nullable_ShippingAddress(const Nullable< Registry::MojeIDImplData::ShippingAddress > &src);

/**
 * Exception if argument is special
 */
struct ArgumentIsSpecial:std::invalid_argument
{
    ArgumentIsSpecial();
    virtual ~ArgumentIsSpecial() {}
};

void unwrap_DateTime(const Registry::IsoDateTime &src, boost::posix_time::ptime &dst);
Registry::IsoDateTime_var wrap_DateTime(const boost::posix_time::ptime &src);

/**
 * Exception if argument value is not enum ValidationResult value
 */
struct NotEnumValidationResultValue:std::invalid_argument
{
    NotEnumValidationResultValue();
    virtual ~NotEnumValidationResultValue() {}
};

/**
 * Exception if argument value indicate that its value was never set
 */
struct ValidationResultWasNotSet:std::invalid_argument
{
    ValidationResultWasNotSet();
    virtual ~ValidationResultWasNotSet() {}
};

void wrap_ValidationResult(Registry::MojeIDImplData::ValidationResult::Value src, Registry::MojeID::ValidationResult &dst);

void wrap_AddressValidationResult(const Registry::MojeIDImplData::AddressValidationResult &src,
                                  Registry::MojeID::AddressValidationResult &dst);

void wrap_MandatoryAddressValidationResult(const Registry::MojeIDImplData::MandatoryAddressValidationResult &src,
                                           Registry::MojeID::MandatoryAddressValidationResult &dst);

void wrap_RegistrationValidationResult(const Registry::MojeIDImplData::RegistrationValidationResult &src,
                                       Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR &dst);

void wrap_UpdateContactPrepareValidationResult(const Registry::MojeIDImplData::UpdateContactPrepareValidationResult &src,
                                               Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR &dst);

void wrap_CreateValidationRequestValidationResult(const Registry::MojeIDImplData::CreateValidationRequestValidationResult &src,
                                                  Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR &dst);

void wrap_ProcessRegistrationValidationResult(const Registry::MojeIDImplData::ProcessRegistrationValidationResult &src,
                                              Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR &dst);

void unwrap_CreateContact(const Registry::MojeID::CreateContact &src, Registry::MojeIDImplData::CreateContact &dst);

void unwrap_UpdateContact(const Registry::MojeID::UpdateContact &src, Registry::MojeIDImplData::UpdateContact &dst);

void unwrap_UpdateTransferContact(const Registry::MojeID::UpdateTransferContact &src, Registry::MojeIDImplData::UpdateTransferContact &dst);

Registry::MojeID::InfoContact_var wrap_InfoContact(const Registry::MojeIDImplData::InfoContact &src);

void wrap_InfoContactPublishFlags(const Registry::MojeIDImplData::InfoContactPublishFlags &src,
                                  Registry::MojeID::InfoContactPublishFlags &dst);

void wrap_ContactStateInfo(const Registry::MojeIDImplData::ContactStateInfo &src, Registry::MojeID::ContactStateInfo &dst);
Registry::MojeID::ContactStateInfo_var wrap_ContactStateInfo(const Registry::MojeIDImplData::ContactStateInfo &src);

Registry::MojeID::ContactStateInfoList_var wrap_ContactStateInfoList(const Registry::MojeIDImplData::ContactStateInfoList &src);

Registry::Buffer_var wrap_Buffer(const Fred::Backend::Buffer &src);

/**
 * Exception if allocbuf is unable to alocate memory
 */
struct AllocbufFailed:std::invalid_argument
{
    AllocbufFailed();
    virtual ~AllocbufFailed() {}
};

Registry::MojeID::ContactHandleList_var wrap_ContactHandleList(const Registry::MojeIDImplData::ContactHandleList &src);

void wrap_MessageLimitExceeded(const Registry::MojeIDImplData::MessageLimitExceeded &src, Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED &dst);

void raise_REGISTRATION_VALIDATION_ERROR(const Registry::MojeIDImplData::RegistrationValidationResult &src);
void raise_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR(const Registry::MojeIDImplData::UpdateContactPrepareValidationResult &src);
void raise_MESSAGE_LIMIT_EXCEEDED(const Registry::MojeIDImplData::MessageLimitExceeded &src);
void raise_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR(const Registry::MojeIDImplData::CreateValidationRequestValidationResult &src);
void raise_PROCESS_REGISTRATION_VALIDATION_ERROR(const Registry::MojeIDImplData::ProcessRegistrationValidationResult &src);

} // namespace CorbaConversion

#endif
