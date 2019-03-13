/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef PURPOSE_HH_DF4E208D9E66437BA875CF29CB5D07B3
#define PURPOSE_HH_DF4E208D9E66437BA875CF29CB5D07B3

namespace Fred {
namespace Backend {
namespace RecordStatement {

struct Purpose
{
    enum Enum
    {
        private_printout,
        public_printout
    };
};

} // namespace Fred::Backend::RecordStatement
} // namespace Fred::Backend
} // namespace Fred

#endif
