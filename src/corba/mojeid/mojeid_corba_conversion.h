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

#ifndef MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7
#define MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7

#include "util/corba_conversion.h"
#include "src/corba/MojeID2.hh"

namespace CorbaConversion
{
    struct Unwrapper_Registry_MojeID_NullableString_ptr_into_Nullable_std_string
        : Unwrapper_NullableString_ptr_into_Nullable_std_string<
            Registry::MojeID::NullableString*> {};

    template <> struct DEFAULT_UNWRAPPER<Registry::MojeID::NullableString*, Nullable<std::string> >
    {
        typedef Unwrapper_Registry_MojeID_NullableString_ptr_into_Nullable_std_string type;
    };

    struct Wrapper_Nullable_std_string_into_Registry_MojeID_NullableString_var
        : Wrapper_Nullable_std_string_into_NullableString_var<
            Registry::MojeID::NullableString, Registry::MojeID::NullableString_var> {};

    template <> struct DEFAULT_WRAPPER<Nullable<std::string>, Registry::MojeID::NullableString_var>
    {
        typedef Wrapper_Nullable_std_string_into_Registry_MojeID_NullableString_var type;
    };

    /**
     * Exception if argument is special
     */
    class ArgumentIsSpecial : public std::invalid_argument
    {
    public:
        ArgumentIsSpecial() : std::invalid_argument("argument is special") {}
        virtual ~ArgumentIsSpecial() throw() {}
    };

    //Registry::MojeID::Date
    struct Unwrapper_Registry_MojeID_Date_into_boost_gregorian_date
    {
        typedef Registry::MojeID::Date CORBA_TYPE;
        typedef boost::gregorian::date NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_Date_into_boost_gregorian_date::CORBA_TYPE,
        Unwrapper_Registry_MojeID_Date_into_boost_gregorian_date::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_Date_into_boost_gregorian_date type;
    };

    struct Wrapper_boost_gregorian_date_into_Registry_MojeID_Date_var
    {
        typedef Registry::MojeID::Date_var CORBA_TYPE;
        typedef boost::gregorian::date NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_boost_gregorian_date_into_Registry_MojeID_Date_var::NON_CORBA_TYPE,
        Wrapper_boost_gregorian_date_into_Registry_MojeID_Date_var::CORBA_TYPE>
    {
        typedef Wrapper_boost_gregorian_date_into_Registry_MojeID_Date_var type;
    };

    //Registry::MojeID::DateTime
    struct Unwrapper_Registry_MojeID_DateTime_into_boost_posix_time_ptime
    {
        typedef Registry::MojeID::DateTime CORBA_TYPE;
        typedef boost::posix_time::ptime NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_DateTime_into_boost_posix_time_ptime::CORBA_TYPE,
        Unwrapper_Registry_MojeID_DateTime_into_boost_posix_time_ptime::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_DateTime_into_boost_posix_time_ptime type;
    };

    struct Wrapper_boost_posix_time_ptime_into_Registry_MojeID_DateTime_var
    {
        typedef Registry::MojeID::DateTime_var CORBA_TYPE;
        typedef boost::posix_time::ptime NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_boost_posix_time_ptime_into_Registry_MojeID_DateTime_var::NON_CORBA_TYPE,
        Wrapper_boost_posix_time_ptime_into_Registry_MojeID_DateTime_var::CORBA_TYPE>
    {
        typedef Wrapper_boost_posix_time_ptime_into_Registry_MojeID_DateTime_var type;
    };

    //Registry::MojeID::NullableDate
    struct Unwrapper_Registry_MojeID_NullableDate_ptr_into_Nullable_boost_gregorian_date
    {
        typedef Registry::MojeID::NullableDate* CORBA_TYPE;
        typedef Nullable<boost::gregorian::date> NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableDate_ptr_into_Nullable_boost_gregorian_date::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableDate_ptr_into_Nullable_boost_gregorian_date::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableDate_ptr_into_Nullable_boost_gregorian_date type;
    };

