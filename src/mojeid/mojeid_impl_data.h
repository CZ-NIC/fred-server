/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 *  declaration for Registry::MojeIDImplData namespace
 */

#ifndef MOJEID_IMPL_DATA_H_7C7FD17C00D041F4BDAF5CE5A8CE5337//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID_IMPL_DATA_H_7C7FD17C00D041F4BDAF5CE5A8CE5337

#include "src/corba/MojeID.hh"
#include "util/db/nullable.h"
#include <sstream>

namespace Registry
{

namespace MojeIDImplData
{

typedef unsigned long long ContactId;

struct Address
{
    std::string             street1;
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    std::string             city;
    Nullable< std::string > state;
    std::string             postal_code;
    std::string             country;
};

struct ShippingAddress
{
    Nullable< std::string > company_name;
    std::string             street1;
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    std::string             city;
    Nullable< std::string > state;
    std::string             postal_code;
    std::string             country;
};

typedef Registry::MojeID::ValidationResult ValidationResult;

struct AddressValidationResult
{
    AddressValidationResult()
    :   street1    (Registry::MojeID::OK),
        city       (Registry::MojeID::OK),
        postal_code(Registry::MojeID::OK),
        country    (Registry::MojeID::OK) { }
    ValidationResult street1;
    ValidationResult city;
    ValidationResult postal_code;
    ValidationResult country;
};

struct MandatoryAddressValidationResult
{
    MandatoryAddressValidationResult()
    :   address_presence(Registry::MojeID::OK),
        street1         (Registry::MojeID::OK),
        city            (Registry::MojeID::OK),
        postal_code     (Registry::MojeID::OK),
        country         (Registry::MojeID::OK) { }
    ValidationResult address_presence;
    ValidationResult street1;
    ValidationResult city;
    ValidationResult postal_code;
    ValidationResult country;
};

struct ShippingAddressValidationResult
{
    ShippingAddressValidationResult()
    :   street1    (Registry::MojeID::OK),
        city       (Registry::MojeID::OK),
        postal_code(Registry::MojeID::OK),
        country    (Registry::MojeID::OK) { }
    ValidationResult street1;
    ValidationResult city;
    ValidationResult postal_code;
    ValidationResult country;
};

struct MessageLimitExceeded
{
    boost::gregorian::date limit_expire_date;
    unsigned short         limit_count;
    unsigned short         limit_days;
    std::string as_string()const
    {
        std::ostringstream out;
        out << "MessageLimitExceeded: limit_expire_date = " << limit_expire_date << ", "
               "limit_count = " << limit_count << ", limit_days = " << limit_days;
        return out.str();
    }
};

struct RegistrationValidationResult
{
    RegistrationValidationResult()
    :   username    (Registry::MojeID::OK),
        first_name  (Registry::MojeID::OK),
        last_name   (Registry::MojeID::OK),
        birth_date  (Registry::MojeID::OK),
        email       (Registry::MojeID::OK),
        notify_email(Registry::MojeID::OK),
        phone       (Registry::MojeID::OK),
        fax         (Registry::MojeID::OK) { }
    ValidationResult                username;
    ValidationResult                first_name;
    ValidationResult                last_name;
    ValidationResult                birth_date;
    ValidationResult                email;
    ValidationResult                notify_email;
    ValidationResult                phone;
    ValidationResult                fax;
    AddressValidationResult         permanent;
    AddressValidationResult         mailing;
    AddressValidationResult         billing;
    ShippingAddressValidationResult shipping;
    ShippingAddressValidationResult shipping2;
    ShippingAddressValidationResult shipping3;
};

struct UpdateContactPrepareValidationResult
{
    UpdateContactPrepareValidationResult()
    :   first_name  (Registry::MojeID::OK),
        last_name   (Registry::MojeID::OK),
        birth_date  (Registry::MojeID::OK),
        email       (Registry::MojeID::OK),
        notify_email(Registry::MojeID::OK),
        phone       (Registry::MojeID::OK),
        fax         (Registry::MojeID::OK) { }
    ValidationResult                first_name;
    ValidationResult                last_name;
    ValidationResult                birth_date;
    ValidationResult                email;
    ValidationResult                notify_email;
    ValidationResult                phone;
    ValidationResult                fax;
    AddressValidationResult         permanent;
    AddressValidationResult         mailing;
    AddressValidationResult         billing;
    ShippingAddressValidationResult shipping;
    ShippingAddressValidationResult shipping2;
    ShippingAddressValidationResult shipping3;
};

struct CreateValidationRequestValidationResult
{
    CreateValidationRequestValidationResult()
    :   first_name  (Registry::MojeID::OK),
        last_name   (Registry::MojeID::OK),
        email       (Registry::MojeID::OK),
        phone       (Registry::MojeID::OK),
        notify_email(Registry::MojeID::OK),
        fax         (Registry::MojeID::OK),
        ssn         (Registry::MojeID::OK) { }
    ValidationResult                 first_name;
    ValidationResult                 last_name;
    MandatoryAddressValidationResult permanent;
    ValidationResult                 email;
    ValidationResult                 phone;
    ValidationResult                 notify_email;
    ValidationResult                 fax;
    ValidationResult                 ssn;
};

struct ProcessRegistrationValidationResult
{
    ProcessRegistrationValidationResult()
    :   email(Registry::MojeID::OK),
        phone(Registry::MojeID::OK) { }
    ValidationResult email;
    ValidationResult phone;
};

struct CreateContact
{
    std::string                        username;
    std::string                        first_name;
    std::string                        last_name;
    Nullable< std::string >            organization;
    Nullable< std::string >            vat_reg_num;
    Nullable< boost::gregorian::date > birth_date;
    Nullable< std::string >            id_card_num;
    Nullable< std::string >            passport_num;
    Nullable< std::string >            ssn_id_num;
    Nullable< std::string >            vat_id_num;
    Address                            permanent;
    Nullable< Address >                mailing;
    Nullable< Address >                billing;
    Nullable< ShippingAddress >        shipping;
    Nullable< ShippingAddress >        shipping2;
    Nullable< ShippingAddress >        shipping3;
    std::string                        email;
    Nullable< std::string >            notify_email;
    std::string                        telephone;
    Nullable< std::string >            fax;
};

struct UpdateContact
{
    ContactId                          id;
    std::string                        first_name;
    std::string                        last_name;
    Nullable< std::string >            organization;
    Nullable< std::string >            vat_reg_num;
    Nullable< boost::gregorian::date > birth_date;
    Nullable< std::string >            id_card_num;
    Nullable< std::string >            passport_num;
    Nullable< std::string >            ssn_id_num;
    Nullable< std::string >            vat_id_num;
    Address                            permanent;
    Nullable< Address >                mailing;
    Nullable< Address >                billing;
    Nullable< ShippingAddress >        shipping;
    Nullable< ShippingAddress >        shipping2;
    Nullable< ShippingAddress >        shipping3;
    std::string                        email;
    Nullable< std::string >            notify_email;
    Nullable< std::string >            telephone;
    Nullable< std::string >            fax;
};

typedef UpdateContact InfoContact; ///< XXX

struct SetContact
{
    Nullable< std::string >            organization;
    Nullable< std::string >            vat_reg_num;
    Nullable< boost::gregorian::date > birth_date;
    Nullable< std::string >            vat_id_num;
    Address                            permanent;
    Nullable< Address >                mailing;
    std::string                        email;
    Nullable< std::string >            notify_email;
    std::string                        telephone;
    Nullable< std::string >            fax;
};

struct ContactStateInfo
{
    ContactId                          contact_id;
    boost::posix_time::ptime           mojeid_activation_datetime;
    boost::gregorian::date             conditionally_identification_date;
    Nullable< boost::gregorian::date > identification_date;
    Nullable< boost::gregorian::date > validation_date;
    Nullable< boost::gregorian::date > linked_date;
};

typedef std::vector< ContactStateInfo > ContactStateInfoList;

typedef std::vector< std::string > ContactHandleList;

struct IdentificationFailed { };

struct IdentificationAlreadyProcessed { };

struct IdentificationAlreadyInvalidated { };

struct ContactChanged { };

struct PublicRequestDoesntExist { };

struct ObjectAdminBlocked { };

struct ObjectUserBlocked { };

struct AlreadyMojeidContact { };

struct ObjectDoesntExist { };

struct IdentificationRequestDoesntExist { };

struct ValidationRequestExists { };

struct ValidationAlreadyProcessed { };

}//namespace Registry::MojeIDImplData
}//namespace Registry

#endif//MOJEID_IMPL_DATA_H_7C7FD17C00D041F4BDAF5CE5A8CE5337
