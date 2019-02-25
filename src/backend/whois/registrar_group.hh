/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
#ifndef REGISTRAR_GROUP_HH_4D341FA239FE4B10ACDD71E65142B2C1
#define REGISTRAR_GROUP_HH_4D341FA239FE4B10ACDD71E65142B2C1

#include "libfred/opcontext.hh"

#include <map>
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Whois {

// maps registrar_group to list of registrar handles, current registrar group members
std::map<std::string, std::vector<std::string> > get_registrar_groups(LibFred::OperationContext& ctx);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
