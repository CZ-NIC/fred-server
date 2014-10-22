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
#include "util/optys/download_client.h"
#include "util/random_data_generator.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestOptys)

const std::string server_name = "test-optys";


/**
 * test csv_parser
 */
BOOST_AUTO_TEST_CASE(test_csv_file_name)
{
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
        "test0.csv\n./\ntest1.csv\ntest2.csv"),"|") ==
        "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
        "test0.csv\n./\ntest1.csv\ntest2.csv\n"),"|") ==
        "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\ntest0.csv\n./\ntest1.csv\ntest2.csv"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "test0.csv\n./\ntest1.csv\n\ntest2.csv"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./\ntest0.csv\ntest1.csv\ntest2.csv"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./\ntest0.csv\ntest1.csv\ntest2.csv\n"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./\n"),"|") ==
            "");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./"),"|") ==
            "");
}

BOOST_AUTO_TEST_SUITE_END();
