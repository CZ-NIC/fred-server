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
*  header of Fred::Object::Type class
*/

#ifndef OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922

#include "util/enum_conversion.h"

/// Fred matters
namespace Fred {
/// Fred objects matters
namespace Object {

typedef unsigned long long Id;

//boost/type_traits.hpp defines DOMAIN
#ifdef DOMAIN
#undef DOMAIN
#endif

/**
 * Bidirectional conversions between string and enum representation of object types.
 */
class Type
{
public:
    /**
     * Names of particular object types.
     */
    enum Value
    {
        CONTACT,///< object is contact
        NSSET,  ///< object is nsset
        DOMAIN, ///< object is domain
        KEYSET, ///< object is keyset
    };
    /**
     * String value converts to its enum equivalent.
     * @param _str database representation of object type
     * @return its enum equivalent
     * @throw std::invalid_argument if conversion is impossible
     */
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into_from< Value >::into_enum_from(_str);
    }
};

}//Fred::Object
}//Fred

namespace Conversion {
namespace Enums {

template < >
struct tools_for< Fred::Object::Type::Value >
{
    static void enum_to_other_init(void (*enum_to_other_set)(Fred::Object::Type::Value, const std::string&))
    {
        using Fred::Object::Type;
        enum_to_other_set(Type::CONTACT, "contact");
        enum_to_other_set(Type::NSSET,   "nsset");
        enum_to_other_set(Type::DOMAIN,  "domain");
        enum_to_other_set(Type::KEYSET,  "keyset");
    }
};

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922
