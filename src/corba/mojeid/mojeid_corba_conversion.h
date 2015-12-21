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
        struct ValidationError
        {
            enum EnumType
            {
                NOT_AVAILABLE,
                INVALID,
                REQUIRED
            };
        };

        struct AddressValidationError
        {
            Nullable<ValidationError::EnumType> street1;
            Nullable<ValidationError::EnumType> city;
            Nullable<ValidationError::EnumType> postal_code;
            Nullable<ValidationError::EnumType> country;
        };

        struct MandatoryAddressValidationError
        {
            Nullable<ValidationError::EnumType> address_presence;
            Nullable<ValidationError::EnumType> street1;
            Nullable<ValidationError::EnumType> city;
            Nullable<ValidationError::EnumType> postal_code;
            Nullable<ValidationError::EnumType> country;
        };

        struct ShippingAddressValidationError
        {
            Nullable<ValidationError::EnumType> street1;
            Nullable<ValidationError::EnumType> city;
            Nullable<ValidationError::EnumType> postal_code;
            Nullable<ValidationError::EnumType> country;
        };

        struct MessageLimitExceeded
        {
            boost::gregorian::date limit_expire_date;
            unsigned short limit_count;
            unsigned short limit_days;
        };

        struct RegistrationValidationError
        {
            Nullable<ValidationError::EnumType> username;
            Nullable<ValidationError::EnumType> first_name;
            Nullable<ValidationError::EnumType> last_name;
            Nullable<ValidationError::EnumType> birth_date;
            Nullable<ValidationError::EnumType> email;
            Nullable<ValidationError::EnumType> notify_email;
            Nullable<ValidationError::EnumType> phone;
            Nullable<ValidationError::EnumType> fax;
            Nullable<AddressValidationError> permanent;
            Nullable<AddressValidationError> mailing;
            Nullable<AddressValidationError> billing;
            Nullable<ShippingAddressValidationError> shipping;
            Nullable<ShippingAddressValidationError> shipping2;
            Nullable<ShippingAddressValidationError> shipping3;
        };

        struct UpdateContactPrepareValidationError
        {
            Nullable<ValidationError::EnumType> first_name;
            Nullable<ValidationError::EnumType> last_name;
            Nullable<ValidationError::EnumType> birth_date;
            Nullable<ValidationError::EnumType> email;
            Nullable<ValidationError::EnumType> notify_email;
            Nullable<ValidationError::EnumType> phone;
            Nullable<ValidationError::EnumType> fax;
            Nullable<AddressValidationError> permanent;
            Nullable<AddressValidationError> mailing;
            Nullable<AddressValidationError> billing;
            Nullable<ShippingAddressValidationError> shipping;
            Nullable<ShippingAddressValidationError> shipping2;
            Nullable<ShippingAddressValidationError> shipping3;
        };

        struct CreateValidationRequestValidationError
        {
            Nullable<ValidationError::EnumType> first_name;
            Nullable<ValidationError::EnumType> last_name;
            Nullable<MandatoryAddressValidationError> permanent;
            Nullable<ValidationError::EnumType> email;
            Nullable<ValidationError::EnumType> phone;
            Nullable<ValidationError::EnumType> notify_email;
            Nullable<ValidationError::EnumType> fax;
            Nullable<ValidationError::EnumType> ssn;
        };

        struct ProcessRegistrationValidationError
        {
            Nullable<ValidationError::EnumType> email;
            Nullable<ValidationError::EnumType> phone;
        };
    }
}

namespace CorbaConversion
{

    /**
     * Exception if argument value is not enum ValidationError value
     */
    class NotEnumValidationErrorValue : public std::invalid_argument
    {
    public:
        NotEnumValidationErrorValue() : std::invalid_argument(
            "argument value is not enum ValidationError value") {}
        virtual ~NotEnumValidationErrorValue() throw() {}
    };

    //Registry::MojeID::ValidationError
    struct Unwrapper_Registry_MojeID_ValidationError_into_Registry_MojeIDImplData_ValidationError_EnumType
    {
        typedef Registry::MojeID::ValidationError CORBA_TYPE;
        typedef Registry::MojeIDImplData::ValidationError::EnumType NON_CORBA_TYPE;
        static void unwrap( CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_ValidationError_into_Registry_MojeIDImplData_ValidationError_EnumType::CORBA_TYPE,
    Unwrapper_Registry_MojeID_ValidationError_into_Registry_MojeIDImplData_ValidationError_EnumType::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_ValidationError_into_Registry_MojeIDImplData_ValidationError_EnumType type;
    };

