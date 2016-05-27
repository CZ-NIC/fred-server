/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef LOCALIZED_STATES_H_0E3D3ED62A7B8FCC566C8FC4C927EF6D//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define LOCALIZED_STATES_H_0E3D3ED62A7B8FCC566C8FC4C927EF6D

#include "src/fredlib/object/object_state.h"

#include <map>

namespace Epp {

typedef std::map< Fred::Object_State::Enum, std::string > LocalizedStates;

}

#endif//LOCALIZED_STATES_H_0E3D3ED62A7B8FCC566C8FC4C927EF6D