    struct Wrapper_Nullable_boost_gregorian_date_into_Registry_MojeID_NullableDate_var
    {
        typedef Registry::MojeID::NullableDate_var CORBA_TYPE;
        typedef Nullable<boost::gregorian::date> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_boost_gregorian_date_into_Registry_MojeID_NullableDate_var::NON_CORBA_TYPE,
    Wrapper_Nullable_boost_gregorian_date_into_Registry_MojeID_NullableDate_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_boost_gregorian_date_into_Registry_MojeID_NullableDate_var type;
    };

    //Registry::MojeID::NullableBoolean
    struct Unwrapper_Registry_MojeID_NullableBoolean_ptr_into_Nullable_bool
    {
        typedef Registry::MojeID::NullableBoolean* CORBA_TYPE;
        typedef Nullable<bool> NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableBoolean_ptr_into_Nullable_bool::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableBoolean_ptr_into_Nullable_bool::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableBoolean_ptr_into_Nullable_bool type;
    };

    struct Wrapper_Nullable_bool_into_Registry_MojeID_NullableBoolean_var
    {
        typedef Registry::MojeID::NullableBoolean_var CORBA_TYPE;
        typedef Nullable<bool> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_bool_into_Registry_MojeID_NullableBoolean_var::NON_CORBA_TYPE,
    Wrapper_Nullable_bool_into_Registry_MojeID_NullableBoolean_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_bool_into_Registry_MojeID_NullableBoolean_var type;
    };
}

namespace Registry
{
    namespace MojeIDImplData
    {
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
    }
}

namespace CorbaConversion
{
    //Registry::MojeID::Address
    struct Unwrapper_Registry_MojeID_Address_into_Registry_MojeIDImplData_Address
    {
        typedef Registry::MojeID::Address CORBA_TYPE;
        typedef Registry::MojeIDImplData::Address NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_Address_into_Registry_MojeIDImplData_Address::CORBA_TYPE,
        Unwrapper_Registry_MojeID_Address_into_Registry_MojeIDImplData_Address::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_Address_into_Registry_MojeIDImplData_Address type;
    };

    struct Wrapper_Registry_MojeIDImplData_Address_into_Registry_MojeID_Address_var
    {
        typedef Registry::MojeID::Address_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::Address NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_Registry_MojeIDImplData_Address_into_Registry_MojeID_Address_var::NON_CORBA_TYPE,
        Wrapper_Registry_MojeIDImplData_Address_into_Registry_MojeID_Address_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_Address_into_Registry_MojeID_Address_var type;
    };


    //Registry::MojeID::NullableAddress
    struct Unwrapper_Registry_MojeID_NullableAddress_ptr_into_Nullable_Registry_MojeIDImplData_Address
    {
        typedef Registry::MojeID::NullableAddress* CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::Address> NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableAddress_ptr_into_Nullable_Registry_MojeIDImplData_Address::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableAddress_ptr_into_Nullable_Registry_MojeIDImplData_Address::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableAddress_ptr_into_Nullable_Registry_MojeIDImplData_Address type;
    };

    struct Wrapper_Nullable_Registry_MojeIDImplData_Address_into_Registry_MojeID_NullableAddress_var
    {
        typedef Registry::MojeID::NullableAddress_var CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::Address> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_Registry_MojeIDImplData_Address_into_Registry_MojeID_NullableAddress_var::NON_CORBA_TYPE,
    Wrapper_Nullable_Registry_MojeIDImplData_Address_into_Registry_MojeID_NullableAddress_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_Registry_MojeIDImplData_Address_into_Registry_MojeID_NullableAddress_var type;
    };
}

namespace Registry
{
    namespace MojeIDImplData
    {
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
    }
}

namespace CorbaConversion
{
    //Registry::MojeID::ShippingAddress
    struct Unwrapper_Registry_MojeID_ShippingAddress_into_Registry_MojeIDImplData_ShippingAddress
    {
        typedef Registry::MojeID::ShippingAddress CORBA_TYPE;
        typedef Registry::MojeIDImplData::ShippingAddress NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_ShippingAddress_into_Registry_MojeIDImplData_ShippingAddress::CORBA_TYPE,
        Unwrapper_Registry_MojeID_ShippingAddress_into_Registry_MojeIDImplData_ShippingAddress::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_ShippingAddress_into_Registry_MojeIDImplData_ShippingAddress type;
    };

