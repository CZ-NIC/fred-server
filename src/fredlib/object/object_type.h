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
    enum Enum
    {
        CONTACT,///< object is contact
        NSSET,  ///< object is nsset
        DOMAIN, ///< object is domain
        KEYSET, ///< object is keyset
    };
};

}//Fred::Object
}//Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Fred::Object::Type::Enum value)
{
    switch (value)
    {
        case Fred::Object::Type::CONTACT: return "contact";
        case Fred::Object::Type::NSSET:   return "nsset";
        case Fred::Object::Type::DOMAIN:  return "domain";
        case Fred::Object::Type::KEYSET:  return "keyset";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Object::Type::Enum");
};

template < >
inline Fred::Object::Type::Enum from_db_handle< Fred::Object::Type >(const std::string &db_handle)
{
    if (to_db_handle(Fred::Object::Type::CONTACT) == db_handle) { return Fred::Object::Type::CONTACT; }
    if (to_db_handle(Fred::Object::Type::NSSET)   == db_handle) { return Fred::Object::Type::NSSET; }
    if (to_db_handle(Fred::Object::Type::DOMAIN)  == db_handle) { return Fred::Object::Type::DOMAIN; }
    if (to_db_handle(Fred::Object::Type::KEYSET)  == db_handle) { return Fred::Object::Type::KEYSET; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Object::Type::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922
