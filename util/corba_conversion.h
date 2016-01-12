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
 *  declaration for CORBA conversion
 */

#ifndef CORBA_CONVERSION_H_4402374c71c24ba88a4dfc1886eeebb5
#define CORBA_CONVERSION_H_4402374c71c24ba88a4dfc1886eeebb5

#include <stdexcept>
#include <string>
#include <omniORB4/CORBA.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/integer_traits.hpp>

/**
 * CORBA conversions
 */
namespace CorbaConversion
{
    /**
     * Default conversion template from non-CORBA type into CORBA type.
     * Meant to be specialized for each source and destination type pair. It classifies both
     * template argument types as CORBA_TYPE and NON_CORBA_TYPE and contains static method like this:
     * void wrap(const NON_CORBA_TYPE&, CORBA_TYPE&)
     * @tparam SRC_TYPE is conversion source non-CORBA type
     * @tparam DST_TYPE is conversion destination CORBA type
     */
    template < class SRC_TYPE, class DST_TYPE > struct DEFAULT_WRAPPER;

    /**
     * Conversion given in template argument from non-CORBA type into CORBA type.
     * @tparam WRAPPER is a type with the same interface as specialization of DEFAULT_WRAPPER
     * @param src is source non-CORBA type instance
     * @return dst is destination CORBA type instance
     */
    template < typename WRAPPER >
    void wrap_by(const typename WRAPPER::NON_CORBA_TYPE &src, typename WRAPPER::CORBA_TYPE &dst)
    {
        WRAPPER::wrap(src, dst);
    }

    /**
     * Conversion given in template argument from non-CORBA type into CORBA type.
     * @tparam WRAPPER is a type with the same interface as specialization of DEFAULT_WRAPPER
     * @param src is source non-CORBA type instance
     * @return CORBA type instance
     */
    template < typename WRAPPER >
    typename WRAPPER::CORBA_TYPE wrap_into_by(const typename WRAPPER::NON_CORBA_TYPE &src)
    {
        typename WRAPPER::CORBA_TYPE dst;
        wrap_by< WRAPPER >(src, dst);
        return dst;
    }

    /**
     * Default conversion from non-CORBA type into CORBA type as specified in default
     * conversion template specialization for given pair of types.
     * @tparam SRC_TYPE is conversion non-CORBA source type
     * @tparam DST_TYPE is conversion CORBA destination type
     * @param src is source non-CORBA type instance
     * @param dst is destination CORBA type instance
     */
    template < class SRC_TYPE, class DST_TYPE >
    void wrap(const SRC_TYPE &src, DST_TYPE &dst)
    {
        typedef DEFAULT_WRAPPER< SRC_TYPE, DST_TYPE > WRAPPER;
        BOOST_MPL_ASSERT((boost::is_same< SRC_TYPE, typename WRAPPER::NON_CORBA_TYPE >));
        BOOST_MPL_ASSERT((boost::is_same< DST_TYPE, typename WRAPPER::CORBA_TYPE >));

        wrap_by< WRAPPER >(src, dst);
    }

    /**
     * Default conversion from non-CORBA type into CORBA type as specified in default
     * conversion template specialization for given pair of types.
     * @tparam SRC_TYPE is conversion non-CORBA source type
     * @tparam DST_TYPE is conversion CORBA destination type
     * @param src is source non-CORBA type instance
     * @return CORBA type instance
     */
    template < class DST_TYPE, class SRC_TYPE >
    DST_TYPE wrap_into(const SRC_TYPE &src)
    {
        DST_TYPE dst;
        wrap(src, dst);
        return dst;
    }

    /**
     * Default conversion template from CORBA type into non-CORBA type.
     * Meant to be specialized for each source and destination type pair. It classifies both
     * template argument types as CORBA_TYPE and NON_CORBA_TYPE and contains static method like this:
     * void unwrap(const CORBA_TYPE&, NON_CORBA_TYPE&)
     * @tparam SRC_TYPE is conversion source CORBA type
     * @tparam DST_TYPE is conversion destination non-CORBA type
     */
    template < class SRC_TYPE, class DST_TYPE > struct DEFAULT_UNWRAPPER;