    struct Wrapper_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_ShippingAddress_var
    {
        typedef Registry::MojeID::ShippingAddress_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::ShippingAddress NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_ShippingAddress_var::NON_CORBA_TYPE,
        Wrapper_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_ShippingAddress_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_ShippingAddress_var type;
    };


    //Registry::MojeID::NullableShippingAddress
    struct Unwrapper_Registry_MojeID_NullableShippingAddress_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddress
    {
        typedef Registry::MojeID::NullableShippingAddress* CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::ShippingAddress> NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableShippingAddress_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddress::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableShippingAddress_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddress::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableShippingAddress_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddress type;
    };

    struct Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_NullableShippingAddress_var
    {
        typedef Registry::MojeID::NullableShippingAddress_var CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::ShippingAddress> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_NullableShippingAddress_var::NON_CORBA_TYPE,
    Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_NullableShippingAddress_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddress_into_Registry_MojeID_NullableShippingAddress_var type;
    };
}

namespace Registry
{
    namespace MojeIDImplData
    {
        typedef Registry::MojeID::ValidationResult ValidationResult;

        struct AddressValidationResult
        {
            ValidationResult street1;
            ValidationResult city;
            ValidationResult postal_code;
            ValidationResult country;
        };

        struct MandatoryAddressValidationResult
        {
            ValidationResult address_presence;
            ValidationResult street1;
            ValidationResult city;
            ValidationResult postal_code;
            ValidationResult country;
        };

        struct ShippingAddressValidationResult
        {
            ValidationResult street1;
            ValidationResult city;
            ValidationResult postal_code;
            ValidationResult country;
        };

        struct MessageLimitExceeded
        {
            boost::gregorian::date limit_expire_date;
            unsigned short limit_count;
            unsigned short limit_days;
        };

        struct RegistrationValidationResult
        {
            ValidationResult username;
            ValidationResult first_name;
            ValidationResult last_name;
            ValidationResult birth_date;
            ValidationResult email;
            ValidationResult notify_email;
            ValidationResult phone;
            ValidationResult fax;
            AddressValidationResult permanent;
            AddressValidationResult mailing;
            AddressValidationResult billing;
            ShippingAddressValidationResult shipping;
            ShippingAddressValidationResult shipping2;
            ShippingAddressValidationResult shipping3;
        };

        struct UpdateContactPrepareValidationResult
        {
            ValidationResult first_name;
            ValidationResult last_name;
            ValidationResult birth_date;
            ValidationResult email;
            ValidationResult notify_email;
            ValidationResult phone;
            ValidationResult fax;
            AddressValidationResult permanent;
            AddressValidationResult mailing;
            AddressValidationResult billing;
            ShippingAddressValidationResult shipping;
            ShippingAddressValidationResult shipping2;
            ShippingAddressValidationResult shipping3;
        };

        struct CreateValidationRequestValidationResult
        {
            ValidationResult first_name;
            ValidationResult last_name;
            MandatoryAddressValidationResult permanent;
            ValidationResult email;
            ValidationResult phone;
            ValidationResult notify_email;
            ValidationResult fax;
            ValidationResult ssn;
        };

        struct ProcessRegistrationValidationResult
        {
            ValidationResult email;
            ValidationResult phone;
        };
    }
}

namespace CorbaConversion
{

    /**
     * Exception if argument value is not enum ValidationResult value
     */
    class NotEnumValidationResultValue : public std::invalid_argument
    {
    public:
        NotEnumValidationResultValue() : std::invalid_argument(
            "argument value is not enum ValidationResult value") {}
        virtual ~NotEnumValidationResultValue() throw() {}
    };

    //Registry::MojeID::ValidationResult
    struct Wrapper_Registry_MojeIDImplData_ValidationResult_into_Registry_MojeID_ValidationResult
    {
        typedef Registry::MojeID::ValidationResult         CORBA_TYPE;
        typedef Registry::MojeIDImplData::ValidationResult NON_CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_ValidationResult_into_Registry_MojeID_ValidationResult::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_ValidationResult_into_Registry_MojeID_ValidationResult::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ValidationResult_into_Registry_MojeID_ValidationResult type;
    };


