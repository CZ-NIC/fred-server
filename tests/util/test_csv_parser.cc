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

#include <sstream>

#include "util/util.h"
#include "util/printable.h"
#include "util/csv_parser.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestCsvParser)

const std::string server_name = "test-csv-parser";


static std::string format_csv_data(std::vector<std::vector<std::string> > data)
{
    std::stringstream ret;
    for(unsigned long long i = 0; i < data.size(); ++i)
    {
        ret << "#" << i << " |" << Util::format_container(data.at(i),"|") << "|";
    }
    return ret.str();
}

/**
 * test csv_parser
 */
BOOST_AUTO_TEST_CASE(test_trivial)
{
    BOOST_CHECK(Util::CsvParser("").parse().size() == 0);
    BOOST_CHECK(Util::CsvParser("1").parse().size() == 1);

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        );
    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );
}

BOOST_AUTO_TEST_CASE(test_unquoting_data)
{
    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;b3;b4\n"
        "c1;c2;\"c3\";c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;\"b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "\"b1\";\"b2\";\"b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "\"b1\";b2;\"b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "\"a1\";a2;a3;a4\n"
        "b1;\"b2\";b3;b4\n"
        "c1;c2;\"c3\";c4\n"
        "d1;d2;d3;\"d4\""
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;\"b3;b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3;b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "\"b1;\";b2;\"b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1;|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;\"b3\";\"b4;\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4;|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;\"b3\";\"b4\n\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4\n|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "\"\nb1\";b2;\"b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |\nb1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;\"b3\";\"\nb4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|\nb4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "\"b1\n\";b2;\"b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1\n|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );
}

BOOST_AUTO_TEST_CASE(test_unquoting_quote)
{
    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;b3;\"b4\"\"\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4\"|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1;b2;b3;\"\"\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|\"b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        " \"\"b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 | \"b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        " \"\"\"b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 | \"\"b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        " \"b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 | \"b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1\"b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1\"b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1\"\"b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1\"b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1\"\"\"b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1\"\"b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "b1\"\"\"\"b1;b2;b3;b4\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1\"\"b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "\"\"\"a1\";a2;a3;a4\n"
        "b1;\"\"\"\"\"b2\";b3;b4\n"
        "c1;c2;\"c3\"\"\"\"\";c4\n"
        "d1;d2;d3;\"d4\"\"\""
        ).parse()) ==
        "#0 |\"a1|a2|a3|a4|"
        "#1 |b1|\"\"b2|b3|b4|"
        "#2 |c1|c2|c3\"\"|c4|"
        "#3 |d1|d2|d3|d4\"|"
        );

    try
    {
        Util::CsvParser(
            "a1;\"a2 \"\";a3;a4\n"
            "b1;b2;b3;b4\n"
            "c1;c2;c3;c4\n"
            "d1;d2;d3;d4"
            ).parse();

        BOOST_ERROR("quoting check");
    }
    catch(const std::exception& exception)
    {
        BOOST_CHECK(&exception);
    }

}

BOOST_AUTO_TEST_CASE(test_newline)
{
    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\n"
        "\"b1\";b2;\"b3\";\"b4\"\n"
        "c1;c2;c3;c4\n"
        "d1;d2;d3;d4\n"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "\"a1\";a2;a3;a4\n"
        "b1;\"b2\";b3;b4\n"
        "c1;c2;\"c3\";c4\n"
        "d1;d2;d3;\"d4\"\n"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\r\n"
        "\"b1\";b2;\"b3\";\"b4\"\r\n"
        "c1;c2;c3;c4\r\n"
        "d1;d2;d3;d4\r\n"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "\"a1\";a2;a3;a4\r\n"
        "b1;\"b2\";b3;b4\r\n"
        "c1;c2;\"c3\";c4\r\n"
        "d1;d2;d3;\"d4\"\r\n"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\r"
        "\"b1\";b2;\"b3\";\"b4\"\r"
        "c1;c2;c3;c4\r"
        "d1;d2;d3;d4\r"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "\"a1\";a2;a3;a4\r"
        "b1;\"b2\";b3;b4\r"
        "c1;c2;\"c3\";c4\r"
        "d1;d2;d3;\"d4\"\r"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\r\n"
        "\"b1\";b2;\"b3\";\"b4\"\r\n"
        "c1;c2;c3;c4\r\n"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "\"a1\";a2;a3;a4\r\n"
        "b1;\"b2\";b3;b4\r\n"
        "c1;c2;\"c3\";c4\r\n"
        "d1;d2;d3;\"d4\""
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "a1;a2;a3;a4\r"
        "\"b1\";b2;\"b3\";\"b4\"\r"
        "c1;c2;c3;c4\r"
        "d1;d2;d3;d4"
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

    BOOST_CHECK(format_csv_data(Util::CsvParser(
        "\"a1\";a2;a3;a4\r"
        "b1;\"b2\";b3;b4\r"
        "c1;c2;\"c3\";c4\r"
        "d1;d2;d3;\"d4\""
        ).parse()) ==
        "#0 |a1|a2|a3|a4|"
        "#1 |b1|b2|b3|b4|"
        "#2 |c1|c2|c3|c4|"
        "#3 |d1|d2|d3|d4|"
        );

}



BOOST_AUTO_TEST_SUITE_END();
