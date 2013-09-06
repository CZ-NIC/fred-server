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

namespace Util
{
    /**
     * compares Nullable<T> and Optional<T>
     * Suitable for those situations when var IS NULL <==> var IS NOT SET (in the "if and only if" meaning)
     * @param T Underlying type for both template specialization (obviously must be the same).
     */

    template<class T>
    bool is_equal( const Nullable<T>& lhs, const Optional<T>& rhs
    ) {
        if( lhs.isnull() && !rhs.isset() ) {
            return true;
        }
        if( !lhs.isnull() && rhs.isset() ) {
            return static_cast<T>(lhs) == rhs.get_value();
        }
        return false;
    }

    /**
     * compares Optional<T> and Nullable<T>
     * Identical to the Nullable<T> == Optional<T> version.
     */
    template<class T>
    bool is_equal( const Optional<T>& lhs, const Nullable<T>& rhs )
    {
        if( rhs.isnull() && !lhs.isset() ) {
            return true;
        }
        if( !rhs.isnull() && lhs.isset() ) {
            return static_cast<T>(lhs) == lhs.get_value();
        }
        return false;
    }
}
#endif // #include guard end

