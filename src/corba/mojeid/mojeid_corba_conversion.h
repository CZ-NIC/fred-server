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
    //class for wrapping into type which holds (owns and manages) true CORBA types declared in idl
    //example: true CORBA type name is Buffer
    //         so its holder types are Buffer_var, Buffer_member, Buffer_out
    //each of holder types offers method out() which returns reference to an "empty" storage where
    //is it safe to store CORBA object created via new operator
    //the crucial benefit is that I don't need any default wrappers for CORBA holder types, I only
    //have to define default wrapper for true CORBA type
    class corba_holder_type_wrapper
    {
    public:
        //obtains reference to safe storage from CORBA holder type
        //initializes it and
        //converts source value into it by using DEFAULT_CORBA_WRAPPER
        template < class SRC_TYPE, class CORBA_HOLDER_TYPE >
        static void wrap(const SRC_TYPE &src, CORBA_HOLDER_TYPE &dst)
        {
            wrap_into_result_of_out_method(src, dst.out());
        }

        //it's the same with one difference only: the source type offers nullable semantics
        template < class SRC_TYPE, class CORBA_HOLDER_TYPE >
        static void wrap(const Nullable< SRC_TYPE > &src, CORBA_HOLDER_TYPE &dst)
        {
            wrap_nullable_into_result_of_out_method(src, dst.out());
        }

        //exception class which signals that there is a reasonable suspicion of memory leaking
        struct DangerOfMemoryLeaking:std::runtime_error
        {
            DangerOfMemoryLeaking():std::runtime_error("There is a danger of memory leaking.") { }
            virtual ~DangerOfMemoryLeaking() throw() { }
        };
    private:
        //storage should be "empty" before an assignment
        //it's not necessary but it's cheap defensive check
        template < class DST_TYPE >
        static void check_readiness_for_assignment(const DST_TYPE *const &storage)
        {
            if (storage != NULL) {
                throw DangerOfMemoryLeaking();
            }
        }

        //operations on result of out method of CORBA holder types
        template < class SRC_TYPE, class DST_TYPE, bool = false >
        class result_of_out_method
        {
        public:
            //inits storage by new object pointer and sets it to the converted value via default wrapper
            static void init_and_set(const SRC_TYPE &src, DST_TYPE *&storage)
            {
                CorbaConversion::wrap(src, *init(storage));
            }
        private:
            static DST_TYPE* init(DST_TYPE *&storage)
            {
                check_readiness_for_assignment(storage);
                return storage = new DST_TYPE;
            }
        };

        //specialization for CORBA string which is represented by plain char pointer type and
        //which unfortunately deserves other handling
        template < bool X >
        class result_of_out_method< std::string, char, X >
        {
        public:
            static void init_and_set(const std::string &src, char *&storage);
        };

        //a way how to distinguish between common source type
        template < class SRC_TYPE, class DST_TYPE >
        static void wrap_into_result_of_out_method(const SRC_TYPE &src, DST_TYPE *&storage)
        {
            result_of_out_method< SRC_TYPE, DST_TYPE >::init_and_set(src, storage);
        }

        //and source type with nullable semantics
        template < class SRC_TYPE, class DST_TYPE >
        static void wrap_nullable_into_result_of_out_method(const Nullable< SRC_TYPE > &src, DST_TYPE *&storage)
        {
            if (!src.isnull()) {
                result_of_out_method< SRC_TYPE, DST_TYPE >::init_and_set(src.get_value(), storage);
            }
            else {
                check_readiness_for_assignment(storage);
            }
        }
    };

    //from instance of CORBA holder type autodeduces and call default wrapper for corresponding true CORBA type
    template < class SRC_TYPE, class CORBA_HOLDER_TYPE >
    void wrap_into_holder(const SRC_TYPE &src, CORBA_HOLDER_TYPE &dst)
    {
        corba_holder_type_wrapper::wrap(src, dst);
    }

    //it's the same with one difference only: the source type offers nullable semantics
    template < class SRC_TYPE, class CORBA_HOLDER_TYPE >
    void wrap_nullable_into_holder(const Nullable< SRC_TYPE > &src, CORBA_HOLDER_TYPE &dst)
    {
        corba_holder_type_wrapper::wrap(src, dst);
    }

    //looks like default wrapper class for wrapping common source type without nullable semantics
    //into CORBA valuetype using default wrapper for corresponding true CORBA type
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

    //its member type 'exception' is used to signal common wrapper conversion trouble
    template < class EXCEPTION_CLASS, class SRC_TYPE >
    struct DEFAULT_WRAPPER_FAILURE;

    /**
     * Wraps @param src into EXCEPTION_CLASS instance and throws it.
     * @throw EXCEPTION_CLASS if conversion is successful
     * @throw DEFAULT_WRAPPER_FAILURE< EXCEPTION_CLASS, SRC_TYPE >::exception if conversion failed
     * @note __noreturn__ doesn't mean "returns nothing" but "never returns"
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

    //in common cases is it INTERNAL_SERVER_ERROR
    template < class EXCEPTION_CLASS, class SRC_TYPE >
    struct DEFAULT_WRAPPER_FAILURE
    {
        typedef Registry::MojeID::Server::INTERNAL_SERVER_ERROR exception;
    };

    //calls default unwrapper for corresponding true CORBA type
    template < class SRC_HOLDER_TYPE, class DST_TYPE >
    void unwrap_holder(const SRC_HOLDER_TYPE &src, DST_TYPE &dst)
    {
        unwrap(src.in(), dst);
    }

    //looks like default unwrapper class for unwrapping CORBA valuetype represented by plain pointer
    //into common type gifted with nullable semantics
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

    //looks like default wrapper class for wrapping common vector type into CORBA sequence
    //using default wrapper for corresponding true CORBA type of sequence items where operator[]
    //of sequence returns reference to the true CORBA type of item
    //example:
    //    template < >
    //    struct DEFAULT_WRAPPER< Impl::VectorOfStructures,
    //                            IDL::SequenceOfStructures >
    //    :   Wrapper_std_vector_into_Seq_of_refs< Impl::VectorOfStructures,
    //                                             IDL::SequenceOfStructures > { };
    //    for each item of Impl::VectorOfStructures calls default wrapper< Impl::Structure, IDL::Structure >
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

    //looks like default wrapper class for wrapping common vector type into CORBA sequence
    //using default wrapper for corresponding true CORBA type of sequence items where method out()
    //of sequence item returns reference to the true CORBA type of item
    //example:
    //    template < >
    //    struct DEFAULT_WRAPPER< Impl::VectorOfStrings,
    //                            IDL::SequenceOfStrings >
    //    :   Wrapper_std_vector_into_Seq_of_holders< Impl::VectorOfStrings,
    //                                                IDL::SequenceOfStrings > { };
    //    for each item of Impl::VectorOfStructures calls default wrapper< Impl::Strings, char* >
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
    struct DEFAULT_WRAPPER< Registry::MojeIDImplData::Buffer, Registry::MojeID::Buffer >
    {
        typedef Registry::MojeIDImplData::Buffer NON_CORBA_TYPE;
        typedef Registry::MojeID::Buffer         CORBA_TYPE;
        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst);
    };

    template < >
    struct DEFAULT_WRAPPER< std::string, Registry::MojeID::BufferValue >
    :   Wrapper_container_into_OctetSeq< std::string, Registry::MojeID::BufferValue > { };

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
