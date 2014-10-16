/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  csv parser
 */


#ifndef CSV_PARSER_H_0db51262c43845f18f8f82a0f8f8fb96
#define CSV_PARSER_H_0db51262c43845f18f8f82a0f8f8fb96

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/algorithm/string.hpp>


namespace Util
{
    /**
     * csv data parser
     */
    class CsvParser
    {
        const std::string csv_file_;
        const char delimiter_;
        const char quotation_mark_;
    public:
        CsvParser(const std::string& csv_file, const char delimiter = ';', const char quotation_mark = '"')
        : csv_file_(csv_file)
        , delimiter_(delimiter)
        , quotation_mark_(quotation_mark)
        {}

        std::vector<std::vector<std::string> > parse();
    };
}

#endif
