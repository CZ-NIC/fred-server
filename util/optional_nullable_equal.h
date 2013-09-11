/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  optional and nullable values (of the same type) comparison
 */

#ifndef OPTIONAL_NULLABLE_EQUAL_H_454434343413
#define OPTIONAL_NULLABLE_EQUAL_H_454434343413

#include "util/optional_value.h"
#include "db/nullable.h"
#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/if.hpp>
/* WARNING: for unknown reason the function works only when boost/lambda/bind.hpp is included.
 * Otherwise type of T in boost::lambda::if_then_else_return is unknown and therefore no operator==() can be called.
 * Based on stack-overflow discussion: http://stackoverflow.com/questions/2559246/help-with-boostlambda-expression
 * I suspect that boost/bind is used if boost/lambda/bind is not included.
 */
#include <boost/lambda/bind.hpp>

/**
 * compares Nullable<T> and Optional<T>
 * Suitable for those situations when var IS NULL <==> var IS NOT SET (in the "if and only if" meaning)
 * @param T Underlying type for both template specialization (obviously must be the same).
 * @param comparator Functor for custom comparison on uderlying type (e. g. way to insensitive comparison of std::strings). Default argument uses the type T operator==().
 */

template<class T>
bool equal( const Nullable<T>& lhs,
            const Optional<T>& rhs,
            boost::function<bool(const T&, const T&) > comparator
                = boost::function<bool(const T&, const T&) >(boost::lambda::if_then_else_return(boost::lambda::_1 == boost::lambda::_2, true, false)) ) {
    if( lhs.isnull() && !rhs.isset() ) {
        return true;
    }
    if( !lhs.isnull() && rhs.isset() ) {
        return comparator(static_cast<T>(lhs), rhs.get_value());
    }
    return false;
}

/**
 * compares Optional<T> and Nullable<T>
 * Identical to the Nullable<T> == Optional<T> version.
 */
template<class T>
bool equal( const Optional<T>& lhs,
            const Nullable<T>& rhs,
            boost::function<bool(const T&, const T&) > comparator
                = boost::function<bool(const T&, const T&) >(boost::lambda::if_then_else_return(boost::lambda::_1 == boost::lambda::_2, true, false)) ) {
    if( rhs.isnull() && !lhs.isset() ) {
        return true;
    }
    if( !rhs.isnull() && lhs.isset() ) {
        return comparator(static_cast<T>(rhs), lhs.get_value());
    }
    return false;
}

#endif // #include guard end

