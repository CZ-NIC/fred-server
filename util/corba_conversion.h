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
        typedef DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE> DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::type::CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::type::NON_CORBA_TYPE>));

        DEFAULT_CONVERSION::type::unwrap(in, out);
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
        typedef DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE> DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::type::NON_CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::type::CORBA_TYPE>));

        DEFAULT_CONVERSION::type::wrap(in, out);
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
        typedef DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE> DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::type::NON_CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::type::CORBA_TYPE>));

        NON_CORBA_TYPE res;
        DEFAULT_CONVERSION::type::unwrap(in, res);
        return res;
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
        typedef DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE> DEFAULT_CONVERSION;
        BOOST_MPL_ASSERT((boost::is_same<CORBA_TYPE, typename DEFAULT_CONVERSION::type::CORBA_TYPE>));
        BOOST_MPL_ASSERT((boost::is_same<NON_CORBA_TYPE, typename DEFAULT_CONVERSION::type::NON_CORBA_TYPE>));

        CORBA_TYPE res;
        DEFAULT_CONVERSION::type::wrap(in, res);
        return res;
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

    //CORBA::String_var
    struct Unwrapper_String_var_into_std_string
    {
        typedef CORBA::String_var CORBA_TYPE;
        typedef std::string NON_CORBA_TYPE;
        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out);
    };
    template <> struct DEFAULT_UNWRAPPER<
        Unwrapper_String_var_into_std_string::CORBA_TYPE,
        Unwrapper_String_var_into_std_string::NON_CORBA_TYPE>
    {
        typedef Unwrapper_String_var_into_std_string type;
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

    //valuetype String_var
    template< typename CORBA_VALUETYPE_STRING_VAR_TYPE >
    struct Unwrapper_NullableString_var_into_Nullable_std_string
    {
        typedef CORBA_VALUETYPE_STRING_VAR_TYPE CORBA_TYPE;
        typedef Nullable<std::string> NON_CORBA_TYPE;

        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
        {
            if(ct_in.in() == NULL)
            {
                nct_out = Nullable<std::string>();
            }
            else
            {
                nct_out = Nullable<std::string>(ct_in.in()->_value());
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

}
#endif

