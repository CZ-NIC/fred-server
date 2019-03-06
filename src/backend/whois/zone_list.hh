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
#ifndef ZONE_LIST_HH_60F796AB501E4A08A32279D6D14F17B7
#define ZONE_LIST_HH_60F796AB501E4A08A32279D6D14F17B7

#include "libfred/opcontext.hh"

#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Whois {

//list of managed zone names
std::vector<std::string> get_managed_zone_list(
        LibFred::OperationContext& ctx);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
