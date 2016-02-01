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
#include "util/db/nullable.h"
#include "src/corba/MojeID.hh"
#include "src/mojeid/mojeid_impl_data.h"

namespace CorbaConversion
{
    namespace Internal
    {
        struct SafeStorageNotEmpty:std::runtime_error
        {
            SafeStorageNotEmpty():std::runtime_error("Storage has to be NULL initialized.") { }
            virtual ~SafeStorageNotEmpty() throw() { }
        };

        template < class DST_TYPE >
        void check_empty_storage(const DST_TYPE *const &safe_dst)
        {
            if (safe_dst != NULL) {
                throw SafeStorageNotEmpty();
            }
        }

        template < class SRC_TYPE, class DST_TYPE >
        struct into_safe_storage
        {
            static void wrap(const SRC_TYPE &src, DST_TYPE *&safe_dst)
            {
                check_empty_storage(safe_dst);
                safe_dst = new DST_TYPE;
                CorbaConversion::wrap(src, *safe_dst);
            }
        };

        template < >
        struct into_safe_storage< std::string, char >
        {
            static void wrap(const std::string &src, char *&safe_dst);
        };

        template < class SRC_TYPE, class DST_TYPE >
        static void wrap_into_safe_storage(const SRC_TYPE &src, DST_TYPE *&safe_dst)
        {
            into_safe_storage< SRC_TYPE, DST_TYPE >::wrap(src, safe_dst);
        }

        template < class SRC_TYPE, class DST_TYPE >
        static void wrap_nullable_into_safe_storage(const Nullable< SRC_TYPE > &src, DST_TYPE *&safe_dst)
        {
            if (!src.isnull()) {
                into_safe_storage< SRC_TYPE, DST_TYPE >::wrap(src.get_value(), safe_dst);
            }
            else {
                check_empty_storage(safe_dst);
            }
        }
    }//namespace CorbaConversion::Internal

    template < class SRC_TYPE, class DST_HOLDER_TYPE >
    void wrap_into_holder(const SRC_TYPE &src, DST_HOLDER_TYPE &dst)
    {
        Internal::wrap_into_safe_storage(src, dst.out());
    }

    template < class SRC_TYPE, class DST_HOLDER_TYPE >
    void wrap_nullable_into_holder(const Nullable< SRC_TYPE > &src, DST_HOLDER_TYPE &dst)
    {
        Internal::wrap_nullable_into_safe_storage(src, dst.out());
    }

    template < class SRC_TYPE, class DST_NULLABLE_TYPE >
    struct Wrapper_value_into_Nullable
    {
        typedef SRC_TYPE          NON_CORBA_TYPE;
        typedef DST_NULLABLE_TYPE CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
        {
            CorbaConversion::wrap(src, dst._boxed_inout());
        }
    };

    template < class EXCEPTION_CLASS, class SRC_TYPE >
    struct DEFAULT_WRAPPER_FAILURE;

    /**
     * Wraps @param src into EXCEPTION_CLASS instance and throws it.
     * @throw EXCEPTION_CLASS if conversion is successful
     * @throw DEFAULT_WRAPPER_FAILURE< EXCEPTION_CLASS, SRC_TYPE >::exception if conversion failed
     */
    template < class EXCEPTION_CLASS, class SRC_TYPE >
    void raise(const SRC_TYPE &src) __attribute__ ((__noreturn__));

    template < class EXCEPTION_CLASS, class SRC_TYPE >
    void raise(const SRC_TYPE &src)
    {
        EXCEPTION_CLASS e;
        try {
            CorbaConversion::wrap(src, e);
        }
        catch (...) {
            throw typename DEFAULT_WRAPPER_FAILURE< EXCEPTION_CLASS, SRC_TYPE >::exception();
        }
        throw e;
    };

    template < class EXCEPTION_CLASS, class SRC_TYPE >
    struct DEFAULT_WRAPPER_FAILURE
    {
        typedef Registry::MojeID::Server::INTERNAL_SERVER_ERROR exception;
    };


    template < class SRC_HOLDER_TYPE, class DST_TYPE >
    void unwrap_holder(const SRC_HOLDER_TYPE &src, DST_TYPE &dst)
    {
        unwrap(src.in(), dst);
    }

