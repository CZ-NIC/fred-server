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
 *  header of mojeid implementation internals
 */

#ifndef MOJEID_IMPL_INTERNAL_HH_FE2F4C8DBC954060B7F95CB5FD9C93D4
#define MOJEID_IMPL_INTERNAL_HH_FE2F4C8DBC954060B7F95CB5FD9C93D4

#include "src/backend/mojeid/mojeid_checkers.hh"

namespace Registry {
namespace MojeIDImplInternal {

struct check_contact_optional_addresses
:   LibFred::check_contact_addresses_mailing,
    LibFred::check_contact_addresses_billing,
    LibFred::check_contact_addresses_shipping,
    LibFred::check_contact_addresses_shipping2,
    LibFred::check_contact_addresses_shipping3
{
    check_contact_optional_addresses(const LibFred::InfoContactData &_data)
    :   LibFred::check_contact_addresses_mailing(_data),
        LibFred::check_contact_addresses_billing(_data),
        LibFred::check_contact_addresses_shipping(_data),
        LibFred::check_contact_addresses_shipping2(_data),
        LibFred::check_contact_addresses_shipping3(_data)
    { }
    bool success()const
    {
        return this->LibFred::check_contact_addresses_mailing::success()   &&
               this->LibFred::check_contact_addresses_billing::success()   &&
               this->LibFred::check_contact_addresses_shipping::success()  &&
               this->LibFred::check_contact_addresses_shipping2::success() &&
               this->LibFred::check_contact_addresses_shipping3::success();
    }
};

typedef boost::mpl::list< LibFred::MojeID::check_contact_username,
                          LibFred::check_contact_name,
                          LibFred::MojeID::check_contact_birthday_validity,
                          LibFred::check_contact_email_presence,
                          LibFred::check_contact_email_validity,
                          LibFred::check_contact_notifyemail_validity,
                          LibFred::check_contact_phone_presence,
                          LibFred::check_contact_phone_validity,
                          LibFred::check_contact_fax_validity,
                          LibFred::check_contact_place_address,
                          check_contact_optional_addresses > check_mojeid_registration;

typedef boost::mpl::list< LibFred::check_contact_email_availability,
                          LibFred::check_contact_phone_availability > check_mojeid_registration_ctx;

typedef LibFred::Check< boost::mpl::list< check_mojeid_registration,
                                       check_mojeid_registration_ctx > > CheckMojeIDRegistration;

void raise(const CheckMojeIDRegistration &result);


typedef boost::mpl::list< LibFred::MojeID::Check::states_before_transfer_into_mojeid > check_transfer_contact_prepare_presence;

typedef LibFred::Check< check_transfer_contact_prepare_presence > CheckTransferContactPrepareStates;

void raise(const CheckTransferContactPrepareStates &result);


typedef boost::mpl::list< LibFred::MojeID::check_contact_username,
                          LibFred::check_contact_name,
                          LibFred::MojeID::check_contact_birthday_validity,
                          LibFred::check_contact_email_presence,
                          LibFred::check_contact_email_validity,
                          LibFred::check_contact_notifyemail_validity,
                          LibFred::check_contact_phone_presence,
                          LibFred::check_contact_phone_validity,
                          LibFred::check_contact_fax_validity,
                          LibFred::check_contact_place_address,
                          check_contact_optional_addresses > check_mojeid_create_contact;

typedef boost::mpl::list< LibFred::MojeID::check_contact_username_availability,
                          LibFred::check_contact_email_availability,
                          LibFred::check_contact_phone_availability > check_mojeid_create_contact_ctx;

typedef LibFred::Check< boost::mpl::list< check_mojeid_create_contact,
                                       check_mojeid_create_contact_ctx > > CheckCreateContactPrepare;

void raise(const CheckCreateContactPrepare &result);


typedef boost::mpl::list< LibFred::check_contact_name,
                          LibFred::MojeID::check_contact_birthday_validity,
                          LibFred::check_contact_email_presence,
                          LibFred::check_contact_email_validity,
                          LibFred::check_contact_notifyemail_validity,
                          LibFred::check_contact_phone_validity,
                          LibFred::check_contact_fax_validity,
                          LibFred::check_contact_place_address,
                          check_contact_optional_addresses > check_update_contact_prepare;

typedef LibFred::Check< check_update_contact_prepare > CheckUpdateContactPrepare;

void raise(const CheckUpdateContactPrepare &result);


typedef boost::mpl::list< LibFred::check_contact_name,
                          LibFred::check_contact_place_address,
                          LibFred::check_contact_email_presence,
                          LibFred::check_contact_email_validity,
                          LibFred::check_contact_phone_validity,
                          LibFred::check_contact_notifyemail_validity,
                          LibFred::check_contact_fax_validity,
                          LibFred::MojeID::check_contact_ssn > check_create_validation_request;

typedef LibFred::Check< check_create_validation_request > CheckCreateValidationRequest;

void raise(const CheckCreateValidationRequest &result);


typedef boost::mpl::list< LibFred::MojeID::check_contact_username,
                          LibFred::check_contact_name,
                          LibFred::MojeID::check_contact_ssn,
                          LibFred::check_contact_email_presence,
                          LibFred::check_contact_email_validity,
                          LibFred::check_contact_notifyemail_validity,
                          LibFred::check_contact_phone_presence,
                          LibFred::check_contact_phone_validity,
                          LibFred::check_contact_fax_validity,
                          LibFred::check_contact_place_address,
                          check_contact_optional_addresses > check_update_transfer_contact_prepare;

typedef LibFred::Check< check_update_transfer_contact_prepare > CheckUpdateTransferContactPrepare;

void raise(const CheckUpdateTransferContactPrepare &result);


typedef boost::mpl::list< LibFred::check_contact_email_presence,
                          LibFred::check_contact_email_validity,
                          LibFred::check_contact_phone_presence,
                          LibFred::check_contact_phone_validity > check_process_registration_validation;

typedef LibFred::Check< check_process_registration_validation > CheckProcessRegistrationValidation;

void raise(const CheckProcessRegistrationValidation &result);

} // namespace Registry::MojeIDImplInternal
} // namespace Registry

#endif
