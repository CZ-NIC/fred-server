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
 * Converts enum value to the corresponding database handle represented by string.
 * @tparam ENUM_TYPE autodeducted enum type
 * @param value an enum value which has to be converted to its string counterpart
 * @return matching string counterpart
 * @throw std::invalid_argument in case that conversion fails
 */
template <typename ENUM_TYPE>
std::string to_db_handle(ENUM_TYPE value);

/**
 * Converts database handle represented by string into its enum representation.
 * @tparam ENUM_HOST_TYPE type which hosts enum
 * @param db_handle string which has to be converted to its enum counterpart
 * @return matching enum counterpart
 * @throw std::invalid_argument in case that conversion fails
 */
template < typename ENUM_HOST_TYPE >
typename ENUM_HOST_TYPE::Enum from_db_handle(const std::string &db_handle);

/**
 * Helps to implement from_db_handle function using to_db_handle conversion function.
 * @tparam S type of source value
 * @tparam D type of destination value
 * @tparam items number of all possible destination values
 * @param src source value
 * @param set_of_results C array of all possible destination values
 * @param forward_transformation function which transforms D value into S value
 * @return matching counterpart value
 * @throw std::invalid_argument in case that conversion fails
 */
template <class S, class D, unsigned items>
D inverse_transformation(const S& src, const D (&set_of_results)[items], S (*forward_transformation)(D))
{
    const D* const end_of_results = set_of_results + items;
    for (const D* result_candidate_ptr = set_of_results; result_candidate_ptr < end_of_results; ++result_candidate_ptr)
    {
        if (forward_transformation(*result_candidate_ptr) == src)
        {
            return *result_candidate_ptr;
        }
    }
    throw std::invalid_argument("conversion does not exist");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//ENUM_CONVERSION_H_97D0943DD36E63B820D471ECB6C3D088