    /**
     * Conversion given in template argument from CORBA type into non-CORBA type.
     * @tparam UNWRAPPER is a type with the same interface as specialization of DEFAULT_UNWRAPPER
     * @param src is source CORBA type instance
     * @return dst is destination non-CORBA type instance
     */
    template < typename UNWRAPPER >
    void unwrap_by(const typename UNWRAPPER::CORBA_TYPE &src, typename UNWRAPPER::NON_CORBA_TYPE &dst)
    {
        UNWRAPPER::unwrap(src, dst);
    }

    /**
     * Conversion given in template argument from CORBA type into non-CORBA type.
     * @tparam UNWRAPPER is a type with the same interface as specialization of DEFAULT_UNWRAPPER
     * @param src is source CORBA type instance
     * @return non-CORBA type instance
     */
    template < typename UNWRAPPER >
    typename UNWRAPPER::NON_CORBA_TYPE unwrap_into_by(const typename UNWRAPPER::CORBA_TYPE &src)
    {
        typename UNWRAPPER::NON_CORBA_TYPE dst;
        unwrap_by< UNWRAPPER >(src, dst);
        return dst;
    }

    /**
     * Default conversion from CORBA type into non-CORBA type as specified in default
     * conversion template specialization for given pair of types.
     * @tparam SRC_TYPE is conversion CORBA source type
     * @tparam DST_TYPE is conversion non-CORBA destination type
     * @param src is source CORBA type instance
     * @param dst is destination non-CORBA type instance
     */
    template < class SRC_TYPE, class DST_TYPE >
    void unwrap(const SRC_TYPE &src, DST_TYPE &dst)
    {
        typedef DEFAULT_UNWRAPPER< SRC_TYPE, DST_TYPE > UNWRAPPER;
        BOOST_MPL_ASSERT((boost::is_same< SRC_TYPE, typename UNWRAPPER::CORBA_TYPE >));
        BOOST_MPL_ASSERT((boost::is_same< DST_TYPE, typename UNWRAPPER::NON_CORBA_TYPE >));

        unwrap_by< UNWRAPPER >(src, dst);
    }

    /**
     * Default conversion from CORBA type into non-CORBA type as specified in default
     * conversion template specialization for given pair of types.
     * @tparam SRC_TYPE is conversion CORBA source type
     * @tparam DST_TYPE is conversion non-CORBA destination type
     * @param src is source CORBA type instance
     * @return non-CORBA type instance
     */
    template < class DST_TYPE, class SRC_TYPE >
    DST_TYPE unwrap_into(const SRC_TYPE &src)
    {
        DST_TYPE dst;
        unwrap(src, dst);
        return  dst;
    }

    /**
     * Exception if pointer is wrongly NULL
     */
    class PointerIsNULL : public std::invalid_argument
    {
    public:
        PointerIsNULL() : std::invalid_argument("pointer is NULL") {}
        virtual ~PointerIsNULL() throw() {}
    };

