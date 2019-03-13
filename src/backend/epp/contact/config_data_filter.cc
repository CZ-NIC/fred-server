/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/contact/config_data_filter.hh"

namespace Epp {
namespace Contact {

ConfigDataFilter& ConfigDataFilter::set_name(const std::string& name)
{
    name_ = name;
    return *this;
}

template <>
bool ConfigDataFilter::is_type_of<ConfigDataFilter::Empty>()const
{
    return name_.empty();
}

}//namespace Epp::Contact
}//namespace Epp
