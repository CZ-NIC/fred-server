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
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableString*, Nullable< std::string > >
    :   Unwrapper_NullableString_ptr_into_Nullable_std_string< Registry::MojeID::NullableString* > { };

    template < >
    struct DEFAULT_WRAPPER< Nullable< std::string >, Registry::MojeID::NullableString_var >
    :   Wrapper_Nullable_std_string_into_NullableString_var<
            Registry::MojeID::NullableString, Registry::MojeID::NullableString_var > { };

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
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::Date, boost::gregorian::date >
    {
        typedef Registry::MojeID::Date CORBA_TYPE;
        typedef boost::gregorian::date NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< boost::gregorian::date, Registry::MojeID::Date_var >
    {
        typedef boost::gregorian::date     NON_CORBA_TYPE;
        typedef Registry::MojeID::Date_var CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::DateTime
    struct Unwrapper_Registry_MojeID_DateTime_into_boost_posix_time_ptime
    {
    };
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::DateTime, boost::posix_time::ptime >
    {
        typedef Registry::MojeID::DateTime CORBA_TYPE;
        typedef boost::posix_time::ptime   NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< boost::posix_time::ptime, Registry::MojeID::DateTime_var >
    {
        typedef boost::posix_time::ptime       NON_CORBA_TYPE;
        typedef Registry::MojeID::DateTime_var CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::NullableDate
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableDate*, Nullable< boost::gregorian::date > >
    {
        typedef Registry::MojeID::NullableDate    *CORBA_TYPE;
        typedef Nullable< boost::gregorian::date > NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Nullable< boost::gregorian::date >, Registry::MojeID::NullableDate_var >
    {
        typedef Nullable< boost::gregorian::date > NON_CORBA_TYPE;
        typedef Registry::MojeID::NullableDate_var CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::NullableBoolean
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableBoolean*, Nullable< bool > >
    {
        typedef Registry::MojeID::NullableBoolean* CORBA_TYPE;
        typedef Nullable< bool >                   NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Nullable< bool >, Registry::MojeID::NullableBoolean_var >
    {
        typedef Nullable< bool >                      NON_CORBA_TYPE;
        typedef Registry::MojeID::NullableBoolean_var CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };
}

namespace Registry
{
    namespace MojeIDImplData
    {
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
    }
}

namespace CorbaConversion
{
    //Registry::MojeID::Address
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::Address, Registry::MojeIDImplData::Address >
    {
        typedef Registry::MojeID::Address         CORBA_TYPE;
        typedef Registry::MojeIDImplData::Address NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::Address, Registry::MojeID::Address_var >
    {
        typedef Registry::MojeIDImplData::Address NON_CORBA_TYPE;
        typedef Registry::MojeID::Address_var     CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };


    //Registry::MojeID::NullableAddress
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableAddress*, Nullable< Registry::MojeIDImplData::Address > >
    {
        typedef Registry::MojeID::NullableAddress            *CORBA_TYPE;
        typedef Nullable< Registry::MojeIDImplData::Address > NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Nullable< Registry::MojeIDImplData::Address >, Registry::MojeID::NullableAddress_var >
    {
        typedef Nullable< Registry::MojeIDImplData::Address > NON_CORBA_TYPE;
        typedef Registry::MojeID::NullableAddress_var         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };
}

namespace Registry
{
    namespace MojeIDImplData
    {
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
    }
}

namespace CorbaConversion
{
    //Registry::MojeID::ShippingAddress
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::ShippingAddress, Registry::MojeIDImplData::ShippingAddress >
    {
        typedef Registry::MojeID::ShippingAddress         CORBA_TYPE;
        typedef Registry::MojeIDImplData::ShippingAddress NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddress, Registry::MojeID::ShippingAddress_var >
    {
        typedef Registry::MojeIDImplData::ShippingAddress NON_CORBA_TYPE;
        typedef Registry::MojeID::ShippingAddress_var     CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };


    //Registry::MojeID::NullableShippingAddress
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableShippingAddress*,
                              Nullable< Registry::MojeIDImplData::ShippingAddress > >
    {
        typedef Registry::MojeID::NullableShippingAddress            *CORBA_TYPE;
        typedef Nullable< Registry::MojeIDImplData::ShippingAddress > NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Nullable< Registry::MojeIDImplData::ShippingAddress >,
                            Registry::MojeID::NullableShippingAddress_var >
    {
        typedef Nullable< Registry::MojeIDImplData::ShippingAddress > NON_CORBA_TYPE;
        typedef Registry::MojeID::NullableShippingAddress_var         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
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
            unsigned short         limit_count;
            unsigned short         limit_days;
        };

        struct RegistrationValidationResult
        {
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
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ValidationResult, Registry::MojeID::ValidationResult >
    {
        typedef Registry::MojeIDImplData::ValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::ValidationResult         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };


    //Registry::MojeID::AddressValidationResult
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::AddressValidationResult,
                            Registry::MojeID::AddressValidationResult_var >
    {
        typedef Registry::MojeIDImplData::AddressValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::AddressValidationResult_var     CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::MandatoryAddressValidationResult
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::MandatoryAddressValidationResult,
                            Registry::MojeID::MandatoryAddressValidationResult_var >
    {
        typedef Registry::MojeIDImplData::MandatoryAddressValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::MandatoryAddressValidationResult_var     CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::ShippingAddressValidationResult
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddressValidationResult,
                            Registry::MojeID::ShippingAddressValidationResult_var >
    {
        typedef Registry::MojeIDImplData::ShippingAddressValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::ShippingAddressValidationResult_var     CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::MessageLimitExceeded,
                            Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED >
    {
        typedef Registry::MojeIDImplData::MessageLimitExceeded   NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::RegistrationValidationResult,
                            Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::RegistrationValidationResult  NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::UpdateContactPrepareValidationResult,
                            Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::UpdateContactPrepareValidationResult    NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::CreateValidationRequestValidationResult,
                            Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::CreateValidationRequestValidationResult    NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ProcessRegistrationValidationResult,
                            Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::ProcessRegistrationValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        /**
         * All exceptions gets translated to Registry::MojeID::Server::INTERNAL_SERVER_ERROR.
         * @throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR
         */
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };
}

namespace Registry
{
    namespace MojeIDImplData
    {
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
            unsigned long long                 id;
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
            unsigned long long                 contact_id;
            boost::posix_time::ptime           mojeid_activation_datetime;
            boost::gregorian::date             conditionally_identification_date;
            Nullable< boost::gregorian::date > identification_date;
            Nullable< boost::gregorian::date > validation_date;
            Nullable< boost::gregorian::date > linked_date;
        };
    }
}

namespace CorbaConversion
{
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::CreateContact, Registry::MojeIDImplData::CreateContact >
    {
        typedef Registry::MojeID::CreateContact         CORBA_TYPE;
        typedef Registry::MojeIDImplData::CreateContact NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::UpdateContact, Registry::MojeIDImplData::UpdateContact >
    {
        typedef Registry::MojeID::UpdateContact         CORBA_TYPE;
        typedef Registry::MojeIDImplData::UpdateContact NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::SetContact, Registry::MojeIDImplData::SetContact >
    {
        typedef Registry::MojeID::SetContact         CORBA_TYPE;
        typedef Registry::MojeIDImplData::SetContact NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::InfoContact, Registry::MojeID::InfoContact_var >
    {
        typedef Registry::MojeIDImplData::InfoContact NON_CORBA_TYPE;
        typedef Registry::MojeID::InfoContact_var     CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ContactStateInfo, Registry::MojeID::ContactStateInfo_var >
    {
        typedef Registry::MojeIDImplData::ContactStateInfo NON_CORBA_TYPE;
        typedef Registry::MojeID::ContactStateInfo_var     CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< std::vector< Registry::MojeIDImplData::ContactStateInfo >,
                            Registry::MojeID::ContactStateInfoList_var >
    :   Wrapper_std_vector_into_Seq_var<
            Wrapper_std_vector_into_Seq< DEFAULT_WRAPPER< Registry::MojeIDImplData::ContactStateInfo,
                                                          Registry::MojeID::ContactStateInfo_var >,
                                         std::vector< Registry::MojeIDImplData::ContactStateInfo >,
                                         Registry::MojeID::ContactStateInfoList >,
            Registry::MojeID::ContactStateInfoList_var > { };

    template < >
    struct DEFAULT_WRAPPER< std::string, Registry::MojeID::Buffer_var >
    :   Wrapper_container_into_OctetSeq_var< Registry::MojeID::Buffer,
                                             Registry::MojeID::Buffer_var,
                                             std::string > { };

    template <> struct DEFAULT_WRAPPER<std::vector<std::string>, Registry::MojeID::ContactHandleList_var>
    :   Wrapper_std_vector_into_Seq_var<
            Wrapper_std_vector_into_Seq< Wrapper_std_string_into_String_var,
                                         std::vector< std::string >,
                                         Registry::MojeID::ContactHandleList >,
            Registry::MojeID::ContactHandleList_var > {};
}

#endif//MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7