    //string
    struct Unwrapper_const_char_ptr_into_std_string
    {
        typedef const char* CORBA_TYPE;
        typedef std::string NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE src, NON_CORBA_TYPE &dst);
    };
    template < >
    struct DEFAULT_UNWRAPPER< const char*, std::string >
    :   Unwrapper_const_char_ptr_into_std_string { };

    //tmpl seq
    /**
     * generic implementation of setting CORBA sequence, previous content of @param cs will be discarded
     * from container with const_iterator, begin(), end() and size() member
     */
    template < typename ELEMENT_CONVERSION, class NON_CORBA_CONTAINER, class CORBA_SEQ >
    struct Wrapper_std_vector_into_Seq
    {
        typedef CORBA_SEQ           CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
        {
            dst.length(src.size());
            ::size_t dst_idx = 0;
            for (typename NON_CORBA_CONTAINER::const_iterator src_ptr = src.begin() ; src_ptr != src.end();
                 ++src_ptr, ++dst_idx)
            {
                dst[dst_idx] = wrap_into_by< ELEMENT_CONVERSION >(*src_ptr);
            }
        }
    };

    /**
     * generic implementation of setting non-CORBA container with clear(), reserve() and push_back() member
     * from CORBA sequence, previous content of @param ol will be discarded
     */
    template < typename ELEMENT_CONVERSION, class CORBA_SEQ, class NON_CORBA_CONTAINER >
    struct Unwrapper_Seq_into_std_vector
    {
        typedef CORBA_SEQ           CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst)
        {
            dst.clear();
            dst.reserve(src.length());
            for (::size_t src_idx = 0 ; src_idx < src.length(); ++src_idx) {
                dst.push_back(unwrap_into_by< ELEMENT_CONVERSION >(src[src_idx]));
            }
        }
    };

    template < class CORBA_SEQ, class NON_CORBA_CONTAINER,
               class NON_CORBA_CONTAINER_VALUE_TYPE = typename NON_CORBA_CONTAINER::value_type >
    struct Unwrapper_Seq_of_holders_into_std_vector
    {
        typedef CORBA_SEQ           CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst)
        {
            dst.clear();
            dst.reserve(src.length());
            for (::size_t src_idx = 0 ; src_idx < src.length(); ++src_idx) {
                NON_CORBA_CONTAINER_VALUE_TYPE element;
                CorbaConversion::unwrap(src[src_idx].in(), element);
                dst.push_back(element);
            }
        }
    };

    template < class CORBA_SEQ, class NON_CORBA_CONTAINER,
               class NON_CORBA_CONTAINER_VALUE_TYPE = typename NON_CORBA_CONTAINER::value_type >
    struct Unwrapper_Seq_of_refs_into_std_vector
    {
        typedef CORBA_SEQ           CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst)
        {
            dst.clear();
            dst.reserve(src.length());
            for (::size_t src_idx = 0 ; src_idx < src.length(); ++src_idx) {
                NON_CORBA_CONTAINER_VALUE_TYPE element;
                CorbaConversion::unwrap(src[src_idx], element);
                dst.push_back(element);
            }
        }
    };

    /**
     * Exception if allocbuf is unable to alocate memory
     */
    class AllocbufFailed : public std::invalid_argument
    {
    public:
        AllocbufFailed() : std::invalid_argument("cannot allocate the requested memory") {}
        virtual ~AllocbufFailed() throw() {}
    };

    /**
     * Convert from std::vector<unsigned char>, std::string or compatible into sequence<octet> based CORBA types
     */
    template < class NON_CORBA_CONTAINER, class CORBA_OCTET_SEQ >
    struct Wrapper_container_into_OctetSeq
    {
        typedef CORBA_OCTET_SEQ     CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void wrap(const NON_CORBA_TYPE &src, CORBA_TYPE &dst)
        {
            try {
                dst.length(src.size());
                if (!src.empty()) {
                    std::memcpy(dst.get_buffer(), &(src[0]), src.size());
                }
            }
            catch (...) {
                throw AllocbufFailed();
            }
        }
    };

    /**
     * Convert from sequence<octet> based CORBA types into std::vector<unsigned char>, std::string or compatible
     */
    template < class CORBA_OCTET_SEQ, class NON_CORBA_CONTAINER >
    struct Unwrapper_OctetSeq_into_container
    {
        typedef CORBA_OCTET_SEQ     CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap(const CORBA_TYPE &src, NON_CORBA_TYPE &dst)
        {
            dst.clear();
            if (0 < src.length()) {
                dst.reserve(src.length());
                dst.insert(dst.begin(), src.get_buffer(), src.get_buffer() + src.length());
            }
        }
    };




    /**
     * Converted value is out of range
     */
    class IntegralConversionOutOfRange : public std::invalid_argument
    {
    public:
        IntegralConversionOutOfRange() : std::invalid_argument("Converted value is out of range") {}
        explicit IntegralConversionOutOfRange(const std::string &msg) : std::invalid_argument(msg) {}
        virtual ~IntegralConversionOutOfRange() throw() {}
    };

    /**
     * Basic integral types conversion using boost numeric_cast
     */
    template < class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE >
    void integral_type_converter(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE &dst)
    {
        typedef boost::integer_traits< SOURCE_INTEGRAL_TYPE > source_integral_type_traits;
        typedef boost::integer_traits< TARGET_INTEGRAL_TYPE > target_integral_type_traits;

        BOOST_MPL_ASSERT_MSG(source_integral_type_traits::is_integral, source_type_have_to_be_integral, (SOURCE_INTEGRAL_TYPE));
        BOOST_MPL_ASSERT_MSG(target_integral_type_traits::is_integral, target_type_have_to_be_integral, (TARGET_INTEGRAL_TYPE));

        try {
            dst = boost::numeric_cast< TARGET_INTEGRAL_TYPE >(src);
        }
        catch (const boost::numeric::negative_overflow &e) {
            throw IntegralConversionOutOfRange(e.what());
        }
        catch (const boost::numeric::positive_overflow &e) {
            throw IntegralConversionOutOfRange(e.what());
        }
    }

    //common default convertors of integral types
    template < class CORBA_INTEGRAL_TYPE, class NON_CORBA_INTEGRAL_TYPE >
    struct Default_unwrapper_of_integral_types
    {
        typedef CORBA_INTEGRAL_TYPE     CORBA_TYPE;
        typedef NON_CORBA_INTEGRAL_TYPE NON_CORBA_TYPE;
        static void unwrap(CORBA_TYPE src, NON_CORBA_TYPE &dst)
        {
            integral_type_converter(src, dst);
        }
    };

    template < class NON_CORBA_INTEGRAL_TYPE, class CORBA_INTEGRAL_TYPE >
    struct Default_wrapper_of_integral_types
    {
        typedef CORBA_INTEGRAL_TYPE     CORBA_TYPE;
        typedef NON_CORBA_INTEGRAL_TYPE NON_CORBA_TYPE;
        static void wrap(NON_CORBA_TYPE src, CORBA_TYPE &dst)
        {
            integral_type_converter(src, dst);
        }
    };

