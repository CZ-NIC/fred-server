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

#ifndef STATES_LOCALIZED_H_96ACF1844D5F4404B4694EABA35A19B7
#define STATES_LOCALIZED_H_96ACF1844D5F4404B4694EABA35A19B7

#include "src/fredlib/object/object_state.h"

#include <map>

namespace Epp {

struct LocalizedStates
{
    typedef std::map< Fred::Object_State::Enum, std::string > Descriptions;
    Descriptions descriptions;
    std::string ok_state_description;
};


}

#endif