    //Registry::MojeID::AddressValidationResult
    struct Wrapper_Registry_MojeIDImplData_AddressValidationResult_into_Registry_MojeID_AddressValidationResult_var
    {
        typedef Registry::MojeID::AddressValidationResult_var     CORBA_TYPE;
        typedef Registry::MojeIDImplData::AddressValidationResult NON_CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_AddressValidationResult_into_Registry_MojeID_AddressValidationResult_var::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_AddressValidationResult_into_Registry_MojeID_AddressValidationResult_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_AddressValidationResult_into_Registry_MojeID_AddressValidationResult_var type;
    };

    //Registry::MojeID::MandatoryAddressValidationResult
    struct Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationResult_into_Registry_MojeID_MandatoryAddressValidationResult_var
    {
        typedef Registry::MojeID::MandatoryAddressValidationResult_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::MandatoryAddressValidationResult NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationResult_into_Registry_MojeID_MandatoryAddressValidationResult_var::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationResult_into_Registry_MojeID_MandatoryAddressValidationResult_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationResult_into_Registry_MojeID_MandatoryAddressValidationResult_var type;
    };

    //Registry::MojeID::ShippingAddressValidationResult
    struct Wrapper_Registry_MojeIDImplData_ShippingAddressValidationResult_into_Registry_MojeID_ShippingAddressValidationResult_var
    {
        typedef Registry::MojeID::ShippingAddressValidationResult_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::ShippingAddressValidationResult NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_ShippingAddressValidationResult_into_Registry_MojeID_ShippingAddressValidationResult_var::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_ShippingAddressValidationResult_into_Registry_MojeID_ShippingAddressValidationResult_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ShippingAddressValidationResult_into_Registry_MojeID_ShippingAddressValidationResult_var type;
    };

    //Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED
    struct Wrapper_Registry_MojeIDImplData_MessageLimitExceeded_into_Registry_MojeID_Server_MESSAGE_LIMIT_EXCEEDED
    {
        typedef Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED CORBA_TYPE;
        typedef Registry::MojeIDImplData::MessageLimitExceeded NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_MessageLimitExceeded_into_Registry_MojeID_Server_MESSAGE_LIMIT_EXCEEDED::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_MessageLimitExceeded_into_Registry_MojeID_Server_MESSAGE_LIMIT_EXCEEDED::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_MessageLimitExceeded_into_Registry_MojeID_Server_MESSAGE_LIMIT_EXCEEDED type;
    };

    //Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR
    struct Wrapper_Registry_MojeIDImplData_RegistrationValidationResult_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::RegistrationValidationResult NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_RegistrationValidationResult_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_RegistrationValidationResult_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_RegistrationValidationResult_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR type;
    };

    //Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR
    struct Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationResult_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::UpdateContactPrepareValidationResult NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationResult_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationResult_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationResult_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR type;
    };

    //Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR
    struct Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationResult_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::CreateValidationRequestValidationResult NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationResult_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationResult_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationResult_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR type;
    };

    //Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR
    struct Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationResult_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::ProcessRegistrationValidationResult NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationResult_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationResult_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationResult_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR type;
    };
}

namespace Registry
{
    namespace MojeIDImplData
    {
        struct CreateContact
        {
            std::string username;
            std::string first_name;
            std::string last_name;
            Nullable<std::string> organization;
            Nullable<std::string> vat_reg_num;
            Nullable<boost::gregorian::date> birth_date;
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
            unsigned long long id;
            std::string first_name;
            std::string last_name;
            Nullable<std::string> organization;
            Nullable<std::string> vat_reg_num;
            Nullable<boost::gregorian::date> birth_date;
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

        typedef UpdateContact InfoContact; ///< XXX

        struct SetContact
        {
            Nullable<std::string> organization;
            Nullable<std::string> vat_reg_num;
            Nullable<boost::gregorian::date> birth_date;
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
            unsigned long long contact_id;
            boost::posix_time::ptime mojeid_activation_datetime;
            boost::gregorian::date conditionally_identification_date;
            Nullable<boost::gregorian::date> identification_date;
            Nullable<boost::gregorian::date> validation_date;
            Nullable<boost::gregorian::date> linked_date;
        };
    }
}

namespace CorbaConversion
{
    struct Unwrapper_Registry_MojeID_CreateContact_into_Registry_MojeIDImplData_CreateContact
    {
        typedef Registry::MojeID::CreateContact CORBA_TYPE;
        typedef Registry::MojeIDImplData::CreateContact NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_CreateContact_into_Registry_MojeIDImplData_CreateContact::CORBA_TYPE,
        Unwrapper_Registry_MojeID_CreateContact_into_Registry_MojeIDImplData_CreateContact::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_CreateContact_into_Registry_MojeIDImplData_CreateContact type;
    };

