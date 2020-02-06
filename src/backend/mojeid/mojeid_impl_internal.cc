/*
 * Copyright (C) 2016-2020  CZ.NIC, z. s. p. o.
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
 *  implementation of mojeid internals
 */

#include "src/backend/mojeid/mojeid_impl_internal.hh"
#include "src/backend/mojeid/mojeid_impl_data.hh"
#include "src/bin/corba/MojeID.hh"

namespace Fred {
namespace Backend {
namespace MojeIdImplInternal {

namespace {

template <class EXCEPTION_CLASS>
void set_optional_address_validation_result(
        const Fred::Backend::GeneralCheck::contact_optional_address& result,
        EXCEPTION_CLASS& e)
{
    if (result.success())
    {
        e.set(MojeIdImplData::ValidationResult::OK);
        return;
    }

    e.street1 = result.street1_absent ? MojeIdImplData::ValidationResult::REQUIRED
                                      : MojeIdImplData::ValidationResult::OK;

    e.city = result.city_absent ? MojeIdImplData::ValidationResult::REQUIRED
                                : MojeIdImplData::ValidationResult::OK;

    e.postal_code = result.postalcode_absent ? MojeIdImplData::ValidationResult::REQUIRED
                                             : MojeIdImplData::ValidationResult::OK;

    e.country = result.country_absent ? MojeIdImplData::ValidationResult::REQUIRED
                                      : MojeIdImplData::ValidationResult::OK;
}

template <class EXCEPTION_CLASS>
void set_optional_addresses_validation_result(
        const check_contact_optional_addresses& result,
        EXCEPTION_CLASS& e)
{
    set_optional_address_validation_result(
            static_cast<const Fred::Backend::check_contact_addresses_mailing&>(result), e.mailing);
    set_optional_address_validation_result(
            static_cast<const Fred::Backend::check_contact_addresses_billing&>(result), e.billing);
    set_optional_address_validation_result(
            static_cast<const Fred::Backend::check_contact_addresses_shipping&>(result), e.shipping);
    set_optional_address_validation_result(
            static_cast<const Fred::Backend::check_contact_addresses_shipping2&>(result), e.shipping2);
    set_optional_address_validation_result(
            static_cast<const Fred::Backend::check_contact_addresses_shipping3&>(result), e.shipping3);
}

template <class EXCEPTION_CLASS>
void set_permanent_address_validation_result(
        const Fred::Backend::check_contact_place_address& result,
        EXCEPTION_CLASS& e)
{
    e.permanent.address_presence = result.absent ? MojeIdImplData::ValidationResult::REQUIRED
                                                 : MojeIdImplData::ValidationResult::OK;

    set_optional_address_validation_result(result, e.permanent);
}

template <class PRESENCE_ANCESTRAL_CLASS,
        class VALIDITY_ANCESTRAL_CLASS,
        class AVAILABILITY_ANCESTRAL_CLASS,
        class CHECK_CLASS>
void set_presence_validity_availability_result(
        const CHECK_CLASS& check,
        MojeIdImplData::ValidationResult::Value& result)
{
    if (!check.PRESENCE_ANCESTRAL_CLASS::success())
    {
        result = MojeIdImplData::ValidationResult::REQUIRED;
    }
    else if (!check.VALIDITY_ANCESTRAL_CLASS::success())
    {
        result = MojeIdImplData::ValidationResult::INVALID;
    }
    else if (!check.AVAILABILITY_ANCESTRAL_CLASS::success())
    {
        result = MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    }
    else
    {
        result = MojeIdImplData::ValidationResult::OK;
    }
}

template <class PRESENCE_ANCESTRAL_CLASS,
        class VALIDITY_ANCESTRAL_CLASS,
        class CHECK_CLASS>
void set_presence_validity_result(
        const CHECK_CLASS& check,
        MojeIdImplData::ValidationResult::Value& result)
{
    if (!check.PRESENCE_ANCESTRAL_CLASS::success())
    {
        result = MojeIdImplData::ValidationResult::REQUIRED;
    }
    else if (!check.VALIDITY_ANCESTRAL_CLASS::success())
    {
        result = MojeIdImplData::ValidationResult::INVALID;
    }
    else
    {
        result = MojeIdImplData::ValidationResult::OK;
    }
}

template <class VALIDITY_ANCESTRAL_CLASS,
        class AVAILABILITY_ANCESTRAL_CLASS,
        class CHECK_CLASS>
void set_validity_availability_result(
        const CHECK_CLASS& check,
        MojeIdImplData::ValidationResult::Value& result)
{
    if (!check.VALIDITY_ANCESTRAL_CLASS::success())
    {
        result = MojeIdImplData::ValidationResult::INVALID;
    }
    else if (!check.AVAILABILITY_ANCESTRAL_CLASS::success())
    {
        result = MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    }
    else
    {
        result = MojeIdImplData::ValidationResult::OK;
    }
}

template <class CHECK_CLASS, class EXCEPTION_CLASS>
void set_contact_name_result(const CHECK_CLASS& result, EXCEPTION_CLASS& e)
{
    if (result.Fred::Backend::check_contact_name::first_name_absent)
    {
        e.name = MojeIdImplData::ValidationResult::REQUIRED;
    }
    else if (result.Fred::Backend::check_contact_name::last_name_absent)
    {
        e.name = MojeIdImplData::ValidationResult::INVALID;
    }
    else
    {
        e.name = MojeIdImplData::ValidationResult::OK;
    }
}

void set_validity_result(bool valid, MojeIdImplData::ValidationResult::Value& result)
{
    result = valid ? MojeIdImplData::ValidationResult::OK
                   : MojeIdImplData::ValidationResult::INVALID;
}

template <class EXCEPTION_CLASS>
void set_ssn_result(const Fred::Backend::MojeId::check_contact_ssn& result, EXCEPTION_CLASS& e)
{
    if (result.birthdate_absent)
    {
        e.birth_date = MojeIdImplData::ValidationResult::REQUIRED;
    }
    else if (result.birthdate_invalid)
    {
        e.birth_date = MojeIdImplData::ValidationResult::INVALID;
    }
    else
    {
        e.birth_date = MojeIdImplData::ValidationResult::OK;
    }

    if (result.vat_id_num_absent)
    {
        e.vat_id_num = MojeIdImplData::ValidationResult::REQUIRED;
    }
    else
    {
        e.vat_id_num = MojeIdImplData::ValidationResult::OK;
    }
}

} // namespace Fred::Backend::MojeId::{anonymous}

void raise(const CheckMojeIdRegistration& result)
{
    MojeIdImplData::RegistrationValidationResult e;

    set_validity_result(!result.Fred::Backend::MojeId::check_contact_username::invalid, e.username);

    set_contact_name_result(result, e);

    set_validity_result(result.Fred::Backend::MojeId::check_contact_birthday_validity::success(), e.birth_date);
    e.vat_id_num = MojeIdImplData::ValidationResult::OK;

    set_presence_validity_availability_result<
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_email_availability>(result, e.email);

    set_validity_result(result.Fred::Backend::check_contact_notifyemail_validity::success(), e.notify_email);

    set_presence_validity_availability_result<
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity,
            Fred::Backend::check_contact_phone_availability>(result, e.phone);

    set_validity_result(result.Fred::Backend::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckTransferContactPrepareStates& result)
{
    if (result.Fred::Backend::MojeId::states_before_transfer_into_mojeid::mojeid_contact_present)
    {
        throw MojeIdImplData::AlreadyMojeidContact();
    }
    if (result.Fred::Backend::MojeId::states_before_transfer_into_mojeid::server_admin_blocked)
    {
        throw MojeIdImplData::ObjectAdminBlocked();
    }
    if (result.Fred::Backend::MojeId::states_before_transfer_into_mojeid::server_user_blocked)
    {
        throw MojeIdImplData::ObjectUserBlocked();
    }
}

void raise(const CheckCreateContactPrepare& result)
{
    MojeIdImplData::RegistrationValidationResult e;

    if (!result.Fred::Backend::MojeId::check_contact_username_availability::success())
    {
        e.username = MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    }
    else if (result.Fred::Backend::MojeId::check_contact_username::absent)
    {
        e.username = MojeIdImplData::ValidationResult::REQUIRED;
    }
    else if (result.Fred::Backend::MojeId::check_contact_username::invalid)
    {
        e.username = MojeIdImplData::ValidationResult::INVALID;
    }
    else
    {
        e.username = MojeIdImplData::ValidationResult::OK;
    }

    set_contact_name_result(result, e);

    set_validity_result(result.Fred::Backend::MojeId::check_contact_birthday_validity::success(), e.birth_date);
    e.vat_id_num = MojeIdImplData::ValidationResult::OK;

    set_presence_validity_availability_result<
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_email_availability>(result, e.email);

    set_validity_result(result.Fred::Backend::check_contact_notifyemail_validity::success(), e.notify_email);

    set_presence_validity_availability_result<
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity,
            Fred::Backend::check_contact_phone_availability>(result, e.phone);

    set_validity_result(result.Fred::Backend::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckUpdateContactPrepare& result)
{
    MojeIdImplData::UpdateContactPrepareValidationResult e;

    set_contact_name_result(result, e);

    set_validity_result(result.Fred::Backend::MojeId::check_contact_birthday_validity::success(), e.birth_date);

    set_presence_validity_result<
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity>(result, e.email);

    set_validity_result(result.Fred::Backend::check_contact_notifyemail_validity::success(), e.notify_email);

    set_validity_result(result.Fred::Backend::check_contact_phone_validity::success(), e.phone);

    set_validity_result(result.Fred::Backend::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckUpdateValidatedContactPrepare& result)
{
    MojeIdImplData::UpdateValidatedContactPrepareValidationResult e;

    set_contact_name_result(result, e);
    set_validity_result(result.Fred::Backend::MojeId::check_contact_birthday_validity::success(), e.birth_date);
    set_permanent_address_validation_result(result, e);
    throw e;
}

void raise(const CheckCreateValidationRequest& result)
{
    MojeIdImplData::CreateValidationRequestValidationResult e;

    set_contact_name_result(result, e);

    set_permanent_address_validation_result(result, e);

    set_presence_validity_result<
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity>(result, e.email);

    set_validity_result(result.Fred::Backend::check_contact_phone_validity::success(), e.phone);

    set_validity_result(result.Fred::Backend::check_contact_notifyemail_validity::success(), e.notify_email);

    set_validity_result(result.Fred::Backend::check_contact_fax_validity::success(), e.fax);

    set_ssn_result(result, e);

    throw e;
}

void raise(const CheckUpdateTransferContactPrepare& result)
{
    MojeIdImplData::RegistrationValidationResult e;

    set_validity_result(!result.Fred::Backend::MojeId::check_contact_username::invalid, e.username);

    set_contact_name_result(result, e);

    set_ssn_result(result, e);

    set_presence_validity_result<
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity>(result, e.email);

    set_validity_result(result.Fred::Backend::check_contact_notifyemail_validity::success(), e.notify_email);

    set_presence_validity_result<
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity>(result, e.phone);

    set_validity_result(result.Fred::Backend::check_contact_fax_validity::success(), e.fax);

    set_permanent_address_validation_result(result, e);
    set_optional_addresses_validation_result(result, e);

    throw e;
}

void raise(const CheckProcessRegistrationValidation& result)
{
    MojeIdImplData::ProcessRegistrationValidationResult e;

    set_presence_validity_result<
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity>(result, e.email);

    set_presence_validity_result<
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity>(result, e.phone);

    throw e;
}

} // namespace Fred::Backend::MojeIdImplInternal
} // namespace Fred::Backend
} // namespace Fred
