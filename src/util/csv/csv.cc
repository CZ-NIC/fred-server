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

#include "src/util/csv/csv.hh"

#include <sstream>
#include <algorithm>

namespace Fred
{
namespace Util
{

namespace
{

std::string escape_csv_cell(const std::string& _cell)
{
    std::ostringstream output;
    output << '"';
    if (_cell.find('"') != std::string::npos)
    {
        for (const char c: _cell)
        {
            if (c == '"')
            {
                output << c;
            }
            output << c;
        }
    }
    else
    {
        output << _cell;
    }
    output << '"';
    return output.str();
}

} // Fred::Util::{anonymous}

CsvCells::CsvCells(const std::initializer_list<std::initializer_list<std::string>>& _cells)
{
    data.reserve(_cells.size());
    for (const auto& row: _cells)
    {
        data.emplace_back(row.begin(), row.end());
    }
}

template<char Separator>
std::string CsvCells::to_string() const
{
    std::size_t no_of_columns = 0;
    for (const auto& row: data)
    {
        if(row.size() > no_of_columns)
        {
            no_of_columns = row.size();
        }
    }
    const std::size_t no_of_separators = no_of_columns > 0 ? no_of_columns - 1 : 0;
    std::ostringstream output;
    for (const auto& row: data)
    {
        auto remaining_no_of_separators = no_of_separators;
        for (const auto& cell: row)
        {
            if (std::find_if(cell.begin(), cell.end(),
                             [](char c){return c == '"' || c == Separator || c == '\n';}) != cell.end())
            {
                output << escape_csv_cell(cell);
            }
            else
            {
                output << cell;
            }
            if (remaining_no_of_separators > 0)
            {
                output << Separator;
                --remaining_no_of_separators;
            }
        }
        for (std::size_t i = remaining_no_of_separators; i > 0; --i)
        {
            output << Separator;
        }
        output << "\r\n";
    }
    return output.str();
}

template std::string CsvCells::to_string<';'>() const;

} // namespace Fred::Util
} // namespace Fred
