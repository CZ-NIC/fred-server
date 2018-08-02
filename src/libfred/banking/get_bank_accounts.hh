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

#ifndef GET_BANK_ACCOUNTS_HH_8B3A01866499420DAFEE35DFE7FDF917
#define GET_BANK_ACCOUNTS_HH_8B3A01866499420DAFEE35DFE7FDF917

#include <string>
#include <vector>

namespace LibFred {
namespace Banking {

struct EnumListItem
{
  unsigned long long id;
  std::string name;
};
typedef std::vector<EnumListItem> EnumList;

EnumList getBankAccounts();

#endif

} // namespace LibFred::Banking
} // namespace LibFred
