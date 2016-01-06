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

#include "util/db/nullable.h"
/**
 * CORBA conversions
 */
namespace CorbaConversion
{
    /**
     * Conversion given in template argument from CORBA type into non-CORBA type.
     * @tparam UNWRAP_IMPL is implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method compatible with: static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
     * @param in is input CORBA type instance
     * @return non-CORBA type instance
     */
    template<class UNWRAP_IMPL>
    typename UNWRAP_IMPL::NON_CORBA_TYPE unwrap_by( typename UNWRAP_IMPL::CORBA_TYPE const & in)
    {
        typename UNWRAP_IMPL::NON_CORBA_TYPE res;
        UNWRAP_IMPL::unwrap(in, res);
        return res;
    }

    /**
     * Conversion given in template argument from non-CORBA type into CORBA type.
     * @tparam WRAP_IMPL is implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method compatible with: static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
     * @param in is input non-CORBA type instance
     * @return CORBA type instance
     */
    template<class WRAP_IMPL>
    typename WRAP_IMPL::CORBA_TYPE wrap_by( typename WRAP_IMPL::NON_CORBA_TYPE const & in)
    {
        typename WRAP_IMPL::CORBA_TYPE res;
        WRAP_IMPL::wrap(in, res);
        return res;
    }

    /**
     * Default conversion traits template from non-CORBA type into CORBA type.
     * It's meant to be specialized for source and destination type for each conversion in typedef type.
     * Specialization with typedef type designates implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method compatible with: static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
     * @tparam NON_CORBA_TYPE is conversion source type
     * @tparam CORBA_TYPE is conversion destination type
     */
    template < class NON_CORBA_TYPE, class CORBA_TYPE > struct DEFAULT_WRAPPER;

    /**
     * Default conversion traits template from CORBA type into non-CORBA type.
     * Meant to be specialized for source and destination type for each conversion in typedef type.
     * Specialization with typedef type designates implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method compatible with: static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
     * @tparam CORBA_TYPE is conversion source type
     * @tparam NON_CORBA_TYPE is conversion destination type
     */
    template < class CORBA_TYPE, class NON_CORBA_TYPE > struct DEFAULT_UNWRAPPER;

