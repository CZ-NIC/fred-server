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

#include "src/util/cfg/validate_args.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

std::ostream& operator<<(std::ostream& o, const Checked::Date& s)
{
    return o << boost::gregorian::to_iso_extended_string(s.date);
}

std::istream& operator>>(std::istream& i, Checked::Date& s)
{
    std::string date;
    i >> date;
    s.date = boost::gregorian::from_string(date);
    return i;
}
