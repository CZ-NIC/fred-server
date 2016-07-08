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
 */

#ifndef EPP_IDENT_TYPE_H_42425413544121
#define EPP_IDENT_TYPE_H_42425413544121

#include <string>
#include <stdexcept>

#include "src/epp/exception.h"

namespace Epp {

struct IdentType {
    enum Enum {
        birth_certificate,
        identity_card,
        passport,
        organization_identification,
        social_security_number,
        birthday
    };
};

inline std::string to_db_handle(const IdentType::Enum ident_type) {
    switch(ident_type) {
        case IdentType::identity_card:
            return "OP";
        case IdentType::passport:
            return "PASS";
        case IdentType::organization_identification:
            return "ICO";
        case IdentType::social_security_number:
            return "MPSV";
        case IdentType::birthday:
            return "BIRTHDAY";
        case IdentType::birth_certificate:  /* not used anymore */
            return "";
    };

    return "";
}

template<typename T> inline typename T::Enum from_db_handle(const std::string& _handle);

/**
 * @throws InvalidIdentTypeDbHandle
 */
template<> inline IdentType::Enum from_db_handle<IdentType>(const std::string& _handle) {

    if(_handle == "OP") {
        return IdentType::identity_card;
    }

    if(_handle == "PASS") {
        return IdentType::passport;
    }

    if(_handle == "ICO") {
        return IdentType::organization_identification;
    }

    if(_handle == "MPSV") {
        return IdentType::social_security_number;
    }

    if(_handle == "BIRTHDAY") {
        return IdentType::birthday;
    }

    throw InvalidIdentTypeDbHandle();
}

}

#endif
