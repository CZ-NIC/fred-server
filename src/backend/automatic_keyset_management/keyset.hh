/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef KEYSET_HH_A8A67E66692040148C32B954A28423F7
#define KEYSET_HH_A8A67E66692040148C32B954A28423F7

#include "src/backend/automatic_keyset_management/dns_key.hh"

namespace LibFred {
namespace AutomaticKeysetManagement {

typedef std::set<DnsKey> DnsKeys;

struct Keyset
{
    DnsKeys dns_keys;
};

} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
