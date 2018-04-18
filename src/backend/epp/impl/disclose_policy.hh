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

#ifndef DISCLOSE_POLICY_HH_FCD7D3675EA64EA09DB7E316BBD07BA5
#define DISCLOSE_POLICY_HH_FCD7D3675EA64EA09DB7E316BBD07BA5

namespace Epp {

constexpr bool is_the_default_policy_to_disclose() { return false; }
constexpr bool is_the_default_policy_to_hide() { return !is_the_default_policy_to_disclose(); }

} // namespace Epp

#endif
