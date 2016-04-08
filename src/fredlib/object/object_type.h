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
        contact,///< object is contact
        nsset,  ///< object is nsset
        domain, ///< object is domain
        keyset, ///< object is keyset
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
        case Fred::Object::Type::contact: return "contact";
        case Fred::Object::Type::nsset:   return "nsset";
        case Fred::Object::Type::domain:  return "domain";
        case Fred::Object::Type::keyset:  return "keyset";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Object::Type::Enum");
};

template < >
inline Fred::Object::Type::Enum from_db_handle< Fred::Object::Type >(const std::string &db_handle)
{
    if (to_db_handle(Fred::Object::Type::contact) == db_handle) { return Fred::Object::Type::contact; }
    if (to_db_handle(Fred::Object::Type::nsset)   == db_handle) { return Fred::Object::Type::nsset; }
    if (to_db_handle(Fred::Object::Type::domain)  == db_handle) { return Fred::Object::Type::domain; }
    if (to_db_handle(Fred::Object::Type::keyset)  == db_handle) { return Fred::Object::Type::keyset; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Object::Type::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922
