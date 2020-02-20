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
#ifndef MOJEID_IMPL_DATA_HH_6021A798220F424AAA6327B3E2F0C458
#define MOJEID_IMPL_DATA_HH_6021A798220F424AAA6327B3E2F0C458

#include "util/db/nullable.hh"
#include <sstream>

namespace Fred {
namespace Backend {
namespace MojeIdImplData {

typedef unsigned long long ContactId;

struct Address
{
    std::string street1;
    Nullable<std::string> street2;
    Nullable<std::string> street3;
    std::string city;
    Nullable<std::string> state;
    std::string postal_code;
    std::string country;
};

struct ShippingAddress
{
    Nullable<std::string> company_name;
    std::string street1;
    Nullable<std::string> street2;
    Nullable<std::string> street3;
    std::string city;
    Nullable<std::string> state;
    std::string postal_code;
    std::string country;
};

struct ValidationResult
{
    enum Value
    {
        UNKNOWN, //the first value represented by 0 is most likely set when explicit initialization is missing
        OK,
        NOT_AVAILABLE,
        INVALID,
        REQUIRED,
    };
};

struct AddressValidationResult
{
    AddressValidationResult()
        : street1(ValidationResult::UNKNOWN),
          city(ValidationResult::UNKNOWN),
          postal_code(ValidationResult::UNKNOWN),
          country(ValidationResult::UNKNOWN)
    {
    }
    void set(ValidationResult::Value result)
    {
        street1 = result;
        city = result;
        postal_code = result;
        country = result;
    }
    ValidationResult::Value street1;
    ValidationResult::Value city;
    ValidationResult::Value postal_code;
    ValidationResult::Value country;
};

struct MandatoryAddressValidationResult
{
    MandatoryAddressValidationResult()
        : address_presence(ValidationResult::UNKNOWN),
          street1(ValidationResult::UNKNOWN),
          city(ValidationResult::UNKNOWN),
          postal_code(ValidationResult::UNKNOWN),
          country(ValidationResult::UNKNOWN)
    {
    }
    void set(ValidationResult::Value result)
    {
        street1 = result;
        city = result;
        postal_code = result;
        country = result;
    }
    ValidationResult::Value address_presence;
    ValidationResult::Value street1;
    ValidationResult::Value city;
    ValidationResult::Value postal_code;
    ValidationResult::Value country;
};

struct MessageLimitExceeded
{
    boost::posix_time::ptime limit_expire_datetime;
    std::string as_string() const
    {
        std::ostringstream out;
        out << "MessageLimitExceeded: limit_expire_datetime = " << limit_expire_datetime;
        return out.str();
    }
};

struct RegistrationValidationResult
{
    RegistrationValidationResult()
        : username(ValidationResult::UNKNOWN),
          name(ValidationResult::UNKNOWN),
          birth_date(ValidationResult::UNKNOWN),
          vat_id_num(ValidationResult::UNKNOWN),
          email(ValidationResult::UNKNOWN),
          notify_email(ValidationResult::UNKNOWN),
          phone(ValidationResult::UNKNOWN),
          fax(ValidationResult::UNKNOWN)
    {
    }
    ValidationResult::Value username;
    ValidationResult::Value name;
    ValidationResult::Value birth_date;
    ValidationResult::Value vat_id_num;
    ValidationResult::Value email;
    ValidationResult::Value notify_email;
    ValidationResult::Value phone;
    ValidationResult::Value fax;
    MandatoryAddressValidationResult permanent;
    AddressValidationResult mailing;
    AddressValidationResult billing;
    AddressValidationResult shipping;
    AddressValidationResult shipping2;
    AddressValidationResult shipping3;
};

struct UpdateContactPrepareValidationResult
{
    UpdateContactPrepareValidationResult()
        : name(ValidationResult::UNKNOWN),
          birth_date(ValidationResult::UNKNOWN),
          email(ValidationResult::UNKNOWN),
          notify_email(ValidationResult::UNKNOWN),
          phone(ValidationResult::UNKNOWN),
          fax(ValidationResult::UNKNOWN)
    {
    }
    ValidationResult::Value name;
    ValidationResult::Value birth_date;
    ValidationResult::Value email;
    ValidationResult::Value notify_email;
    ValidationResult::Value phone;
    ValidationResult::Value fax;
    MandatoryAddressValidationResult permanent;
    AddressValidationResult mailing;
    AddressValidationResult billing;
    AddressValidationResult shipping;
    AddressValidationResult shipping2;
    AddressValidationResult shipping3;
};

struct UpdateValidatedContactPrepareValidationResult
{
    UpdateValidatedContactPrepareValidationResult()
        : name(ValidationResult::UNKNOWN),
          birth_date(ValidationResult::UNKNOWN)
    {
    }
    ValidationResult::Value name;
    ValidationResult::Value birth_date;
    MandatoryAddressValidationResult permanent;
};

struct CreateValidationRequestValidationResult
{
    CreateValidationRequestValidationResult()
        : name(ValidationResult::UNKNOWN),
          email(ValidationResult::UNKNOWN),
          phone(ValidationResult::UNKNOWN),
          notify_email(ValidationResult::UNKNOWN),
          fax(ValidationResult::UNKNOWN),
          birth_date(ValidationResult::UNKNOWN),
          vat_id_num(ValidationResult::UNKNOWN)
    {
    }
    ValidationResult::Value name;
    MandatoryAddressValidationResult permanent;
    ValidationResult::Value email;
    ValidationResult::Value phone;
    ValidationResult::Value notify_email;
    ValidationResult::Value fax;
    ValidationResult::Value birth_date;
    ValidationResult::Value vat_id_num;
};

struct ProcessRegistrationValidationResult
{
    ProcessRegistrationValidationResult()
        : email(ValidationResult::UNKNOWN),
          phone(ValidationResult::UNKNOWN)
    {
    }
    ValidationResult::Value email;
    ValidationResult::Value phone;
};

struct Birthdate
{
    Birthdate()
        : value()
    {
    }

