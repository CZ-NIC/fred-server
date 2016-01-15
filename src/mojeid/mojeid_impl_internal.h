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
 *  header of mojeid2 implementation internals
 */

#ifndef MOJEID_IMPL_INTERNAL_H_89048D6E69724FBA9BA759D543C86AF9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID_IMPL_INTERNAL_H_89048D6E69724FBA9BA759D543C86AF9

#include "src/mojeid/mojeid2_checkers.h"

namespace Registry {
namespace MojeIDImplInternal {

typedef boost::mpl::list< Fred::check_contact_name,
                          Fred::MojeID::check_contact_birthday,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_phone_presence,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_fax_validity,
                          Fred::check_contact_place_address,
                          Fred::check_contact_addresses_mailing,
                          Fred::check_contact_addresses_billing,
                          Fred::check_contact_addresses_shipping,
                          Fred::check_contact_addresses_shipping2,
                          Fred::check_contact_addresses_shipping3 > check_mojeid_registration;

typedef boost::mpl::list< Fred::MojeID::check_contact_username_availability,
                          Fred::check_contact_email_availability,
                          Fred::check_contact_phone_availability > check_mojeid_registration_ctx;

typedef boost::mpl::list< Fred::MojeID::Check::states_before_transfer_into_mojeid > check_transfer_contact_prepare_presence;

typedef Fred::Check< boost::mpl::list< check_mojeid_registration,
                                       check_mojeid_registration_ctx,
                                       check_transfer_contact_prepare_presence > > CheckMojeIDRegistration;

void raise(const CheckMojeIDRegistration &result) __attribute__ ((__noreturn__));

typedef boost::mpl::list< Fred::MojeID::check_contact_username,
                          Fred::check_contact_name,
                          Fred::MojeID::check_contact_birthday_validity,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_phone_presence,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_fax_validity,
                          Fred::check_contact_place_address,
                          Fred::check_contact_addresses_mailing,
                          Fred::check_contact_addresses_billing,
                          Fred::check_contact_addresses_shipping,
                          Fred::check_contact_addresses_shipping2,
                          Fred::check_contact_addresses_shipping3 > check_mojeid_create_contact;


typedef Fred::Check< boost::mpl::list< check_mojeid_create_contact,
                                       check_mojeid_registration_ctx > > CheckCreateContactPrepare;


typedef boost::mpl::list< Fred::check_contact_name,
                          Fred::check_contact_place_address_mandatory,
                          Fred::check_contact_addresses_mailing,
                          Fred::check_contact_addresses_billing,
                          Fred::check_contact_addresses_shipping,
                          Fred::check_contact_addresses_shipping2,
                          Fred::check_contact_addresses_shipping3,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_phone_presence,
                          Fred::check_contact_phone_validity > check_create_contact_prepare;

typedef boost::mpl::list< Fred::MojeID::check_contact_username_availability,
                          Fred::check_contact_email_availability,
                          Fred::check_contact_phone_availability > check_create_contact_prepare_ctx;

typedef boost::mpl::list< Fred::check_contact_email_availability,
                          Fred::check_contact_phone_availability > check_process_registration_request_ctx;

typedef Fred::Check< boost::mpl::list< check_create_contact_prepare,
                check_process_registration_request_ctx > > CheckProcessRegistrationRequest;



typedef boost::mpl::list< Fred::check_contact_name,
                          Fred::MojeID::check_contact_birthday,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_phone_presence,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_fax_validity,
                          Fred::check_contact_place_address,
                          Fred::check_contact_addresses_mailing,
                          Fred::check_contact_addresses_billing,
                          Fred::check_contact_addresses_shipping,
                          Fred::check_contact_addresses_shipping2,
                          Fred::check_contact_addresses_shipping3 > check_update_contact_prepare;

typedef Fred::Check< check_update_contact_prepare > CheckUpdateContactPrepare;

typedef CheckUpdateContactPrepare UpdateContactPrepareError;

typedef boost::mpl::list< Fred::check_contact_name,
                          Fred::check_contact_place_address,
                          Fred::check_contact_addresses_mailing,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_fax_validity > check_update_transfer;
typedef Fred::Check< check_update_transfer > CheckUpdateTransfer;
typedef CheckUpdateTransfer UpdateTransferError;

typedef check_update_contact_prepare check_process_identification_request;
typedef Fred::Check< check_process_identification_request > CheckProcessIdentificationRequest;
typedef CheckProcessIdentificationRequest ProcessIdentificationRequestError;

typedef boost::mpl::list< Fred::check_contact_name,
                          Fred::check_contact_place_address_mandatory,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_fax_validity,
                          Fred::MojeID::check_contact_ssn > check_create_validation_request;
typedef Fred::Check< check_create_validation_request > CheckCreateValidationRequest;
typedef CheckCreateValidationRequest CreateValidationRequestError;

}//namespace Registry::MojeIDImplInternal
}//namespace Registry

#endif//MOJEID_IMPL_INTERNAL_H_89048D6E69724FBA9BA759D543C86AF9
