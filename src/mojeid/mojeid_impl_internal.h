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

#ifndef MOJEID_IMPL_INTERNAL_H_89048D6E69724FBA9BA759D543C86AF9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID_IMPL_INTERNAL_H_89048D6E69724FBA9BA759D543C86AF9

#include "src/mojeid/mojeid_checkers.h"

namespace Registry {
namespace MojeIDImplInternal {

struct check_contact_optional_addresses
:   Fred::check_contact_addresses_mailing,
    Fred::check_contact_addresses_billing,
    Fred::check_contact_addresses_shipping,
    Fred::check_contact_addresses_shipping2,
    Fred::check_contact_addresses_shipping3
{
    check_contact_optional_addresses(const Fred::InfoContactData &_data)
    :   Fred::check_contact_addresses_mailing(_data),
        Fred::check_contact_addresses_billing(_data),
        Fred::check_contact_addresses_shipping(_data),
        Fred::check_contact_addresses_shipping2(_data),
        Fred::check_contact_addresses_shipping3(_data)
    { }
    bool success()const
    {
        return this->Fred::check_contact_addresses_mailing::success()   &&
               this->Fred::check_contact_addresses_billing::success()   &&
               this->Fred::check_contact_addresses_shipping::success()  &&
               this->Fred::check_contact_addresses_shipping2::success() &&
               this->Fred::check_contact_addresses_shipping3::success();
    }
};

typedef boost::mpl::list< Fred::MojeID::check_contact_username,
                          Fred::check_contact_name,
                          Fred::MojeID::check_contact_birthday,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_phone_presence,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_fax_validity,
                          Fred::check_contact_place_address,
                          check_contact_optional_addresses > check_mojeid_registration;

typedef boost::mpl::list< Fred::check_contact_email_availability,
                          Fred::check_contact_phone_availability > check_mojeid_registration_ctx;

typedef boost::mpl::list< Fred::MojeID::Check::states_before_transfer_into_mojeid > check_transfer_contact_prepare_presence;

typedef Fred::Check< boost::mpl::list< check_mojeid_registration,
                                       check_mojeid_registration_ctx,
                                       check_transfer_contact_prepare_presence > > CheckMojeIDRegistration;

void raise(const CheckMojeIDRegistration &result);


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
                          check_contact_optional_addresses > check_mojeid_create_contact;

typedef boost::mpl::list< Fred::MojeID::check_contact_username_availability,
                          Fred::check_contact_email_availability,
                          Fred::check_contact_phone_availability > check_mojeid_create_contact_ctx;

typedef Fred::Check< boost::mpl::list< check_mojeid_create_contact,
                                       check_mojeid_create_contact_ctx > > CheckCreateContactPrepare;

void raise(const CheckCreateContactPrepare &result);


typedef boost::mpl::list< Fred::check_contact_name,
                          Fred::MojeID::check_contact_birthday_validity,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_phone_presence,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_fax_validity,
                          Fred::check_contact_place_address,
                          check_contact_optional_addresses > check_update_contact_prepare;

typedef Fred::Check< check_update_contact_prepare > CheckUpdateContactPrepare;

void raise(const CheckUpdateContactPrepare &result);


typedef boost::mpl::list< Fred::check_contact_name,
                          Fred::check_contact_place_address_mandatory,
                          Fred::check_contact_email_presence,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_fax_validity,
                          Fred::MojeID::check_contact_ssn > check_create_validation_request;

typedef Fred::Check< check_create_validation_request > CheckCreateValidationRequest;

void raise(const CheckCreateValidationRequest &result);


typedef boost::mpl::list< Fred::MojeID::check_contact_username,
                          Fred::MojeID::check_contact_birthday,
                          Fred::check_contact_email_validity,
                          Fred::check_contact_notifyemail_validity,
                          Fred::check_contact_phone_validity,
                          Fred::check_contact_fax_validity,
                          Fred::check_contact_place_address,
                          check_contact_optional_addresses > check_update_transfer_contact_prepare;

typedef boost::mpl::list< Fred::check_contact_email_availability,
                          Fred::check_contact_phone_availability > check_update_transfer_contact_prepare_ctx;

typedef Fred::Check< boost::mpl::list< check_update_transfer_contact_prepare,
                                       check_update_transfer_contact_prepare_ctx,
                                       check_transfer_contact_prepare_presence > > CheckUpdateTransferContactPrepare;

void raise(const CheckUpdateTransferContactPrepare &result);

}//namespace Registry::MojeIDImplInternal
}//namespace Registry

#endif//MOJEID_IMPL_INTERNAL_H_89048D6E69724FBA9BA759D543C86AF9
