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

#ifndef PURPOSE_HH_DF4E208D9E66437BA875CF29CB5D07B3
#define PURPOSE_HH_DF4E208D9E66437BA875CF29CB5D07B3

namespace Registry {
namespace RecordStatement {

struct Purpose
{
    enum Enum
    {
        private_printout,
        public_printout
    };
};

} // namespace Registry::RecordStatement
} // namespace Registry

#endif
