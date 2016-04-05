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

#ifndef EPP_OBJECT_STATE_H_1238979645140
#define EPP_OBJECT_STATE_H_1238979645140

#include "src/epp/session_lang.h"

#include <string>
#include <utility>

namespace Epp {

struct ObjectState {
    std::string name;
    std::pair<std::string, SessionLang::Enum> description;
    bool external;

    ObjectState(
        const std::string& _name,
        const std::pair<std::string, SessionLang::Enum>& _description,
        bool _external
    ) :
        name(_name),
        description(_description),
        external(_external)
    { }
};

}
#endif
