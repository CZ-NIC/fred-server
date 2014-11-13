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
#include "util/random_data_generator.h"

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

void inc_test_csv( std::string& csv_file, std::string alphabet ="a;\"\r\n")
{
    if(alphabet.empty()) throw std::runtime_error("empty alphabet");

    for(unsigned long long i = 0; i < (csv_file.length() + 1); ++i)
    {
        if(i == csv_file.length())
        {
            csv_file += alphabet.at(0);
            return;
        }

        if(csv_file.at(i) == alphabet.at(alphabet.length() - 1))
        {
            csv_file.at(i) = alphabet.at(0);
            continue;
        }
        else
        {
            csv_file.at(i) = alphabet.at(alphabet.find(csv_file.at(i)) + 1);
            return;
        }
    }
}

BOOST_AUTO_TEST_CASE(test_err_crlf)
{
    std::string csv_file;

    while(csv_file.length() < 10)
    {
        inc_test_csv(csv_file, "a;\"\r\n");

        try
        {
            Util::CsvParser(csv_file).parse();
        }
        catch(std::runtime_error&)
        {
        }
        catch(...)
        {
            BOOST_FAIL(csv_file);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_err_lf)
{
    std::string csv_file;

    while(csv_file.length() < 12)
    {
        inc_test_csv(csv_file, "a;\"\n");

        try
        {
            Util::CsvParser(csv_file).parse();
        }
        catch(std::runtime_error&)
        {
        }
        catch(...)
        {
            BOOST_FAIL(csv_file);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_err_nodata_lf)
{
    std::string csv_file;

    while(csv_file.length() < 13)
    {
        inc_test_csv(csv_file, ";\"\n");

        try
        {
            Util::CsvParser(csv_file).parse();
        }
        catch(std::runtime_error&)
        {
        }
        catch(...)
        {
            BOOST_FAIL(csv_file);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_err_big_random_data)
{
    RandomDataGenerator rnd;
    std::string csv_file(1000000,'a');
    std::string alphabet ="a;\"\r\n";

    for(unsigned long long j = 0; j < 1000; ++j)
    {
        for(unsigned long long i = 0 ; i < 1000000; ++i) csv_file.at(i) = alphabet.at(rnd.xnum1_5() - 1);

        try
        {
            Util::CsvParser(csv_file).parse();
            //BOOST_MESSAGE(csv_file);
        }
        catch(std::runtime_error&)
        {
            //BOOST_WARN_MESSAGE( false, csv_file);
        }
        catch(...)
        {
            BOOST_FAIL(csv_file);
        }
    }
}


BOOST_AUTO_TEST_SUITE_END();