    template < class SRC_TYPE_PTR, class DST_TYPE >
    struct Unwrapper_ptr_into_Nullable
    {
        typedef SRC_TYPE_PTR*        CORBA_TYPE;
        typedef Nullable< DST_TYPE > NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE src_ptr, NON_CORBA_TYPE &dst)
        {
            if (src_ptr == NULL) {
                dst = NON_CORBA_TYPE();
            }
            else {
                dst = unwrap_into< DST_TYPE >(src_ptr->_boxed_in());
            }
        }
    };

    template < class NON_CORBA_CONTAINER, class CORBA_SEQ >
    struct Wrapper_std_vector_into_Seq_of_refs
    {
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;
        typedef CORBA_SEQ           CORBA_TYPE;

        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
        {
            dst.length(src.size());
            ::size_t dst_idx = 0;
            for (typename NON_CORBA_CONTAINER::const_iterator src_ptr = src.begin() ; src_ptr != src.end();
                 ++src_ptr, ++dst_idx)
            {
                CorbaConversion::wrap(*src_ptr, dst[dst_idx]);
            }
        }
    };

    template < class NON_CORBA_CONTAINER, class CORBA_SEQ >
    struct Wrapper_std_vector_into_Seq_of_holders
    {
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;
        typedef CORBA_SEQ           CORBA_TYPE;

        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
        {
            dst.length(src.size());
            ::size_t dst_idx = 0;
            for (typename NON_CORBA_CONTAINER::const_iterator src_ptr = src.begin() ; src_ptr != src.end();
                 ++src_ptr, ++dst_idx)
            {
                CorbaConversion::wrap(*src_ptr, dst[dst_idx].out());
            }
        }
    };

    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableString*, Nullable< std::string > >
    :   Unwrapper_ptr_into_Nullable< Registry::MojeID::NullableString, std::string > { };

    template < >
    struct DEFAULT_WRAPPER< std::string, Registry::MojeID::NullableString >
    :   Wrapper_value_into_Nullable< std::string, Registry::MojeID::NullableString > { };

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
    struct DEFAULT_WRAPPER< boost::gregorian::date, Registry::MojeID::Date >
    {
        typedef boost::gregorian::date NON_CORBA_TYPE;
        typedef Registry::MojeID::Date CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::Date, Registry::MojeIDImplData::Date >
    {
        typedef Registry::MojeID::Date         CORBA_TYPE;
        typedef Registry::MojeIDImplData::Date NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::Date, Registry::MojeID::Date >
    {
        typedef Registry::MojeIDImplData::Date NON_CORBA_TYPE;
        typedef Registry::MojeID::Date         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::NullableDate
    template < >
    struct DEFAULT_WRAPPER< boost::gregorian::date, Registry::MojeID::NullableDate >
    :   Wrapper_value_into_Nullable< boost::gregorian::date, Registry::MojeID::NullableDate > { };

    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableDate*, Nullable< Registry::MojeIDImplData::Date > >
    :   Unwrapper_ptr_into_Nullable< Registry::MojeID::NullableDate, Registry::MojeIDImplData::Date > { };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::Date, Registry::MojeID::NullableDate >
    :   Wrapper_value_into_Nullable< Registry::MojeIDImplData::Date, Registry::MojeID::NullableDate > { };


    //Registry::MojeID::DateTime
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::DateTime, boost::posix_time::ptime >
    {
        typedef Registry::MojeID::DateTime CORBA_TYPE;
        typedef boost::posix_time::ptime   NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< boost::posix_time::ptime, Registry::MojeID::DateTime >
    {
        typedef boost::posix_time::ptime   NON_CORBA_TYPE;
        typedef Registry::MojeID::DateTime CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };


    //Registry::MojeID::Address
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::Address, Registry::MojeIDImplData::Address >
    {
        typedef Registry::MojeID::Address         CORBA_TYPE;
        typedef Registry::MojeIDImplData::Address NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::Address, Registry::MojeID::Address >
    {
        typedef Registry::MojeIDImplData::Address NON_CORBA_TYPE;
        typedef Registry::MojeID::Address         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::NullableAddress
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableAddress*, Nullable< Registry::MojeIDImplData::Address > >
    :   Unwrapper_ptr_into_Nullable< Registry::MojeID::NullableAddress, Registry::MojeIDImplData::Address > { };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::Address, Registry::MojeID::NullableAddress >
    :   Wrapper_value_into_Nullable< Registry::MojeIDImplData::Address, Registry::MojeID::NullableAddress > { };


    //Registry::MojeID::ShippingAddress
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::ShippingAddress, Registry::MojeIDImplData::ShippingAddress >
    {
        typedef Registry::MojeID::ShippingAddress         CORBA_TYPE;
        typedef Registry::MojeIDImplData::ShippingAddress NON_CORBA_TYPE;
        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddress, Registry::MojeID::ShippingAddress >
    {
        typedef Registry::MojeIDImplData::ShippingAddress NON_CORBA_TYPE;
        typedef Registry::MojeID::ShippingAddress         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::NullableShippingAddress
    template < >
    struct DEFAULT_UNWRAPPER< Registry::MojeID::NullableShippingAddress*,
                              Nullable< Registry::MojeIDImplData::ShippingAddress > >
    :   Unwrapper_ptr_into_Nullable< Registry::MojeID::NullableShippingAddress,
                                     Registry::MojeIDImplData::ShippingAddress > { };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddress, Registry::MojeID::NullableShippingAddress >
    :   Wrapper_value_into_Nullable< Registry::MojeIDImplData::ShippingAddress, Registry::MojeID::NullableShippingAddress > { };


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
                            Registry::MojeID::AddressValidationResult >
    {
        typedef Registry::MojeIDImplData::AddressValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::AddressValidationResult         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::MandatoryAddressValidationResult
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::MandatoryAddressValidationResult,
                            Registry::MojeID::MandatoryAddressValidationResult >
    {
        typedef Registry::MojeIDImplData::MandatoryAddressValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::MandatoryAddressValidationResult         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::ShippingAddressValidationResult
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ShippingAddressValidationResult,
                            Registry::MojeID::ShippingAddressValidationResult >
    {
        typedef Registry::MojeIDImplData::ShippingAddressValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::ShippingAddressValidationResult         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };


    //Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::MessageLimitExceeded,
                            Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED >
    {
        typedef Registry::MojeIDImplData::MessageLimitExceeded   NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::RegistrationValidationResult,
                            Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::RegistrationValidationResult  NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::UpdateContactPrepareValidationResult,
                            Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::UpdateContactPrepareValidationResult    NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::CreateValidationRequestValidationResult,
                            Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::CreateValidationRequestValidationResult    NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    //Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR
    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ProcessRegistrationValidationResult,
                            Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR >
    {
        typedef Registry::MojeIDImplData::ProcessRegistrationValidationResult NON_CORBA_TYPE;
        typedef Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };


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
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::InfoContact, Registry::MojeID::InfoContact >
    {
        typedef Registry::MojeIDImplData::InfoContact NON_CORBA_TYPE;
        typedef Registry::MojeID::InfoContact         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ContactStateInfo, Registry::MojeID::ContactStateInfo >
    {
        typedef Registry::MojeIDImplData::ContactStateInfo NON_CORBA_TYPE;
        typedef Registry::MojeID::ContactStateInfo         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::ContactStateInfoList,
                            Registry::MojeID::ContactStateInfoList >
    :   Wrapper_std_vector_into_Seq_of_refs< Registry::MojeIDImplData::ContactStateInfoList,
                                             Registry::MojeID::ContactStateInfoList > { };

    template < >
    struct DEFAULT_WRAPPER< std::string, Registry::MojeID::Buffer >
    :   Wrapper_container_into_OctetSeq< std::string, Registry::MojeID::Buffer > { };

    template < >
    struct DEFAULT_WRAPPER< std::string, char* >
    {
        typedef std::string NON_CORBA_TYPE;
        typedef char*       CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
        {
            dst = CORBA::string_dup(src.c_str());
        }
    };

    template < >
    struct DEFAULT_WRAPPER< std::vector< std::string >, Registry::MojeID::ContactHandleList >
    :   Wrapper_std_vector_into_Seq_of_holders< std::vector< std::string >,
                                                Registry::MojeID::ContactHandleList > { };
}//namespace CorbaConversion

#endif//MOJEID_CORBA_CONVERSION_H_e5b26622ca884604abf9cf49892b20d7
