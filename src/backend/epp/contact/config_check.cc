/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/backend/epp/contact/config_check.hh"

namespace Epp {
namespace Contact {

ConfigCheck& ConfigCheck::set_name(const std::string& name)
{
    name_ = name;
    return *this;
}

template <>
bool ConfigCheck::is_type_of<ConfigCheck::Empty>()const
{
    return name_.empty();
}

}//namespace Epp::Contact
}//namespace Epp
