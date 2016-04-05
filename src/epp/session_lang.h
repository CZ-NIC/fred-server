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

#ifndef SRC_EPP_SESSION_LANG_505343440
#define SRC_EPP_SESSION_LANG_505343440

#include <string>

#include "src/epp/exception.h"

namespace Epp {

struct SessionLang {
    enum Enum { en, cz };
};

/**
 * @throws InvalidSessionLang
 */
inline std::string to_db_handle(const SessionLang::Enum lang) {
    switch(lang) {
        case SessionLang::en : return "en";
        case SessionLang::cz : return "cz";
    }

    throw InvalidSessionLang();
}

}

#endif
