/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  csv parser
 */


#ifndef CSV_PARSER_HH_D1BD336B7AE84AC58427CF5412656369
#define CSV_PARSER_HH_D1BD336B7AE84AC58427CF5412656369

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/algorithm/string.hpp>


namespace Util
{
    /**
     * CSV like data parser.
     * Implementation is more simple than high-performance.
     * CSV format is RFC4180 section-2  with exception of:
     * Quotation marks are considered doubled even in unquoted field e.g. test1""test2 -> test1"test2 and test1"test2 -> test1"test2.
     * No header is expected, data fields are expected in the fist line of CSV data.
     * Expected newline is LF, CR or CRLF.
     * Accept and ignores empty lines except one optional newline at the end of data, that do not create row in output.
     * Don't care about number of fields in row.
     */
    class CsvParser
    {
        const std::string csv_data_;
        const char delimiter_;
        const char quotation_mark_;
    public:

        /**
         * Ctor
         * @param csv_data is text to be processed
         * @param delimiter is CSV field delimiter, default semicolon
         * @param quotation_mark is is CSV field quotation mark, default double quotes
         */
        CsvParser(const std::string& csv_data, const char delimiter = ';', const char quotation_mark = '"')
        : csv_data_(csv_data)
        , delimiter_(delimiter)
        , quotation_mark_(quotation_mark)
        {}

        /**
         * Process data and return structured data
         * @return rows of string fields with non uniform row lengths
         * @throws std::runtime_error with short description of failure.
         */
        std::vector<std::vector<std::string> > parse();
    };
}

#endif
