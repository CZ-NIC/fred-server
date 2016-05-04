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
 *  implementation of mojeid internals
 */

#include "src/mojeid/mojeid_impl_internal.h"
#include "src/mojeid/mojeid_impl_data.h"
#include "src/corba/MojeID.hh"

namespace Registry {
namespace MojeIDImplInternal {

namespace {

template < class EXCEPTION_CLASS >
void set_optional_address_validation_result(
    const Fred::GeneralCheck::contact_optional_address &result,
    EXCEPTION_CLASS &e)
{
    if (result.success()) {
        e.set(MojeIDImplData::ValidationResult::OK);
        return;
    }

    if (result.street1_absent) {
        e.street1 = MojeIDImplData::ValidationResult::REQUIRED;
    }

    if (result.city_absent) {
        e.city = MojeIDImplData::ValidationResult::REQUIRED;
    }

    if (result.postalcode_absent) {
        e.postal_code = MojeIDImplData::ValidationResult::REQUIRED;
    }

    if (result.country_absent) {
        e.country = MojeIDImplData::ValidationResult::REQUIRED;
    }
}

template < class EXCEPTION_CLASS >
void set_optional_addresses_validation_result(
    const check_contact_optional_addresses &result,
    EXCEPTION_CLASS &e)
{
    set_optional_address_validation_result(
        static_cast< const Fred::check_contact_addresses_mailing&   >(result), e.mailing);
    set_optional_address_validation_result(
        static_cast< const Fred::check_contact_addresses_billing&   >(result), e.billing);
    set_optional_address_validation_result(
        static_cast< const Fred::check_contact_addresses_shipping&  >(result), e.shipping);
    set_optional_address_validation_result(
        static_cast< const Fred::check_contact_addresses_shipping2& >(result), e.shipping2);
    set_optional_address_validation_result(
        static_cast< const Fred::check_contact_addresses_shipping3& >(result), e.shipping3);
}

template < class EXCEPTION_CLASS >
void set_permanent_address_validation_result(
    const Fred::check_contact_place_address &result,
    EXCEPTION_CLASS &e)
{
    e.permanent.address_presence = result.absent ? MojeIDImplData::ValidationResult::REQUIRED
                                                 : MojeIDImplData::ValidationResult::OK;

    set_optional_address_validation_result(result, e.permanent);
}

template < class PRESENCE_ANCESTRAL_CLASS,
           class VALIDITY_ANCESTRAL_CLASS,
           class AVAILABILITY_ANCESTRAL_CLASS, class CHECK_CLASS >
void set_presence_validity_availability_result(
    const CHECK_CLASS &check,
    MojeIDImplData::ValidationResult::Value &result)
{
    if (!check.PRESENCE_ANCESTRAL_CLASS::success()) {
        result = MojeIDImplData::ValidationResult::REQUIRED;
    }
    else if (!check.VALIDITY_ANCESTRAL_CLASS::success()) {
        result = MojeIDImplData::ValidationResult::INVALID;
    }
    else if (!check.AVAILABILITY_ANCESTRAL_CLASS::success()) {
        result = MojeIDImplData::ValidationResult::NOT_AVAILABLE;
    }
    else {
        result = MojeIDImplData::ValidationResult::OK;
    }
}

template < class PRESENCE_ANCESTRAL_CLASS,
           class VALIDITY_ANCESTRAL_CLASS, class CHECK_CLASS >
void set_presence_validity_result(
    const CHECK_CLASS &check,
    MojeIDImplData::ValidationResult::Value &result)
{
    if (!check.PRESENCE_ANCESTRAL_CLASS::success()) {
        result = MojeIDImplData::ValidationResult::REQUIRED;
    }
    else if (!check.VALIDITY_ANCESTRAL_CLASS::success()) {
        result = MojeIDImplData::ValidationResult::INVALID;
    }
    else {
        result = MojeIDImplData::ValidationResult::OK;
    }
}

template < class VALIDITY_ANCESTRAL_CLASS,
           class AVAILABILITY_ANCESTRAL_CLASS, class CHECK_CLASS >
void set_validity_availability_result(
    const CHECK_CLASS &check,
    MojeIDImplData::ValidationResult::Value &result)
{
    if (!check.VALIDITY_ANCESTRAL_CLASS::success()) {
        result = MojeIDImplData::ValidationResult::INVALID;
    }
    else if (!check.AVAILABILITY_ANCESTRAL_CLASS::success()) {
        result = MojeIDImplData::ValidationResult::NOT_AVAILABLE;
    }
    else {
        result = MojeIDImplData::ValidationResult::OK;
    }
}

template < class CHECK_CLASS, class EXCEPTION_CLASS >
void set_contact_name_result(const CHECK_CLASS &result, EXCEPTION_CLASS &e)
{
    e.first_name = result.Fred::check_contact_name::first_name_absent
                   ? MojeIDImplData::ValidationResult::REQUIRED
                   : MojeIDImplData::ValidationResult::OK;

    e.last_name = result.Fred::check_contact_name::last_name_absent
                   ? MojeIDImplData::ValidationResult::REQUIRED
                   : MojeIDImplData::ValidationResult::OK;
}

void set_validity_result(bool valid, MojeIDImplData::ValidationResult::Value &result)
{
    result = valid ? MojeIDImplData::ValidationResult::OK
                   : MojeIDImplData::ValidationResult::INVALID;
}

void set_availability_result(bool available, MojeIDImplData::ValidationResult::Value &result)
{
    result = available ? MojeIDImplData::ValidationResult::OK
                       : MojeIDImplData::ValidationResult::NOT_AVAILABLE;
}

template < class EXCEPTION_CLASS >
void set_ssn_result(const Fred::MojeID::check_contact_ssn &result, EXCEPTION_CLASS &e)
{
    if (result.birthdate_absent) {
        e.birth_date = MojeIDImplData::ValidationResult::REQUIRED;
    }
    else if (result.birthdate_invalid) {
        e.birth_date = MojeIDImplData::ValidationResult::INVALID;
    }
    else {
        e.birth_date = MojeIDImplData::ValidationResult::OK;
    }

    if (result.vat_id_num_absent) {
        e.vat_id_num = MojeIDImplData::ValidationResult::REQUIRED;
    }
    else {
        e.vat_id_num = MojeIDImplData::ValidationResult::OK;
    }
}

}//namespace Registry::MojeIDImplInternal::{anonymous}

void raise(const CheckMojeIDRegistration &result)
{
    MojeIDImplData::RegistrationValidationResult e;

    set_validity_result(!result.Fred::MojeID::check_contact_username::invalid, e.username);

    set_contact_name_result(result, e);

    set_validity_result(result.Fred::MojeID::check_contact_birthday_validity::success(), e.birth_date);
    e.vat_id_num = MojeIDImplData::ValidationResult::OK;

    set_presence_validity_availability_result<
        Fred::check_contact_email_presence,
        Fred::check_contact_email_validity,
        Fred::check_contact_email_availability >(result, e.email);

    set_validity_result(result.Fred::check_contact_notifyemail_validity::success(), e.notify_email);

    set_presence_validity_availability_result<
        Fred::check_contact_phone_presence,
        Fred::check_contact_phone_validity,
        Fred::check_contact_phone_availability >(result, e.phone);

    set_validity_result(result.Fred::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckTransferContactPrepareStates &result)
{
    if (result.Fred::MojeID::Check::states_before_transfer_into_mojeid::mojeid_contact_present) {
        throw MojeIDImplData::AlreadyMojeidContact();
    }
    if (result.Fred::MojeID::Check::states_before_transfer_into_mojeid::server_admin_blocked) {
        throw MojeIDImplData::ObjectAdminBlocked();
    }
    if (result.Fred::MojeID::Check::states_before_transfer_into_mojeid::server_user_blocked) {
        throw MojeIDImplData::ObjectUserBlocked();
    }
}

void raise(const CheckCreateContactPrepare &result)
{
    MojeIDImplData::RegistrationValidationResult e;

    if (!result.Fred::MojeID::check_contact_username_availability::success()) {
        e.username = MojeIDImplData::ValidationResult::NOT_AVAILABLE;
    }
    else if (result.Fred::MojeID::check_contact_username::absent) {
        e.username = MojeIDImplData::ValidationResult::REQUIRED;
    }
    else if (result.Fred::MojeID::check_contact_username::invalid) {
        e.username = MojeIDImplData::ValidationResult::INVALID;
    }
    else {
        e.username = MojeIDImplData::ValidationResult::OK;
    }

    set_contact_name_result(result, e);

    set_validity_result(result.Fred::MojeID::check_contact_birthday_validity::success(), e.birth_date);
    e.vat_id_num = MojeIDImplData::ValidationResult::OK;

    set_presence_validity_availability_result<
        Fred::check_contact_email_presence,
        Fred::check_contact_email_validity,
        Fred::check_contact_email_availability >(result, e.email);

    set_validity_result(result.Fred::check_contact_notifyemail_validity::success(), e.notify_email);

    set_presence_validity_availability_result<
        Fred::check_contact_phone_presence,
        Fred::check_contact_phone_validity,
        Fred::check_contact_phone_availability >(result, e.phone);

    set_validity_result(result.Fred::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckUpdateContactPrepare &result)
{
    MojeIDImplData::UpdateContactPrepareValidationResult e;

    set_contact_name_result(result, e);

    set_validity_result(result.Fred::MojeID::check_contact_birthday_validity::success(), e.birth_date);

    set_presence_validity_result<
        Fred::check_contact_email_presence,
        Fred::check_contact_email_validity >(result, e.email);

    set_validity_result(result.Fred::check_contact_notifyemail_validity::success(), e.notify_email);

    set_validity_result(result.Fred::check_contact_phone_validity::success(), e.phone);

    set_validity_result(result.Fred::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckCreateValidationRequest &result)
{
    MojeIDImplData::CreateValidationRequestValidationResult e;

    set_contact_name_result(result, e);

    set_permanent_address_validation_result(result, e);

    set_presence_validity_result<
        Fred::check_contact_email_presence,
        Fred::check_contact_email_validity >(result, e.email);

    set_validity_result(result.Fred::check_contact_phone_validity::success(), e.phone);

    set_validity_result(result.Fred::check_contact_notifyemail_validity::success(), e.notify_email);

    set_validity_result(result.Fred::check_contact_fax_validity::success(), e.fax);

    set_ssn_result(result, e);

    throw e;
}

void raise(const CheckUpdateTransferContactPrepare &result)
{
    MojeIDImplData::RegistrationValidationResult e;

    set_validity_result(!result.Fred::MojeID::check_contact_username::invalid, e.username);

    set_contact_name_result(result, e);

    set_ssn_result(result, e);

    set_presence_validity_result<
        Fred::check_contact_email_presence,
        Fred::check_contact_email_validity >(result, e.email);

    set_validity_result(result.Fred::check_contact_notifyemail_validity::success(), e.notify_email);

    set_presence_validity_result<
        Fred::check_contact_phone_presence,
        Fred::check_contact_phone_validity >(result, e.phone);

    set_validity_result(result.Fred::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckProcessRegistrationValidation &result)
{
    MojeIDImplData::ProcessRegistrationValidationResult e;

    set_presence_validity_result<
        Fred::check_contact_email_presence,
        Fred::check_contact_email_validity >(result, e.email);

    set_presence_validity_result<
        Fred::check_contact_phone_presence,
        Fred::check_contact_phone_validity >(result, e.phone);

    throw e;
}

}//namespace Registry::MojeIDImplInternal
}//namespace Registry
