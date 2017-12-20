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
*  header of LibFred::Object_Type class
*/

#ifndef OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922

#include "src/util/enum_conversion.hh"

/// Fred matters
namespace LibFred {

typedef unsigned long long Id;

/**
 * Bidirectional conversions between string and enum representation of object types.
 */
struct Object_Type
{
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

}//Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(LibFred::Object_Type::Enum value)
{
    switch (value)
    {
        case LibFred::Object_Type::contact: return "contact";
        case LibFred::Object_Type::nsset:   return "nsset";
        case LibFred::Object_Type::domain:  return "domain";
        case LibFred::Object_Type::keyset:  return "keyset";
    }
    throw std::invalid_argument("value doesn't exist in LibFred::Object_Type::Enum");
};

template < >
inline LibFred::Object_Type::Enum from_db_handle< LibFred::Object_Type >(const std::string &db_handle)
{
    if (to_db_handle(LibFred::Object_Type::contact) == db_handle) { return LibFred::Object_Type::contact; }
    if (to_db_handle(LibFred::Object_Type::nsset)   == db_handle) { return LibFred::Object_Type::nsset; }
    if (to_db_handle(LibFred::Object_Type::domain)  == db_handle) { return LibFred::Object_Type::domain; }
    if (to_db_handle(LibFred::Object_Type::keyset)  == db_handle) { return LibFred::Object_Type::keyset; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to LibFred::Object_Type::Enum");
}

} // namespace Conversion::Enums
} // namespace Conversion

#endif//OBJECT_TYPE_H_22A124B8D173FCF75E30657FA4D8D922
