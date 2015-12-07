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

namespace Conversion {
namespace Enums {

template < typename ENUM_TYPE > struct tools_for;

template < typename ENUM_TYPE, typename OTHER_TYPE = std::string >
struct into_from
{
    typedef ENUM_TYPE enum_type;
    typedef OTHER_TYPE other_type;
    static enum_type into_enum_from(const other_type &_src)
    {
        boost::call_once(other_to_enum_init, other_to_enum_init_called_);
        const typename OtherToEnum::const_iterator ptr = other_to_enum_.find(_src);
        if (ptr != other_to_enum_.end()) {
            return ptr->second;
        }
        throw std::invalid_argument("invalid value");
    }
    static other_type into_other_from(enum_type _src)
    {
        boost::call_once(enum_to_other_init, enum_to_other_init_called_);
        const typename EnumToOther::const_iterator ptr = enum_to_other_.find(_src);
        if (ptr != enum_to_other_.end()) {
            return ptr->second;
        }
        throw std::invalid_argument("invalid value");
    }
    typedef std::map< other_type, enum_type > OtherToEnum;
    typedef std::map< enum_type, other_type > EnumToOther;
    static boost::once_flag other_to_enum_init_called_;
    static boost::once_flag enum_to_other_init_called_;
    static OtherToEnum other_to_enum_;
    static EnumToOther enum_to_other_;
    static void enum_to_other_init()
    {
        tools_for< enum_type >::enum_to_other_init(enum_to_other);
    }
    static void other_to_enum_init()
    {
        boost::call_once(enum_to_other_init, enum_to_other_init_called_);
        for (typename EnumToOther::const_iterator ptr = enum_to_other_.begin(); ptr != enum_to_other_.end(); ++ptr) {
            other_to_enum_.insert(std::make_pair(ptr->second, ptr->first));
        }
    }
    static void enum_to_other(enum_type _first, const other_type &_second)
    {
        enum_to_other_.insert(std::make_pair(_first, _second));
    }
};

template < typename INTO, typename FROM >
INTO into(FROM value)
{
    return into_from< FROM, INTO >::into_other_from(value);
}

template < typename ET, typename OT >
boost::once_flag into_from< ET, OT >::other_to_enum_init_called_ = BOOST_ONCE_INIT;
template < typename ET, typename OT >
boost::once_flag into_from< ET, OT >::enum_to_other_init_called_ = BOOST_ONCE_INIT;
template < typename ET, typename OT >
typename into_from< ET, OT >::OtherToEnum into_from< ET, OT >::other_to_enum_;
template < typename ET, typename OT >
typename into_from< ET, OT >::EnumToOther into_from< ET, OT >::enum_to_other_;

}//namespace Conversion::Enums
}//namespace Conversion

#endif//ENUM_CONVERSION_STATE_H_97D0943DD36E63B820D471ECB6C3D088
