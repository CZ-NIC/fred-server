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

struct ValidationResult
{
    enum Value
    {
        OK,
        NOT_AVAILABLE,
        INVALID,
        REQUIRED
    };
};

struct AddressValidationResult
{
    AddressValidationResult()
    :   street1    (ValidationResult::OK),
        city       (ValidationResult::OK),
        postal_code(ValidationResult::OK),
        country    (ValidationResult::OK) { }
    ValidationResult::Value street1;
    ValidationResult::Value city;
    ValidationResult::Value postal_code;
    ValidationResult::Value country;
};

struct MandatoryAddressValidationResult
{
    MandatoryAddressValidationResult()
    :   address_presence(ValidationResult::OK),
        street1         (ValidationResult::OK),
        city            (ValidationResult::OK),
        postal_code     (ValidationResult::OK),
        country         (ValidationResult::OK) { }
    ValidationResult::Value address_presence;
    ValidationResult::Value street1;
    ValidationResult::Value city;
    ValidationResult::Value postal_code;
    ValidationResult::Value country;
};

struct MessageLimitExceeded
{
    boost::gregorian::date limit_expire_date;
    std::string as_string()const
    {
        std::ostringstream out;
        out << "MessageLimitExceeded: limit_expire_date = " << limit_expire_date;
        return out.str();
    }
};

struct RegistrationValidationResult
{
    RegistrationValidationResult()
    :   username    (ValidationResult::OK),
        first_name  (ValidationResult::OK),
        last_name   (ValidationResult::OK),
        birth_date  (ValidationResult::OK),
        email       (ValidationResult::OK),
        notify_email(ValidationResult::OK),
        phone       (ValidationResult::OK),
        fax         (ValidationResult::OK) { }
    ValidationResult::Value          username;
    ValidationResult::Value          first_name;
    ValidationResult::Value          last_name;
    ValidationResult::Value          birth_date;
    ValidationResult::Value          email;
    ValidationResult::Value          notify_email;
    ValidationResult::Value          phone;
    ValidationResult::Value          fax;
    MandatoryAddressValidationResult permanent;
    AddressValidationResult          mailing;
    AddressValidationResult          billing;
    AddressValidationResult          shipping;
    AddressValidationResult          shipping2;
    AddressValidationResult          shipping3;
};

struct UpdateContactPrepareValidationResult
{
    UpdateContactPrepareValidationResult()
    :   first_name  (ValidationResult::OK),
        last_name   (ValidationResult::OK),
        birth_date  (ValidationResult::OK),
        email       (ValidationResult::OK),
        notify_email(ValidationResult::OK),
        phone       (ValidationResult::OK),
        fax         (ValidationResult::OK) { }
    ValidationResult::Value          first_name;
    ValidationResult::Value          last_name;
    ValidationResult::Value          birth_date;
    ValidationResult::Value          email;
    ValidationResult::Value          notify_email;
    ValidationResult::Value          phone;
    ValidationResult::Value          fax;
    MandatoryAddressValidationResult permanent;
    AddressValidationResult          mailing;
    AddressValidationResult          billing;
    AddressValidationResult          shipping;
    AddressValidationResult          shipping2;
    AddressValidationResult          shipping3;
};

struct CreateValidationRequestValidationResult
{
    CreateValidationRequestValidationResult()
    :   first_name  (ValidationResult::OK),
        last_name   (ValidationResult::OK),
        email       (ValidationResult::OK),
        phone       (ValidationResult::OK),
        notify_email(ValidationResult::OK),
        fax         (ValidationResult::OK),
        birth_date  (ValidationResult::OK),
        vat_id_num  (ValidationResult::OK) { }
    ValidationResult::Value          first_name;
    ValidationResult::Value          last_name;
    MandatoryAddressValidationResult permanent;
    ValidationResult::Value          email;
    ValidationResult::Value          phone;
    ValidationResult::Value          notify_email;
    ValidationResult::Value          fax;
    ValidationResult::Value          birth_date;
    ValidationResult::Value          vat_id_num;
};

struct ProcessRegistrationValidationResult
{
    ProcessRegistrationValidationResult()
    :   email(ValidationResult::OK),
        phone(ValidationResult::OK) { }
    ValidationResult::Value email;
    ValidationResult::Value phone;
};

struct Date
{
    std::string value;
};

struct CreateContact
{
    std::string                 username;
    std::string                 first_name;
    std::string                 last_name;
    Nullable< std::string >     organization;
    Nullable< std::string >     vat_reg_num;
    Nullable< Date >            birth_date;
    Nullable< std::string >     id_card_num;
    Nullable< std::string >     passport_num;
    Nullable< std::string >     ssn_id_num;
    Nullable< std::string >     vat_id_num;
    Address                     permanent;
    Nullable< Address >         mailing;
    Nullable< Address >         billing;
    Nullable< ShippingAddress > shipping;
    Nullable< ShippingAddress > shipping2;
    Nullable< ShippingAddress > shipping3;
    std::string                 email;
    Nullable< std::string >     notify_email;
    std::string                 telephone;
    Nullable< std::string >     fax;
};

struct UpdateContact
{
    ContactId                   id;
    std::string                 first_name;
    std::string                 last_name;
    Nullable< std::string >     organization;
    Nullable< std::string >     vat_reg_num;
    Nullable< Date >            birth_date;
    Nullable< std::string >     id_card_num;
    Nullable< std::string >     passport_num;
    Nullable< std::string >     ssn_id_num;
    Nullable< std::string >     vat_id_num;
    Address                     permanent;
    Nullable< Address >         mailing;
    Nullable< Address >         billing;
    Nullable< ShippingAddress > shipping;
    Nullable< ShippingAddress > shipping2;
    Nullable< ShippingAddress > shipping3;
    std::string                 email;
    Nullable< std::string >     notify_email;
    Nullable< std::string >     telephone;
    Nullable< std::string >     fax;
};

typedef UpdateContact InfoContact; ///< XXX

struct SetContact
{
    Nullable< std::string > organization;
    Nullable< std::string > vat_reg_num;
    Nullable< Date >        birth_date;
    Nullable< std::string > vat_id_num;
    Address                 permanent;
    Nullable< Address >     mailing;
    std::string             email;
    Nullable< std::string > notify_email;
    std::string             telephone;
    Nullable< std::string > fax;
};

struct ContactStateInfo
{
    ContactId                          contact_id;
    boost::posix_time::ptime           mojeid_activation_datetime;
    Nullable< boost::gregorian::date > identification_date;
    Nullable< boost::gregorian::date > validation_date;
    Nullable< boost::gregorian::date > linked_date;
};

struct Buffer
{
    std::string value;
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
