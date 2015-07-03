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
*  header of general static set of primitive values
*/
#ifndef STATIC_SET_OF_VALUES_H_C1C328458A10DF1F8E4F257659783F52//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define STATIC_SET_OF_VALUES_H_C1C328458A10DF1F8E4F257659783F52

#include <boost/static_assert.hpp>

/**
 * General static set of values (collection of distinct values). Values must be of primitive types (usable as
 * template parameter). This set can be composed of up to 27 different elements.
 * @tparam VALUE_TYPE set based on values of this type
 * @tparam NO_VALUE special value out of allowed values range
 * @tparam v00 first item
 * @tparam v01 second item
 * @tparam v26 last item
 * @note Element with default value isn't member of set. The unbroken series of non-default values must
 * start from v00. Maximal number of elements is currently 27.
 * @section sample Sample of usage
 * @code{.cpp}
 * struct Numeric
 * {
 *     enum Value
 *     {
 *         ONE,
 *         TWO,
 *         THREE,
 *         NO_NUMERIC = -1
 *     };
 * };
 * 
 * typedef static_set_of_values< Numeric::Value, Numeric::NO_NUMERIC,
 *                               Numeric::ONE,
 *                               Numeric::TWO,
 *                               Numeric::THREE > set_of_numerics;
 * @endcode
 */
template < typename VALUE_TYPE, VALUE_TYPE NO_VALUE,
           VALUE_TYPE v00 = NO_VALUE, VALUE_TYPE v01 = NO_VALUE, VALUE_TYPE v02 = NO_VALUE,
           VALUE_TYPE v03 = NO_VALUE, VALUE_TYPE v04 = NO_VALUE, VALUE_TYPE v05 = NO_VALUE,
           VALUE_TYPE v06 = NO_VALUE, VALUE_TYPE v07 = NO_VALUE, VALUE_TYPE v08 = NO_VALUE,
           VALUE_TYPE v09 = NO_VALUE, VALUE_TYPE v10 = NO_VALUE, VALUE_TYPE v11 = NO_VALUE,
           VALUE_TYPE v12 = NO_VALUE, VALUE_TYPE v13 = NO_VALUE, VALUE_TYPE v14 = NO_VALUE,
           VALUE_TYPE v15 = NO_VALUE, VALUE_TYPE v16 = NO_VALUE, VALUE_TYPE v17 = NO_VALUE,
           VALUE_TYPE v18 = NO_VALUE, VALUE_TYPE v19 = NO_VALUE, VALUE_TYPE v20 = NO_VALUE,
           VALUE_TYPE v21 = NO_VALUE, VALUE_TYPE v22 = NO_VALUE, VALUE_TYPE v23 = NO_VALUE,
           VALUE_TYPE v24 = NO_VALUE, VALUE_TYPE v25 = NO_VALUE, VALUE_TYPE v26 = NO_VALUE >
struct static_set_of_values
{
    /**
     * Collection of rest items without the first one.
     */
    typedef static_set_of_values< VALUE_TYPE, NO_VALUE,
                                /*v00*/v01, v02, v03, v04, v05, v06, v07, v08,
                                  v09, v10, v11, v12, v13, v14, v15, v16, v17,
                                  v18, v19, v20, v21, v22, v23, v24, v25, v26 > tail;
    /**
     * Type of elements.
     */
    typedef VALUE_TYPE value_type;
    /**
     * Value of the first element.
     */
    static const value_type value = v00;
    /**
     * Total number of elements.
     */
    static const int count = tail::count + 1;
    /**
     * Auxiliary class for uniqueness checking.
     */
    template < value_type v = value, typename SET = tail >
    struct missing
    {
        typedef typename SET::tail tail;
        static const bool result = (v != SET::value) && missing< v, tail >::result;
    };
    /**
     * Specialization for empty set.
     */
    template < value_type v >
    struct missing< v, static_set_of_values< VALUE_TYPE, NO_VALUE > >
    {
        static const bool result = true;
    };
    /**
     * Set contains this value just once.
     */
    static const bool value_is_unique = missing< value, tail >::result;
    /**
     * Multiple values are forbidden.
     */
    BOOST_STATIC_ASSERT(( value_is_unique ));
};

/**
 * Specialization for empty set.
 */
template < typename VALUE_TYPE, VALUE_TYPE NO_VALUE >
struct static_set_of_values< VALUE_TYPE, NO_VALUE,
                             NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE,
                             NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE,
                             NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE,
                             NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE,
                             NO_VALUE, NO_VALUE, NO_VALUE >
{
    static const int  count           = 0;
    static const bool value_is_unique = true;
};

#endif//STATIC_SET_OF_VALUES_H_C1C328458A10DF1F8E4F257659783F52
