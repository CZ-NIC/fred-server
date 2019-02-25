/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#ifndef OUTPUT_INDENTER_HH_A5F53850F7CB46FD916780BB690A17E8
#define OUTPUT_INDENTER_HH_A5F53850F7CB46FD916780BB690A17E8

#include <ostream>


struct OutputIndenter
{
    unsigned short indent;
    unsigned short level;
    char c;

    OutputIndenter dive() const
    {
        return OutputIndenter(indent, level + 1, c);
    }

    OutputIndenter(unsigned short _i, unsigned short _l, char _c)
        : indent(_i), level(_l), c(_c)
    {
    }

    friend std::ostream& operator<<(std::ostream &_ostream, const OutputIndenter &_oi);
};



#endif /*OUTPUT_INDENTER_H__*/

