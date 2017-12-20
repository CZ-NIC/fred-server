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

#ifndef OBJECT_STATES_LOCALIZED_H_33777B6EEDFD423494782C90D5718B94
#define OBJECT_STATES_LOCALIZED_H_33777B6EEDFD423494782C90D5718B94

#include <map>
#include <string>

namespace Epp {

template <typename T>
struct ObjectStatesLocalized
{

    typedef std::map<typename T::Enum, std::string> Descriptions;

    Descriptions descriptions;
    std::string success_state_localized_description;

};


} // namespace Epp

#endif
