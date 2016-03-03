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
*  header of enum to database handle conversions
*/
#ifndef ENUM_CONVERSION_H_97D0943DD36E63B820D471ECB6C3D088//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define ENUM_CONVERSION_H_97D0943DD36E63B820D471ECB6C3D088

#include <string>
#include <stdexcept>

///< conversion tools
namespace Conversion {
///< enum conversion tools
namespace Enums {

/**
 * Converts database handle represented by string into its enum representation.
 * @tparam ENUM_HOST_TYPE type which hosts enum
 * @param db_handle string which has to be converted to its enum counterpart
 * @return matching enum counterpart
 * @throw std::invalid_argument in case that conversion fails
 */
template < typename ENUM_HOST_TYPE >
inline typename ENUM_HOST_TYPE::Enum from_db_handle(const std::string &db_handle);

/**
 * Helps to implement from_db_handle function using to_db_handle conversion function.
 * @tparam ENUM enum type
 * @tparam ITEMS number of enum items
 * @param db_handle string which has to be converted to its enum counterpart
 * @param values array of all enum items
 * @param type_name name of enum type used for readable exception message
 * @return matching enum counterpart
 * @throw std::invalid_argument in case that conversion fails
 */
template < class ENUM, ::size_t ITEMS >
ENUM from_db_handle_impl(const std::string &db_handle,
                         const ENUM (&values)[ITEMS],
                         const char *type_name)
{
    for (::size_t idx = 0; idx < ITEMS; ++idx) {
        if (to_db_handle(values[idx]) == db_handle) {
            return values[idx];
        }
    }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to " + type_name);
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//ENUM_CONVERSION_H_97D0943DD36E63B820D471ECB6C3D088
