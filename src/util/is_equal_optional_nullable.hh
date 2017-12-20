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

#include <boost/algorithm/string.hpp>

#include "src/util/optional_value.hh"
#include "src/util/db/nullable.hh"


namespace Util
{
    /**
     * compares Nullable<T> and Optional<T>
     * IS NULL <==> var IS NOT SET (in the "if and only if" meaning)
     * operator== of type T is used when Nullable is NOT NULL and Optional is set
     * @param T Underlying type for both template specialization (obviously must be the same).
     */
    template<class T>
    bool is_equal( const Nullable<T>& lhs, const Optional<T>& rhs ) {
        if( lhs.isnull() && !rhs.isset() ) {
            return true;
        }
        if( !lhs.isnull() && rhs.isset() ) {
            return lhs.get_value() == rhs.get_value();
        }
        return false;
    }

    /**
     * compares Optional<T> and Nullable<T>
     * IS NULL <==> var IS NOT SET (in the "if and only if" meaning)
     * operator== of type T is used when Nullable is NOT NULL and Optional is set
     * @param T Underlying type for both template specialization (obviously must be the same).
     */
    template<class T>
    bool is_equal( const Optional<T>& lhs, const Nullable<T>& rhs ) {
        if( rhs.isnull() && !lhs.isset() ) {
            return true;
        }
        if( !rhs.isnull() && lhs.isset() ) {
            return lhs.get_value() == rhs.get_value();
        }
        return false;
    }

    /**
     * compares Nullable<T> parameters using operator== of type T.
     * @param T Underlying type for Nullable templates.
     * @return true when both sides are NULL
     * or result of type T operator== when both sides are NOT NULL
     * or false when one side is NULL and other is not
     */
    template<class T>
    bool is_equal( const Nullable<T>& lhs, const Nullable<T>& rhs ) {
        if( lhs.isnull() && rhs.isnull() ) {
            return true;
        }
        if( !lhs.isnull() && !rhs.isnull() ) {
            return lhs.get_value() == rhs.get_value();
        }
        return false;
    }

    /**
     * compares Nullable<T> parameters using boost::algorithm::to_upper_copy and operator== of type T.
     * @param T Underlying type for Nullable templates.
     * @return true when both sides are NULL
     * or result of type T operator== comparison of T converted using boost::algorithm::to_upper_copy when both sides are NOT NULL
     * or false when one side is NULL and other is not
     */
    template<class T>
    bool is_equal_upper( const Nullable<T>& lhs, const Nullable<T>& rhs ) {
        if( lhs.isnull() && rhs.isnull() ) {
            return true;
        }
        if( !lhs.isnull() && !rhs.isnull() ) {
            return boost::algorithm::to_upper_copy(lhs.get_value()) == boost::algorithm::to_upper_copy(rhs.get_value());
        }
        return false;
    }

    /**
     * compares Nullable<T> parameters using boost::algorithm::to_lower_copy and operator== of type T.
     * @param T Underlying type for Nullable templates.
     * @return true when both sides are NULL
     * or result of type T operator== comparison of T converted using boost::algorithm::to_lower_copy when both sides are NOT NULL
     * or false when one side is NULL and other is not
     */
    template<class T>
    bool is_equal_lower( const Nullable<T>& lhs, const Nullable<T>& rhs ) {
        if( lhs.isnull() && rhs.isnull() ) {
            return true;
        }
        if( !lhs.isnull() && !rhs.isnull() ) {
            return boost::algorithm::to_lower_copy(lhs.get_value()) == boost::algorithm::to_lower_copy(rhs.get_value());
        }
        return false;
    }

}
#endif // #include guard end