    struct Unwrapper_Registry_MojeID_UpdateContact_into_Registry_MojeIDImplData_UpdateContact
    {
        typedef Registry::MojeID::UpdateContact CORBA_TYPE;
        typedef Registry::MojeIDImplData::UpdateContact NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_UpdateContact_into_Registry_MojeIDImplData_UpdateContact::CORBA_TYPE,
        Unwrapper_Registry_MojeID_UpdateContact_into_Registry_MojeIDImplData_UpdateContact::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_UpdateContact_into_Registry_MojeIDImplData_UpdateContact type;
    };

    struct Unwrapper_Registry_MojeID_SetContact_into_Registry_MojeIDImplData_SetContact
    {
        typedef Registry::MojeID::SetContact CORBA_TYPE;
        typedef Registry::MojeIDImplData::SetContact NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_Registry_MojeID_SetContact_into_Registry_MojeIDImplData_SetContact::CORBA_TYPE,
        Unwrapper_Registry_MojeID_SetContact_into_Registry_MojeIDImplData_SetContact::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_SetContact_into_Registry_MojeIDImplData_SetContact type;
    };

    struct Wrapper_Registry_MojeIDImplData_InfoContact_into_Registry_MojeID_InfoContact_var
    {
        typedef Registry::MojeID::InfoContact_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::InfoContact NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_Registry_MojeIDImplData_InfoContact_into_Registry_MojeID_InfoContact_var::NON_CORBA_TYPE,
        Wrapper_Registry_MojeIDImplData_InfoContact_into_Registry_MojeID_InfoContact_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_InfoContact_into_Registry_MojeID_InfoContact_var type;
    };

    struct Wrapper_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfo_var
    {
        typedef Registry::MojeID::ContactStateInfo_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::ContactStateInfo NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfo_var::NON_CORBA_TYPE,
        Wrapper_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfo_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfo_var type;
    };

    struct Wrapper_std_vector_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfoList_var
    : Wrapper_std_vector_into_Seq_var<
          Wrapper_std_vector_into_Seq<Wrapper_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfo_var,
              std::vector<Registry::MojeIDImplData::ContactStateInfo>, Registry::MojeID::ContactStateInfoList>,
          Registry::MojeID::ContactStateInfoList_var>
    {};

    template <> struct DEFAULT_WRAPPER<std::vector<Registry::MojeIDImplData::ContactStateInfo>, Registry::MojeID::ContactStateInfoList_var>
    {
        typedef Wrapper_std_vector_Registry_MojeIDImplData_ContactStateInfo_into_Registry_MojeID_ContactStateInfoList_var type;
    };

    struct Wrapper_std_string_into_Registry_MojeID_Buffer_var
    : Wrapper_container_into_OctetSeq_var<
      Registry::MojeID::Buffer, Registry::MojeID::Buffer_var, std::string>
    {};

    template <> struct DEFAULT_WRAPPER<std::string, Registry::MojeID::Buffer_var>
    {
        typedef Wrapper_std_string_into_Registry_MojeID_Buffer_var type;
    };

    struct Wrapper_std_vector_std_string_into_Registry_MojeID_ContactHandleList_var
    : Wrapper_std_vector_into_Seq_var<
      Wrapper_std_vector_into_Seq<Wrapper_std_string_into_String_var,
          std::vector<std::string>, Registry::MojeID::ContactHandleList> , Registry::MojeID::ContactHandleList_var>
    {};
    template <> struct DEFAULT_WRAPPER<std::vector<std::string>, Registry::MojeID::ContactHandleList_var>
    {
        typedef Wrapper_std_vector_std_string_into_Registry_MojeID_ContactHandleList_var type;
    };

}

#endif//MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7
