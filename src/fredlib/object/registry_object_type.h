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
 *  basic object types in registry
 */

#ifndef FREDLIB_OBJECT_REGISTRY_OBJECT_TYPE_0154500110000
#define FREDLIB_OBJECT_REGISTRY_OBJECT_TYPE_0154500110000

#include <string>

namespace Fred {

/* TODO ready for discussion about more appropriate "enum" implementation */

enum object_type {
    contact,
    domain,
    keyset,
    nsset
};

struct ExceptionUnknownObjectType {
    const char* what() const { return "unknown object type"; }
};

/** @throws ExceptionUnknownObjectType */
inline std::string to_db_handle(object_type _input) {
    switch(_input) {
        case contact:   return "contact";
        case domain:    return "domain";
        case keyset:    return "keyset";
        case nsset:     return "nsset";
    };

    throw ExceptionUnknownObjectType();
}

/** @throws ExceptionUnknownObjectType */
inline object_type object_type_from_db_handle(const std::string& _handle) {
    if(_handle == "contact" ) { return contact; }
    if(_handle == "domain"  ) { return domain;  }
    if(_handle == "keyset"  ) { return keyset;  }
    if(_handle == "nsset"   ) { return nsset;   }

    throw ExceptionUnknownObjectType();
}

}

#endif
