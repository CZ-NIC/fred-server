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
*  header of Fred::Object::State class
*/
#ifndef ENUM_CONVERSION_STATE_H_97D0943DD36E63B820D471ECB6C3D088//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define ENUM_CONVERSION_STATE_H_97D0943DD36E63B820D471ECB6C3D088

#include <stdexcept>
#include <map>
#include <boost/thread/once.hpp>

///< conversion tools
namespace Conversion {
///< enum conversion tools
namespace Enums {

/**
 * Contains routine for one direction of conversion definition. It has to be specialized for each enum type.
 * @tparam ENUM_TYPE enum type which conversion this class operates
 */
template < typename ENUM_TYPE > struct tools_for;

/**
 * Use @ref tools_for< ENUM_TYPE > for definition of both conversion directions.
 * @tparam ENUM_TYPE enum type which this class operates
 */
template < typename ENUM_TYPE >
class operate
{
public:
    typedef ENUM_TYPE enum_type;///< enum type which this class operates

    /**
     * Converts string into enum.
     * @param _src string which has to be converted to its enum counterpart
     * @return matching enum counterpart
     * @throw std::invalid_argument in case that conversion fails
     */
    static enum_type into_enum(const std::string &_src)
    {
        boost::call_once(init_to_enum_direction, init_to_enum_direction_called_);
        const typename ToEnum::const_iterator ptr = to_enum_.find(_src);
        if (ptr != to_enum_.end()) {
            return ptr->second;
        }
        throw std::invalid_argument("invalid value '" + _src + "'");
    }

    /**
     * Converts enum into string.
     * @param _src enum which has to be converted to its string counterpart
     * @return matching string counterpart
     * @throw std::invalid_argument in case that conversion fails
     */
    static std::string into_string(enum_type _src)
    {
        boost::call_once(init_to_string_direction, init_to_string_direction_called_);
        const typename ToString::const_iterator ptr = to_string_.find(_src);
        if (ptr != to_string_.end()) {
            return ptr->second;
        }
        throw std::invalid_argument("invalid value");
    }
private:
    typedef std::map< std::string, enum_type > ToEnum;
    typedef std::map< enum_type, std::string > ToString;
    static boost::once_flag init_to_enum_direction_called_;
    static boost::once_flag init_to_string_direction_called_;
    static ToEnum to_enum_;
    static ToString to_string_;
    static void init_to_string_direction()
    {
        tools_for< enum_type >::define_enum_to_string_relation(set_string_representation_of_enum_value);
    }
    static void init_to_enum_direction()
    {
        boost::call_once(init_to_string_direction, init_to_string_direction_called_);
        for (typename ToString::const_iterator ptr = to_string_.begin(); ptr != to_string_.end(); ++ptr) {
            to_enum_.insert(std::make_pair(ptr->second, ptr->first));
        }
    }
    static void set_string_representation_of_enum_value(enum_type _key, const std::string &_value)
    {
        to_string_.insert(std::make_pair(_key, _value));
    }
};

/**
 * Converts enum into string.
 * @tparam ENUM_TYPE autodeducted enum type which this function operates
 * @param value enum which has to be converted to its string counterpart
 * @return matching string counterpart
 * @throw std::invalid_argument in case that conversion fails
 */
template < typename ENUM_TYPE >
std::string into_string(ENUM_TYPE value)
{
    return operate< ENUM_TYPE >::into_string(value);
}

/**
 * Converts string into enum.
 * @tparam ENUM_TYPE enum type which this function operates
 * @param value string which has to be converted to its enum counterpart
 * @return matching enum counterpart
 * @throw std::invalid_argument in case that conversion fails
 */
template < typename ENUM_TYPE >
ENUM_TYPE into(const std::string &value)
{
    return operate< ENUM_TYPE >::into_enum(value);
}

template < typename ET >
boost::once_flag operate< ET >::init_to_enum_direction_called_ = BOOST_ONCE_INIT;
template < typename ET >
boost::once_flag operate< ET >::init_to_string_direction_called_ = BOOST_ONCE_INIT;
template < typename ET >
typename operate< ET >::ToEnum operate< ET >::to_enum_;
template < typename ET >
typename operate< ET >::ToString operate< ET >::to_string_;

}//namespace Conversion::Enums
}//namespace Conversion

#endif//ENUM_CONVERSION_STATE_H_97D0943DD36E63B820D471ECB6C3D088
