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

/**
 * CORBA conversions
 */
namespace CorbaConversion
{
    /**
     * Conversion given in template argument from CORBA type into non-CORBA type.
     * @tparam UNWRAP_IMPL is implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method: static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
     * @param ct is input CORBA type instance
     * @return non-CORBA type instance
     */
    template<class UNWRAP_IMPL>
    typename UNWRAP_IMPL::NON_CORBA_TYPE unwrap_by( typename UNWRAP_IMPL::CORBA_TYPE const & ct)
    {
        typename UNWRAP_IMPL::NON_CORBA_TYPE res;
        UNWRAP_IMPL::unwrap(ct, res);
        return res;
    }

    /**
     * Conversion given in template argument from non-CORBA type into CORBA type.
     * @tparam WRAP_IMPL is implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method: static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
     * @param nct is input non-CORBA type instance
     * @return CORBA type instance
     */
    template<class WRAP_IMPL>
    typename WRAP_IMPL::CORBA_TYPE wrap_by( typename WRAP_IMPL::NON_CORBA_TYPE const & nct)
    {
        typename WRAP_IMPL::CORBA_TYPE res;
        WRAP_IMPL::wrap(nct, res);
        return res;
    }

    /**
     * Default conversion traits template from non-CORBA type into CORBA type.
     * It's meant to be specialized for source and destination type for each conversion in typedef type.
     * Specialization with typedef type designates implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method: static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out)
     * @tparam NON_CORBA_TYPE is conversion source type
     * @tparam CORBA_TYPE is conversion destination type
     */
    template < class NON_CORBA_TYPE, class CORBA_TYPE > struct DEFAULT_WRAPPER;

    /**
     * Default conversion traits template from CORBA type into non-CORBA type.
     * Meant to be specialized for source and destination type for each conversion in typedef type.
     * Specialization with typedef type designates implementation class type with members:
     *  typedef CORBA_TYPE, typedef NON_CORBA_TYPE
     *  and method: static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
     * @tparam CORBA_TYPE is conversion source type
     * @tparam NON_CORBA_TYPE is conversion destination type
     */
    template < class CORBA_TYPE, class NON_CORBA_TYPE > struct DEFAULT_UNWRAPPER;

    /**
     * Default conversion from CORBA type into non-CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam CORBA_TYPE is conversion source type
     * @tparam NON_CORBA_TYPE is conversion destination type
     * @param ct is input CORBA type instance
     * @param nct is output non-CORBA type instance
     */
    template <typename CORBA_TYPE, typename NON_CORBA_TYPE>
    void unwrap( CORBA_TYPE const & ct, NON_CORBA_TYPE& nct )
    {
        DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE>::type::unwrap(ct, nct);
    }

    /**
     * Default conversion from non-CORBA type into CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam NON_CORBA_TYPE is conversion source type
     * @tparam CORBA_TYPE is conversion destination type
     * @param nct is input non-CORBA type instance
     * @param ct is output CORBA type instance
     */
    template <typename NON_CORBA_TYPE, typename CORBA_TYPE>
    void wrap( NON_CORBA_TYPE const & nct, CORBA_TYPE& ct )
    {
        DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE>::type::wrap(nct, ct);
    }

    /**
     * Default conversion from CORBA type into non-CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam NON_CORBA_TYPE is conversion destination type
     * @tparam CORBA_TYPE is conversion source type
     * @param ct is input CORBA type instance
     * @return non-CORBA type instance
     */
    template <typename NON_CORBA_TYPE, typename CORBA_TYPE>
    typename DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE>::type::NON_CORBA_TYPE unwrap_into( const CORBA_TYPE& ct)
    {
        typename DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE>::type::NON_CORBA_TYPE res;
        DEFAULT_UNWRAPPER<CORBA_TYPE, NON_CORBA_TYPE>::type::unwrap(ct, res);
        return res;
    }

    /**
     * Default conversion from non-CORBA type into CORBA type
     *  as specified in default conversion traits template specialization for given pair of types.
     * @tparam CORBA_TYPE is conversion destination type
     * @tparam NON_CORBA_TYPE is conversion source type
     * @param nct is input non-CORBA type instance
     * @return CORBA type instance
     */
    template <typename CORBA_TYPE, typename NON_CORBA_TYPE>
    typename DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE>::type::CORBA_TYPE wrap_into (const NON_CORBA_TYPE& nct)
    {
        typename DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE>::type::CORBA_TYPE res;
        DEFAULT_WRAPPER<NON_CORBA_TYPE, CORBA_TYPE>::type::wrap(nct, res);
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



}
#endif