    struct Wrapper_Registry_MojeIDImplData_ValidationError_EnumType_into_Registry_MojeID_ValidationError
    {
        typedef Registry::MojeID::ValidationError CORBA_TYPE;
        typedef Registry::MojeIDImplData::ValidationError::EnumType NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_ValidationError_EnumType_into_Registry_MojeID_ValidationError::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_ValidationError_EnumType_into_Registry_MojeID_ValidationError::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ValidationError_EnumType_into_Registry_MojeID_ValidationError type;
    };


    //Registry::MojeID::NullableValidationError
    struct Unwrapper_Registry_MojeID_NullableValidationError_ptr_into_Registry_MojeIDImplData_Nullable_ValidationError_EnumType
    {
        typedef Registry::MojeID::NullableValidationError* CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::ValidationError::EnumType> NON_CORBA_TYPE;
        static void unwrap( CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableValidationError_ptr_into_Registry_MojeIDImplData_Nullable_ValidationError_EnumType::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableValidationError_ptr_into_Registry_MojeIDImplData_Nullable_ValidationError_EnumType::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableValidationError_ptr_into_Registry_MojeIDImplData_Nullable_ValidationError_EnumType type;
    };

    struct Wrapper_Registry_MojeIDImplData_Nullable_ValidationError_EnumType_into_Registry_MojeID_NullableValidationError_var
    {
        typedef Registry::MojeID::NullableValidationError_var CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::ValidationError::EnumType> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_Nullable_ValidationError_EnumType_into_Registry_MojeID_NullableValidationError_var::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_Nullable_ValidationError_EnumType_into_Registry_MojeID_NullableValidationError_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_Nullable_ValidationError_EnumType_into_Registry_MojeID_NullableValidationError_var type;
    };

    //Registry::MojeID::AddressValidationError
    struct Unwrapper_Registry_MojeID_AddressValidationError_into_Registry_MojeIDImplData_AddressValidationError
    {
        typedef Registry::MojeID::AddressValidationError CORBA_TYPE;
        typedef Registry::MojeIDImplData::AddressValidationError NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_AddressValidationError_into_Registry_MojeIDImplData_AddressValidationError::CORBA_TYPE,
    Unwrapper_Registry_MojeID_AddressValidationError_into_Registry_MojeIDImplData_AddressValidationError::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_AddressValidationError_into_Registry_MojeIDImplData_AddressValidationError type;
    };

    struct Wrapper_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_AddressValidationError_var
    {
        typedef Registry::MojeID::AddressValidationError_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::AddressValidationError NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_AddressValidationError_var::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_AddressValidationError_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_AddressValidationError_var type;
    };

    //Registry::MojeID::NullableAddressValidationError
    struct Unwrapper_Registry_MojeID_NullableAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_AddressValidationError
    {
        typedef Registry::MojeID::NullableAddressValidationError* CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::AddressValidationError> NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_AddressValidationError::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_AddressValidationError::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_AddressValidationError type;
    };

    struct Wrapper_Nullable_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_NullableAddressValidationError_var
    {
        typedef Registry::MojeID::NullableAddressValidationError_var CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::AddressValidationError> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_NullableAddressValidationError_var::NON_CORBA_TYPE,
    Wrapper_Nullable_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_NullableAddressValidationError_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_Registry_MojeIDImplData_AddressValidationError_into_Registry_MojeID_NullableAddressValidationError_var type;
    };

    //Registry::MojeID::MandatoryAddressValidationError
    struct Unwrapper_Registry_MojeID_MandatoryAddressValidationError_into_Registry_MojeIDImplData_MandatoryAddressValidationError
    {
        typedef Registry::MojeID::MandatoryAddressValidationError CORBA_TYPE;
        typedef Registry::MojeIDImplData::MandatoryAddressValidationError NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_MandatoryAddressValidationError_into_Registry_MojeIDImplData_MandatoryAddressValidationError::CORBA_TYPE,
    Unwrapper_Registry_MojeID_MandatoryAddressValidationError_into_Registry_MojeIDImplData_MandatoryAddressValidationError::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_MandatoryAddressValidationError_into_Registry_MojeIDImplData_MandatoryAddressValidationError type;
    };

    struct Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_MandatoryAddressValidationError_var
    {
        typedef Registry::MojeID::MandatoryAddressValidationError_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::MandatoryAddressValidationError NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_MandatoryAddressValidationError_var::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_MandatoryAddressValidationError_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_MandatoryAddressValidationError_var type;
    };