    explicit Birthdate(const std::string& _value)
        : value(_value)
    {
    }

    std::string value;
};

struct CreateContact
{
    std::string username;
    std::string name;
    Nullable<std::string> organization;
    Nullable<std::string> vat_reg_num;
    Nullable<Birthdate> birth_date;
    Nullable<std::string> id_card_num;
    Nullable<std::string> passport_num;
    Nullable<std::string> ssn_id_num;
    Nullable<std::string> vat_id_num;
    Address permanent;
    Nullable<Address> mailing;
    Nullable<Address> billing;
    Nullable<ShippingAddress> shipping;
    Nullable<ShippingAddress> shipping2;
    Nullable<ShippingAddress> shipping3;
    std::string email;
    Nullable<std::string> notify_email;
    std::string telephone;
    Nullable<std::string> fax;
};

struct UpdateContact
{
    std::string name;
    Nullable<std::string> organization;
    Nullable<std::string> vat_reg_num;
    Nullable<Birthdate> birth_date;
    Nullable<std::string> id_card_num;
    Nullable<std::string> passport_num;
    Nullable<std::string> ssn_id_num;
    Nullable<std::string> vat_id_num;
    Address permanent;
    Nullable<Address> mailing;
    Nullable<Address> billing;
    Nullable<ShippingAddress> shipping;
    Nullable<ShippingAddress> shipping2;
    Nullable<ShippingAddress> shipping3;
    std::string email;
    Nullable<std::string> notify_email;
    Nullable<std::string> telephone;
    Nullable<std::string> fax;
};

struct ValidatedContactData
{
    std::string name;
    Birthdate birth_date;
    Address permanent;
};

struct InfoContact
{
    ContactId id;
    std::string name;
    Nullable<std::string> organization;
    Nullable<std::string> vat_reg_num;
    Nullable<Birthdate> birth_date;
    Nullable<std::string> id_card_num;
    Nullable<std::string> passport_num;
    Nullable<std::string> ssn_id_num;
    Nullable<std::string> vat_id_num;
    Address permanent;
    Nullable<Address> mailing;
    Nullable<Address> billing;
    Nullable<ShippingAddress> shipping;
    Nullable<ShippingAddress> shipping2;
    Nullable<ShippingAddress> shipping3;
    std::string email;
    Nullable<std::string> notify_email;
    Nullable<std::string> telephone;
    Nullable<std::string> fax;
};

struct InfoContactPublishFlags
{
    bool name;
    bool organization;
    bool vat_reg_num;
    bool birth_date;
    bool id_card_num;
    bool passport_num;
    bool ssn_id_num;
    bool vat_id_num;
    bool email;
    bool notify_email;
    bool telephone;
    bool fax;
    bool permanent;
    bool mailing;
    bool billing;
    bool shipping;
    bool shipping2;
    bool shipping3;
};

struct UpdateTransferContact
{
    std::string name;
    Nullable<std::string> organization;
    Nullable<std::string> vat_reg_num;
    Nullable<Birthdate> birth_date;
    Nullable<std::string> vat_id_num;
    Address permanent;
    Nullable<Address> mailing;
    std::string email;
    Nullable<std::string> notify_email;
    std::string telephone;
    Nullable<std::string> fax;
};

struct ContactStateInfo
{
    ContactId contact_id;
    boost::posix_time::ptime mojeid_activation_datetime;
    Nullable<boost::gregorian::date> identification_date;
    Nullable<boost::gregorian::date> validation_date;
    Nullable<boost::gregorian::date> linked_date;
};

typedef std::vector<ContactStateInfo> ContactStateInfoList;

typedef std::vector<std::string> ContactHandleList;

struct Exception : std::exception
{
};

struct IdentificationFailed : Exception
{
    const char* what() const noexcept
    {
        return "IdentificationFailed";
    }
};

struct IdentificationAlreadyProcessed : Exception
{
    const char* what() const noexcept
    {
        return "IdentificationAlreadyProcessed";
    }
};

struct IdentificationAlreadyInvalidated : Exception
{
    const char* what() const noexcept
    {
        return "IdentificationAlreadyInvalidated";
    }
};

struct ContactChanged : Exception
{
    const char* what() const noexcept
    {
        return "ContactChanged";
    }
};

struct ObjectAdminBlocked : Exception
{
    const char* what() const noexcept
    {
        return "ObjectAdminBlocked";
    }
};

struct ObjectUserBlocked : Exception
{
    const char* what() const noexcept
    {
        return "ObjectUserBlocked";
    }
};

struct AlreadyMojeidContact : Exception
{
    const char* what() const noexcept
    {
        return "AlreadyMojeidContact";
    }
};

struct ObjectDoesntExist : Exception
{
    const char* what() const noexcept
    {
        return "ObjectDoesntExist";
    }
};

struct IdentificationRequestDoesntExist : Exception
{
    const char* what() const noexcept
    {
        return "IdentificationRequestDoesntExist";
    }
};

struct ValidationRequestExists : Exception
{
    const char* what() const noexcept
    {
        return "ValidationRequestExists";
    }
};

struct ValidationAlreadyProcessed : Exception
{
    const char* what() const noexcept
    {
        return "ValidationAlreadyProcessed";
    }
};

} // namespace Fred::Backend::MojeIdImplData
} // namespace Fred::Backend
} // namespace Fred

#endif
