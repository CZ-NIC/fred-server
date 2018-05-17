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

#include <boost/test/unit_test.hpp>

#include <vector>
#include <string>
#include <array>

BOOST_AUTO_TEST_SUITE(TestCsvSerializer)

namespace
{
constexpr char separator = ';';
constexpr char alternative_separator = ',';
}

BOOST_AUTO_TEST_CASE(test_empty_file)
{
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<separator>(std::vector<std::vector<std::string>>()),
            std::string());
}

BOOST_AUTO_TEST_CASE(test_single_empty_cell)
{
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<separator>(std::vector<std::vector<std::string>>(1)),
            std::string("\r\n"));
}

BOOST_AUTO_TEST_CASE(test_single_unquoted_cell)
{
    const std::vector<std::vector<std::string>> input = {{ " 1, 2 " }};
    const std::string expected_output(" 1, 2 \r\n");
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<separator>(input),
            std::string(expected_output));
}

BOOST_AUTO_TEST_CASE(test_single_unquoted_cell_comma)
{
    const std::vector<std::vector<std::string>> input = {{ " 1; 2 " }};
    const std::string expected_output(" 1; 2 \r\n");
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<alternative_separator>(input),
            std::string(expected_output));
}

BOOST_AUTO_TEST_CASE(test_single_quoted_cell)
{
    const std::vector<std::vector<std::string>> input = {{ "\n;\"" }};
    const std::string expected_output("\"\n;\"\"\"\r\n");
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<separator>(input),
            expected_output);
}

BOOST_AUTO_TEST_CASE(test_single_quoted_cell_comma)
{
    const std::vector<std::vector<std::string>> input = {{ "\n,\"" }};
    const std::string expected_output("\"\n,\"\"\"\r\n");
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<alternative_separator>(input),
            expected_output);
}

BOOST_AUTO_TEST_CASE(test_multiple_cells)
{
    const std::vector<std::vector<std::string>> input =
        {
            { "\n;\"" },
            { "1; 2", "3, 4", " 5,; 6 "},
            {},
            {},
            { ",", "\"\""},
            { "", "", "niccc"},
            { " horní lhota ", "Dolní Lhota!", "něco", "nic ", " Žižkov"},
            { "", "", ""},
        };
    const std::string expected_output(
            "\"\n;\"\"\";;;;\r\n"
            "\"1; 2\";3, 4;\" 5,; 6 \";;\r\n"
            ";;;;\r\n"
            ";;;;\r\n"
            ",;\"\"\"\"\"\";;;\r\n"
            ";;niccc;;\r\n"
            " horní lhota ;Dolní Lhota!;něco;nic ; Žižkov\r\n"
            ";;;;\r\n"
        );
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<separator>(input),
            expected_output);
}

BOOST_AUTO_TEST_CASE(test_multiple_cells_comma)
{
    const std::vector<std::vector<std::string>> input =
        {
            { "\n;\"" },
            { "1; 2", "3, 4", " 5,; 6 "},
            {},
            {},
            { "", ""},
            { "", "", "niccc"},
            { " horní lhota ", "Dolní Lhota!", "něco", "nic ", " Žižkov"},
            { "", "\"\"", ","},
        };
    const std::string expected_output(
            "\"\n;\"\"\",,,,\r\n"
            "1; 2,\"3, 4\",\" 5,; 6 \",,\r\n"
            ",,,,\r\n"
            ",,,,\r\n"
            ",,,,\r\n"
            ",,niccc,,\r\n"
            " horní lhota ,Dolní Lhota!,něco,nic , Žižkov\r\n"
            ",\"\"\"\"\"\",\",\",,\r\n"
        );
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<alternative_separator>(input),
            expected_output);
}

BOOST_AUTO_TEST_CASE(test_matrix)
{
    const std::vector<std::array<std::string, 4>> input =
        {
            {"What", "if", "I", "told"},
            {"you", "that", "you", "can"},
            {"emit", "csv", "so", "that"},
            {"it", "can", "contain", "semicolons?"}
        };
    const std::string expected_output(
            "What;if;I;told\r\n"
            "you;that;you;can\r\n"
            "emit;csv;so;that\r\n"
            "it;can;contain;semicolons?\r\n"
        );
    BOOST_CHECK_EQUAL(
            Fred::Util::to_csv_string_using_separator<separator>(input),
            expected_output);
}

BOOST_AUTO_TEST_SUITE_END();
