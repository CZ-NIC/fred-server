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

#include "src/util/csv/rapidcsv.hh"

#include <vector>
#include <string>

namespace Fred
{
namespace Util
{

std::string to_csv_string(const std::vector<std::vector<std::string>>& _cells)
{
    rapidcsv::Document doc(std::string(), rapidcsv::LabelParams(-1, -1), rapidcsv::SeparatorParams(';', true));
    for (std::size_t i = 0; i < _cells.size(); ++i)
    {
        const std::size_t column_size = _cells[i].size();
        for (std::size_t j = 0; j < column_size; ++j)
        {
            doc.SetCell<std::string>(j, i, _cells[i][j]);
        }
    }
    return doc.ToString();
}

} // namespace Fred::Util
} // namespace Fred

#endif
