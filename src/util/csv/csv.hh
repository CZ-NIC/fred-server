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

#ifndef CSV_HH_B446365F01434DEBABBE5930EEBA1DAA
#define CSV_HH_B446365F01434DEBABBE5930EEBA1DAA

#include <initializer_list>
#include <string>
#include <vector>

namespace Fred
{
namespace Util
{

template<char Separator>
std::string escape_csv_cell(const std::string& _cell)
{
    std::ostringstream output;
    if (std::find_if(_cell.begin(), _cell.end(),
                     [](char c){return c == '"' || c == Separator || c == '\n';}) != _cell.end())
    {
        output << '"';
        if (_cell.find('"') != std::string::npos)
        {
            for (const char c: _cell)
            {
                if (c == '"')
                {
                    output << R"("")";
                }
                else
                {
                    output << c;
                }
            }
        }
        else
        {
            output << _cell;
        }
        output << '"';
    }
    else
    {
        output << _cell;
    }
    return output.str();
}

template<char Separator, typename T>
std::string to_csv_string_using_separator(const T& list_of_rows)
{
    std::size_t no_of_columns = 0;
    for (const auto& cell: list_of_rows)
    {
        if(cell.size() > no_of_columns)
        {
            no_of_columns = cell.size();
        }
    }
    const std::size_t no_of_separators = no_of_columns > 0 ? no_of_columns - 1 : 0;
    std::ostringstream output;
    for (const auto& row: list_of_rows)
    {
        auto remaining_no_of_separators = no_of_separators;
        for (const auto& cell: row)
        {
            output << escape_csv_cell<Separator>(cell);
            if (remaining_no_of_separators > 0)
            {
                output << Separator;
                --remaining_no_of_separators;
            }
        }
        for (; remaining_no_of_separators > 0; --remaining_no_of_separators)
        {
            output << Separator;
        }
        output << "\r\n";
    }
    return output.str();
}

} // namespace Fred::Util
} // namespace Fred

#endif
