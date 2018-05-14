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

class CsvCells
{
public:
    CsvCells(const std::initializer_list<std::initializer_list<std::string>>& _cells);
    template<char Separator> std::string to_string() const;
private:
    std::vector<std::vector<std::string>> data;
};

} // namespace Fred::Util
} // namespace Fred

#endif
