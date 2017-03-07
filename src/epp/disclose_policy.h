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

/**
 *  @file
 *  header of disclose policy
 */

#ifndef DISCLOSE_POLICY_H_0BE93A0B1FD6F3AE1C894980D57567D1//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define DISCLOSE_POLICY_H_0BE93A0B1FD6F3AE1C894980D57567D1

namespace Epp {

inline bool is_the_default_policy_to_disclose() { return true; }
inline bool is_the_default_policy_to_hide() { return !is_the_default_policy_to_disclose(); }

}//namespace Epp

#endif//DISCLOSE_POLICY_H_0BE93A0B1FD6F3AE1C894980D57567D1