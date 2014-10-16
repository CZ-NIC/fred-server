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



#include "util/util.h"
#include "util/printable.h"
#include "util/csv_parser.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestCsvParser)

const std::string server_name = "test-csv-parser";


/**
 * test csv_parser
 */
BOOST_AUTO_TEST_CASE(test)
{
    std::string test_data_optys =
        "21313131;Na uvedené adrese neznámý\n"
        "56466466;\"Nevyzvednuto\";\"\"\n"
        "75679799;\"Odstěhoval se\"\n"
        "85313645;Zemřel;\n"
        "19965465;\"Adresa\n nedostatečná\"\n"
        "55666655;Nepřijato \n"
        "45698705;Jiný důvod\n"
        " \"\";2\n"
        " \"\"\"\";4\n"
        " \"\"\"\"\"\"\"\";8\n"
        "\n"
        "\n"
        "\" \";\"\"\n"// 2 quoted fields in row
        "\" \";\" \"\n"// 2 quoted space fields in row
        "end\n"
        ;

    std::string test_data_bug = "\"\"\n\"\"";

    std::vector<std::vector<std::string> > data = Util::CsvParser(test_data_optys).parse();

    for(unsigned long long i = 0; i < data.size(); ++i)
    {
        BOOST_MESSAGE( i << ": |" << Util::format_container(data.at(i),"|") << "|");
    }

    BOOST_CHECK(Util::CsvParser("").parse().size() == 0);
    BOOST_CHECK(Util::CsvParser("1").parse().size() == 1);
}

BOOST_AUTO_TEST_SUITE_END();
