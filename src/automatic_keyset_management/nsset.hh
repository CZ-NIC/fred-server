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

#ifndef NSSET_HH_CA510F4C3EC9401A9FCCBE77BA544F10
#define NSSET_HH_CA510F4C3EC9401A9FCCBE77BA544F10

#include <set>
#include <string>

namespace Fred {
namespace AutomaticKeysetManagement {

typedef std::set<std::string> Nameservers;

struct Nsset
{
    Nameservers nameservers;
};

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
