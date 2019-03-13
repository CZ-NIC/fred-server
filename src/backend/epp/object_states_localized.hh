/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef OBJECT_STATES_LOCALIZED_HH_693FE7BDCAA4477BA1EAD7B5B33FD69F
#define OBJECT_STATES_LOCALIZED_HH_693FE7BDCAA4477BA1EAD7B5B33FD69F

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
