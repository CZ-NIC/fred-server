/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
 *  header of mojeid implementation internals
 */

#ifndef MOJEID_IMPL_INTERNAL_HH_FE2F4C8DBC954060B7F95CB5FD9C93D4
#define MOJEID_IMPL_INTERNAL_HH_FE2F4C8DBC954060B7F95CB5FD9C93D4

#include "src/backend/mojeid/mojeid_checkers.hh"

namespace Fred {
namespace Backend {
namespace MojeIdImplInternal {

struct check_contact_optional_addresses
        : check_contact_addresses_mailing,
          check_contact_addresses_billing,
          check_contact_addresses_shipping,
          check_contact_addresses_shipping2,
          check_contact_addresses_shipping3
{
    check_contact_optional_addresses(const LibFred::InfoContactData& _data)
        : ::Fred::Backend::check_contact_addresses_mailing(_data),
          ::Fred::Backend::check_contact_addresses_billing(_data),
          ::Fred::Backend::check_contact_addresses_shipping(_data),
          ::Fred::Backend::check_contact_addresses_shipping2(_data),
          ::Fred::Backend::check_contact_addresses_shipping3(_data)
    {
    }
    bool success() const
    {
        return this->::Fred::Backend::check_contact_addresses_mailing::success() &&
               this->::Fred::Backend::check_contact_addresses_billing::success() &&
               this->::Fred::Backend::check_contact_addresses_shipping::success() &&
               this->::Fred::Backend::check_contact_addresses_shipping2::success() &&
               this->::Fred::Backend::check_contact_addresses_shipping3::success();
    }
};

typedef boost::mpl::list<Fred::Backend::MojeId::check_contact_username,
            Fred::Backend::check_contact_name,
            Fred::Backend::MojeId::check_contact_birthday_validity,
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_notifyemail_validity,
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity,
            Fred::Backend::check_contact_fax_validity,
            Fred::Backend::check_contact_place_address,
            check_contact_optional_addresses>
        check_mojeid_registration;

typedef boost::mpl::list<Fred::Backend::check_contact_email_availability,
            Fred::Backend::check_contact_phone_availability>
        check_mojeid_registration_ctx;

typedef MojeId::Check<boost::mpl::list<check_mojeid_registration,
            check_mojeid_registration_ctx>>
        CheckMojeIdRegistration;

void raise(const CheckMojeIdRegistration& result);


typedef boost::mpl::list<Fred::Backend::MojeId::states_before_transfer_into_mojeid> check_transfer_contact_prepare_presence;

typedef MojeId::Check<check_transfer_contact_prepare_presence> CheckTransferContactPrepareStates;

void raise(const CheckTransferContactPrepareStates& result);


typedef boost::mpl::list<Fred::Backend::MojeId::check_contact_username,
            Fred::Backend::check_contact_name,
            Fred::Backend::MojeId::check_contact_birthday_validity,
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_notifyemail_validity,
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity,
            Fred::Backend::check_contact_fax_validity,
            Fred::Backend::check_contact_place_address,
            check_contact_optional_addresses>
        check_mojeid_create_contact;

typedef boost::mpl::list<Fred::Backend::MojeId::check_contact_username_availability,
            Fred::Backend::check_contact_email_availability,
            Fred::Backend::check_contact_phone_availability>
        check_mojeid_create_contact_ctx;

typedef MojeId::Check<boost::mpl::list<check_mojeid_create_contact,
        check_mojeid_create_contact_ctx> >
        CheckCreateContactPrepare;

void raise(const CheckCreateContactPrepare& result);


typedef boost::mpl::list<Fred::Backend::check_contact_name,
            Fred::Backend::MojeId::check_contact_birthday_validity,
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_notifyemail_validity,
            Fred::Backend::check_contact_phone_validity,
            Fred::Backend::check_contact_fax_validity,
            Fred::Backend::check_contact_place_address,
            check_contact_optional_addresses>
        check_update_contact_prepare;

typedef MojeId::Check<check_update_contact_prepare> CheckUpdateContactPrepare;

void raise(const CheckUpdateContactPrepare& result);


typedef boost::mpl::list<Fred::Backend::check_contact_name,
            Fred::Backend::MojeId::check_contact_birthday_validity,
            Fred::Backend::check_contact_place_address>
        check_update_validated_contact_prepare;

typedef MojeId::Check<check_update_validated_contact_prepare> CheckUpdateValidatedContactPrepare;

void raise(const CheckUpdateValidatedContactPrepare& result);


typedef boost::mpl::list<Fred::Backend::check_contact_name,
            Fred::Backend::check_contact_place_address,
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_phone_validity,
            Fred::Backend::check_contact_notifyemail_validity,
            Fred::Backend::check_contact_fax_validity,
            Fred::Backend::MojeId::check_contact_ssn>
        check_create_validation_request;

typedef MojeId::Check<check_create_validation_request> CheckCreateValidationRequest;

void raise(const CheckCreateValidationRequest& result);


typedef boost::mpl::list<Fred::Backend::MojeId::check_contact_username,
            Fred::Backend::check_contact_name,
            Fred::Backend::MojeId::check_contact_ssn,
            Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_notifyemail_validity,
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity,
            Fred::Backend::check_contact_fax_validity,
            Fred::Backend::check_contact_place_address,
            check_contact_optional_addresses>
        check_update_transfer_contact_prepare;

typedef MojeId::Check<check_update_transfer_contact_prepare> CheckUpdateTransferContactPrepare;

void raise(const CheckUpdateTransferContactPrepare& result);


typedef boost::mpl::list<Fred::Backend::check_contact_email_presence,
            Fred::Backend::check_contact_email_validity,
            Fred::Backend::check_contact_phone_presence,
            Fred::Backend::check_contact_phone_validity>
        check_process_registration_validation;

typedef MojeId::Check<check_process_registration_validation> CheckProcessRegistrationValidation;

void raise(const CheckProcessRegistrationValidation& result);

} // namespace Fred::Backend::MojeIdImplInternal
} // namespace Fred::Backend
} // namespace Fred

#endif