//DEFAULT_WRAPPER< SRC_TYPE, DST_TYPE >
#define DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_WRAPPER(SRC_TYPE, DST_TYPE) \
    template < > struct DEFAULT_WRAPPER< SRC_TYPE, DST_TYPE > \
    : Default_wrapper_of_integral_types< SRC_TYPE, DST_TYPE > { }

//DEFAULT_UNWRAPPER< SRC_TYPE, DST_TYPE >
#define DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_UNWRAPPER(SRC_TYPE, DST_TYPE) \
    template < > struct DEFAULT_UNWRAPPER< SRC_TYPE, DST_TYPE > \
    : Default_unwrapper_of_integral_types< SRC_TYPE, DST_TYPE > { }

//DEFAULT_(UN)WRAPPER< SRC_TYPE, DST_TYPE >
#define DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, DST_TYPE) \
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_WRAPPER(SRC_TYPE, DST_TYPE); \
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_UNWRAPPER(SRC_TYPE, DST_TYPE)

//DEFAULT_(UN)WRAPPER< SRC_TYPE, any integer type >
#define DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(SRC_TYPE) \
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, bool);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, char);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, signed char);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, unsigned char);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, wchar_t);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, short);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, unsigned short);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, int);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, unsigned int);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, long);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, unsigned long);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, long long);\
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, unsigned long long)

//DEFAULT_(UN)WRAPPER< any integer type, any integer type >
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(bool);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(char);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(signed char);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(unsigned char);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(wchar_t);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(short);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(unsigned short);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(int);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(unsigned int);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(long);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(unsigned long);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(long long);
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(unsigned long long);

#undef DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS
#undef DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS
#undef DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_UNWRAPPER
#undef DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_WRAPPER
}

#endif//CORBA_CONVERSION_H_4402374c71c24ba88a4dfc1886eeebb5
