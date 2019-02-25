/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#ifndef NAMESERVER_EXISTS_HH_124FA62B297049ABBF8E0591538D8291
#define NAMESERVER_EXISTS_HH_124FA62B297049ABBF8E0591538D8291

#include <string>

#include "libfred/opcontext.hh"

namespace Fred {
namespace Backend {
namespace Whois {

bool nameserver_exists(
        const std::string& ns_fqdn,
        LibFred::OperationContext& ctx);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
#endif