    //Registry::MojeID::NullableMandatoryAddressValidationError
    struct Unwrapper_Registry_MojeID_NullableMandatoryAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError
    {
        typedef Registry::MojeID::NullableMandatoryAddressValidationError* CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::MandatoryAddressValidationError> NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableMandatoryAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableMandatoryAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableMandatoryAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError type;
    };

    struct Wrapper_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_NullableMandatoryAddressValidationError_var
    {
        typedef Registry::MojeID::NullableMandatoryAddressValidationError_var CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::MandatoryAddressValidationError> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_NullableMandatoryAddressValidationError_var::NON_CORBA_TYPE,
    Wrapper_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_NullableMandatoryAddressValidationError_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_Registry_MojeIDImplData_MandatoryAddressValidationError_into_Registry_MojeID_NullableMandatoryAddressValidationError_var type;
    };

    //Registry::MojeID::ShippingAddressValidationError
    struct Unwrapper_Registry_MojeID_ShippingAddressValidationError_into_Registry_MojeIDImplData_ShippingAddressValidationError
    {
        typedef Registry::MojeID::ShippingAddressValidationError CORBA_TYPE;
        typedef Registry::MojeIDImplData::ShippingAddressValidationError NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_ShippingAddressValidationError_into_Registry_MojeIDImplData_ShippingAddressValidationError::CORBA_TYPE,
    Unwrapper_Registry_MojeID_ShippingAddressValidationError_into_Registry_MojeIDImplData_ShippingAddressValidationError::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_ShippingAddressValidationError_into_Registry_MojeIDImplData_ShippingAddressValidationError type;
    };

    struct Wrapper_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_ShippingAddressValidationError_var
    {
        typedef Registry::MojeID::ShippingAddressValidationError_var CORBA_TYPE;
        typedef Registry::MojeIDImplData::ShippingAddressValidationError NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_ShippingAddressValidationError_var::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_ShippingAddressValidationError_var::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_ShippingAddressValidationError_var type;
    };

    //Registry::MojeID::NullableShippingAddressValidationError
    struct Unwrapper_Registry_MojeID_NullableShippingAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError
    {
        typedef Registry::MojeID::NullableShippingAddressValidationError* CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::ShippingAddressValidationError> NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_Registry_MojeID_NullableShippingAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError::CORBA_TYPE,
    Unwrapper_Registry_MojeID_NullableShippingAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Registry_MojeID_NullableShippingAddressValidationError_ptr_into_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError type;
    };

    struct Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_NullableShippingAddressValidationError_var
    {
        typedef Registry::MojeID::NullableShippingAddressValidationError_var CORBA_TYPE;
        typedef Nullable<Registry::MojeIDImplData::ShippingAddressValidationError> NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_NullableShippingAddressValidationError_var::NON_CORBA_TYPE,
    Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_NullableShippingAddressValidationError_var::CORBA_TYPE>
    {
        typedef Wrapper_Nullable_Registry_MojeIDImplData_ShippingAddressValidationError_into_Registry_MojeID_NullableShippingAddressValidationError_var type;
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
    struct Wrapper_Registry_MojeIDImplData_RegistrationValidationError_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::RegistrationValidationError NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_RegistrationValidationError_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_RegistrationValidationError_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_RegistrationValidationError_into_Registry_MojeID_Server_REGISTRATION_VALIDATION_ERROR type;
    };

    //Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR
    struct Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationError_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::UpdateContactPrepareValidationError NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationError_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationError_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_UpdateContactPrepareValidationError_into_Registry_MojeID_Server_UPDATE_CONTACT_PREPARE_VALIDATION_ERROR type;
    };

    //Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR
    struct Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationError_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::CreateValidationRequestValidationError NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationError_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationError_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_CreateValidationRequestValidationError_into_Registry_MojeID_Server_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR type;
    };

    //Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR
    struct Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationError_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR
    {
        typedef Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        typedef Registry::MojeIDImplData::ProcessRegistrationValidationError NON_CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
    Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationError_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR::NON_CORBA_TYPE,
    Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationError_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR::CORBA_TYPE>
    {
        typedef Wrapper_Registry_MojeIDImplData_ProcessRegistrationValidationError_into_Registry_MojeID_Server_PROCESS_REGISTRATION_VALIDATION_ERROR type;
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
    }
}

namespace CorbaConversion
{
    //Registry::MojeID::CreateContact
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

}

#endif

