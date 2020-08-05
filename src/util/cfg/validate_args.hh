/*
 * Copyright (C) 2011-2020  CZ.NIC, z. s. p. o.
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

#ifndef VALIDATE_ARGS_HH_BAA5803FB80F4518ABC0DC0372B55DCF
#define VALIDATE_ARGS_HH_BAA5803FB80F4518ABC0DC0372B55DCF

#include "src/util/cfg/checked_types.hh"

#include <iostream>

std::ostream& operator<<(std::ostream& o, const Checked::string_fpnumber& s);

std::istream& operator>>(std::istream& i, Checked::string_fpnumber& s);

std::ostream& operator<<(std::ostream&, const Checked::Date&);

std::istream& operator>>(std::istream&, Checked::Date&);

#endif