    /**
     * Default conversion from CORBA type into non-CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam CORBA_TYPE is conversion source type
     * @tparam NON_CORBA_TYPE is conversion destination type
     * @param in is input CORBA type instance
     * @param out is output non-CORBA type instance
     */
    template <typename CORBA_TYPE, typename NON_CORBA_TYPE>
    void unwrap( CORBA_TYPE const & in, NON_CORBA_TYPE& out )
    {
        typedef typename DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE>::type DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::NON_CORBA_TYPE>));

        DEFAULT_CONVERSION::unwrap(in, out);
    }

    /**
     * Default conversion from non-CORBA type into CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam NON_CORBA_TYPE is conversion source type
     * @tparam CORBA_TYPE is conversion destination type
     * @param in is input non-CORBA type instance
     * @param out is output CORBA type instance
     */
    template <typename NON_CORBA_TYPE, typename CORBA_TYPE>
    void wrap( NON_CORBA_TYPE const & in, CORBA_TYPE& out )
    {
        typedef typename DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE>::type DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::NON_CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::CORBA_TYPE>));

        DEFAULT_CONVERSION::wrap(in, out);
    }

    /**
     * Default conversion from CORBA type into non-CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam NON_CORBA_TYPE is conversion destination type
     * @tparam CORBA_TYPE is conversion source type
     * @param in is input CORBA type instance
     * @return non-CORBA type instance
     */
    template <typename NON_CORBA_TYPE, typename CORBA_TYPE>
    NON_CORBA_TYPE unwrap_into( const CORBA_TYPE& in)
    {
        typedef typename DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE>::type DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::NON_CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::CORBA_TYPE>));

        return unwrap_by<DEFAULT_CONVERSION>(in);
    }

    /**
     * Default conversion from non-CORBA type into CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam CORBA_TYPE is conversion destination type
     * @tparam NON_CORBA_TYPE is conversion source type
     * @param in is input non-CORBA type instance
     * @return CORBA type instance
     */
    template <typename CORBA_TYPE, typename NON_CORBA_TYPE>
    CORBA_TYPE wrap_into (const NON_CORBA_TYPE& in)
    {
        typedef typename DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE>::type DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::NON_CORBA_TYPE>));

        return wrap_by<DEFAULT_CONVERSION>(in);
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
        static void unwrap( CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_const_char_ptr_into_std_string::CORBA_TYPE,
    Unwrapper_const_char_ptr_into_std_string::NON_CORBA_TYPE>
    {
        typedef Unwrapper_const_char_ptr_into_std_string type;
    };

    struct Wrapper_std_string_into_String_var
    {
        typedef CORBA::String_var CORBA_TYPE;
        typedef std::string NON_CORBA_TYPE;
        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out );
    };
    template <> struct DEFAULT_WRAPPER<
        Wrapper_std_string_into_String_var::NON_CORBA_TYPE,
        Wrapper_std_string_into_String_var::CORBA_TYPE>
    {
        typedef Wrapper_std_string_into_String_var type;
    };

    //valuetype string
    template< typename CORBA_VALUETYPE_STRING_PTR_TYPE >
    struct Unwrapper_NullableString_ptr_into_Nullable_std_string
    {
        typedef CORBA_VALUETYPE_STRING_PTR_TYPE CORBA_TYPE;
        typedef Nullable<std::string> NON_CORBA_TYPE;

        static void unwrap( CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
        {
            if(ct_in == NULL)
            {
                nct_out = Nullable<std::string>();
            }
            else
            {
                nct_out = unwrap_into<std::string>(ct_in->_value());
            }
        }
    };

    template< typename CORBA_VALUETYPE_STRING_TYPE, typename CORBA_VALUETYPE_STRING_VAR_TYPE >
    struct Wrapper_Nullable_std_string_into_NullableString_var
    {
        typedef CORBA_VALUETYPE_STRING_VAR_TYPE CORBA_TYPE;
        typedef Nullable<std::string> NON_CORBA_TYPE;

        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
        {
            if(nct_in.isnull())
            {
                ct_out = CORBA_VALUETYPE_STRING_VAR_TYPE();
            }
            else
            {
                ct_out = new CORBA_VALUETYPE_STRING_TYPE(nct_in.get_value().c_str());
            }
        }
    };

    //tmpl seq
    /**
     * generic implementation of setting CORBA sequence, previous content of @param cs will be discarded
     * from container with const_iterator, begin(), end() and size() member
     */
    template <class ELEMENT_CONVERSION, class NON_CORBA_CONTAINER, typename CORBA_SEQ>
    struct Wrapper_std_vector_into_Seq
    {
        typedef CORBA_SEQ CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
        {
            ct_out.length(nct_in.size());
            unsigned long long i = 0;
            for(typename NON_CORBA_CONTAINER::const_iterator ci = nct_in.begin() ; ci != nct_in.end(); ++ci,++i)
            {
                ct_out[i] = wrap_by<ELEMENT_CONVERSION>(*ci);
            }
        }
    };

    /**
     * generic implementation of setting non-CORBA container with clear(), reserve() and push_back() member
     * from CORBA sequence, previous content of @param ol will be discarded
     */
    template <class ELEMENT_CONVERSION, typename CORBA_SEQ, class NON_CORBA_CONTAINER>
    struct Unwrapper_Seq_into_std_vector
    {
        typedef CORBA_SEQ CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out )
        {
            nct_out.clear();
            nct_out.reserve(ct_in.length());
            for(unsigned long long i = 0 ; i < ct_in.length();++i)
            {
                nct_out.push_back(unwrap_by<ELEMENT_CONVERSION>(ct_in[i]));
            }

        }
    };

    //tmpl seq var
    template <class CORBA_SEQ_WRAPPER , typename CORBA_SEQ_VAR>
    struct Wrapper_std_vector_into_Seq_var
    {
        typedef CORBA_SEQ_VAR CORBA_TYPE;
        typedef typename CORBA_SEQ_WRAPPER::NON_CORBA_TYPE NON_CORBA_TYPE;

        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
        {
            ct_out = new typename CORBA_SEQ_WRAPPER::CORBA_TYPE;
            CORBA_SEQ_WRAPPER::wrap(nct_in, ct_out.inout());
        }
    };

    template <class CORBA_SEQ_UNWRAPPER, typename CORBA_SEQ_VAR>
    struct Unwrapper_Seq_var_into_std_vector
    {
        typedef CORBA_SEQ_VAR CORBA_TYPE;
        typedef typename CORBA_SEQ_UNWRAPPER::NON_CORBA_TYPE NON_CORBA_TYPE;

        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out )
        {
            CORBA_SEQ_UNWRAPPER::unwrap(ct_in.in(), nct_out);
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
     * Convert from std::vector<unsigned char>, std::string or compatible into sequence<octet> based CORBA _var types
     */
    template <typename CORBA_OCTET_SEQ, typename CORBA_OCTET_SEQ_VAR, typename NON_CORBA_CONTAINER>
    struct Wrapper_container_into_OctetSeq_var
    {
        typedef CORBA_OCTET_SEQ_VAR CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
        {
            if(nct_in.size() == 0)
            {
                ct_out = new CORBA_OCTET_SEQ;
            }
            else
            {
                CORBA::Octet *b = CORBA_OCTET_SEQ::allocbuf(nct_in.size());

                if(b == NULL)
                {
                    throw AllocbufFailed();
                }

                memcpy(b,&nct_in[0],nct_in.size());
                ct_out = new CORBA_OCTET_SEQ(nct_in.size(), nct_in.size(), b, true);
            }
        }
    };

    /**
     * Convert from sequence<octet> based CORBA types into std::vector<unsigned char>, std::string or compatible
     */
    template <typename NON_CORBA_CONTAINER, typename CORBA_OCTET_SEQ>
    struct Unwrapper_OctetSeq_into_container
    {
        typedef CORBA_OCTET_SEQ CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out )
        {
            nct_out = NON_CORBA_TYPE(
                reinterpret_cast<typename NON_CORBA_TYPE::value_type const *>(ct_in.get_buffer()),
                (reinterpret_cast<typename NON_CORBA_TYPE::value_type const *>(ct_in.get_buffer()) + ct_in.length())
            );
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
    template <typename SOURCE_INTEGRAL_TYPE, typename TARGET_INTEGRAL_TYPE>
    void integral_type_converter(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE &dst)
    {
        typedef boost::integer_traits<SOURCE_INTEGRAL_TYPE> source_integral_type_traits;
        typedef boost::integer_traits<TARGET_INTEGRAL_TYPE> target_integral_type_traits;

        BOOST_MPL_ASSERT_MSG( source_integral_type_traits::is_integral, source_type_have_to_be_integral, (SOURCE_INTEGRAL_TYPE));
        BOOST_MPL_ASSERT_MSG( target_integral_type_traits::is_integral, target_type_have_to_be_integral, (TARGET_INTEGRAL_TYPE));

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
        struct type
        {
            typedef CORBA_INTEGRAL_TYPE     CORBA_TYPE;
            typedef NON_CORBA_INTEGRAL_TYPE NON_CORBA_TYPE;
            static void unwrap(CORBA_TYPE src, NON_CORBA_TYPE &dst)
            {
                integral_type_converter(src, dst);
            }
        };
    };

    template < class NON_CORBA_INTEGRAL_TYPE, class CORBA_INTEGRAL_TYPE >
    struct Default_wrapper_of_integral_types
    {
        struct type
        {
            typedef CORBA_INTEGRAL_TYPE     CORBA_TYPE;
            typedef NON_CORBA_INTEGRAL_TYPE NON_CORBA_TYPE;
            static void wrap(NON_CORBA_TYPE src, CORBA_TYPE &dst)
            {
                integral_type_converter(src, dst);
            }
        };
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
    DEFAULT_SRC_INTEGER_TYPE_TO_DST_INTEGER_TYPE_CONVERTERS(SRC_TYPE, unsigned char);\
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
    DEFAULT_SRC_INTEGER_TYPE_TO_ANY_INTEGER_TYPE_CONVERTERS(unsigned char);
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
