/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include "util/db/nullable.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/db_settings.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/util/concurrent_queue.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/config_handler_decl.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_of.hpp>

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <utility>

BOOST_AUTO_TEST_SUITE(DbParamQuery)

BOOST_AUTO_TEST_CASE(init_params_test)
{
    const Database::QueryParam qp1 = Database::QPNull;
    BOOST_TEST_MESSAGE("QPNull: " << qp1.print_buffer());

    const Database::QueryParam qp2 = Database::QueryParam(true,Database::QueryParamData("abc"));
    BOOST_TEST_MESSAGE("QueryParam binary: " << qp2.print_buffer());

    const Database::QueryParam qp3 = "abc";
    BOOST_TEST_MESSAGE("QueryParam text: " << qp3.print_buffer());
}

BOOST_AUTO_TEST_CASE(exec_params_test)
{
    unsigned ret = 0;
    try
    {   //test param data in strings
        Database::Connection conn = Database::Manager::acquire();

        //because of changes to Nullable::operator<<
        BOOST_CHECK(conn.exec_params("SELECT $1::text", Database::query_param_list(Database::QPNull))[0][0].isnull());
        BOOST_CHECK(conn.exec_params("SELECT $1::text", Database::query_param_list(Nullable<std::string>()))[0][0].isnull());

        const std::string query = "SELECT 1 AS id,$1::bigint AS data1,$2::bigint AS data2,$3::text AS data3";

        const std::vector<std::string> params = boost::assign::list_of("2")("3")("Kuk");

        const Database::Result res = conn.exec_params(query, params);
        if ((0 < res.size()) && (res[0].size() == 4))
        {
            //check data
            if (static_cast<unsigned long long>(res[0][0]) != 1)
            {
                ret += 1;
            }
            if (static_cast<unsigned long long>(res[0][1]) != 2)
            {
                ret += 2;
            }
            if (static_cast<unsigned long long>(res[0][2]) != 3)
            {
                ret += 4;
            }
            BOOST_TEST_MESSAGE("test string: " << static_cast<std::string>(res[0][3]));
        }
        else
        {
            ret += 8;
        }
        if (ret != 0)
        {
            BOOST_TEST_MESSAGE("exec_params_test ret: "<< ret);
        }

        const std::string qquery =
                "SELECT $1::int AS id,$2::bigint AS data1,$3::int AS data2,$4::text AS data3,"
                       "$4::text='Kuk' AS data4,$5::int AS data5";

        const Database::QueryParams qparams = Database::query_param_list
            (1)//$1
            (1ll)//$2
            (-1l)//$3
            ("Kuk")//$4
            (Database::QPNull);//$5

        qparams[2].print_buffer();
        qparams[3].print_buffer();

        //test simple binary params
        const Database::Result qres = conn.exec_params(qquery, qparams);

        if ((0 < qres.size()) && (qres[0].size() == 6))
        {
            //check data
            if (static_cast<long>(qres[0][0]) != 1)
            {
                ret += 16;
            }
            if (static_cast<long long>(qres[0][1]) != 1)
            {
                ret += 32;
            }
            if (static_cast<long>(qres[0][2]) != -1)
            {
                ret += 64;
            }

            //std::cout << "test string: " << std::string(qres[0][3])
            //<< " data4: " << std::string(qres[0][4]) <<std::endl;

            if (static_cast<std::string>(qres[0][3]) != "Kuk")
            {
                ret += 1024;
            }
            if (!static_cast<bool>(qres[0][4]))
            {
                ret += 2048;
            }

            BOOST_TEST_MESSAGE("test null: " << static_cast<std::string>(qres[0][5]));

        }
        else
        {
            ret += 128;
        }
        if (ret != 0)
        {
            BOOST_TEST_MESSAGE("exec_params_test ret: "<< ret);
        }
    }
    catch (const std::exception& e)
    {
        BOOST_TEST_MESSAGE("exec_params_test exception reason: "<< e.what());
        ret += 256;
        throw;
    }
    catch (...)
    {
        BOOST_TEST_MESSAGE("exec_params_test exception returning");
        ret += 512;
        if (ret != 0)
        {
            BOOST_TEST_MESSAGE("exec_params_test ret: "<< ret);
        }
    }

    BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(test_timeout_exception)
{
    BOOST_TEST_MESSAGE("Check whether long queries yield an exception with expected message.");

    Database::Connection conn = Database::Manager::acquire();

    // timeout in miliseconds
    conn.setQueryTimeout(900);

    bool thrown = false;
    // now try to sleep for 1 second, it should yield the exception
    try
    {
        conn.exec("SELECT pg_sleep(1)");
    }
    catch (const Database::Exception &e)
    {
        thrown = true;
        const std::string message = e.what();
        // check if the exception contains the text rely on
        if (message.find(Database::Connection::getTimeoutString()) == std::string::npos)
        {
            BOOST_FAIL("Exception doesn't contain expected message.");
        }
    }

    BOOST_REQUIRE_MESSAGE(thrown, "No timeout (or any other) exception thrown, wrong behaviour. ");
}

BOOST_AUTO_TEST_CASE(nullable_to_query_param)
{
    {
        const Database::QueryParam qp = Database::QPNull;
        BOOST_CHECK(qp.is_null());
    }
    {
        const Database::QueryParam qp = Nullable<std::string>();
        BOOST_CHECK(qp.is_null());
    }
    {
        const Database::QueryParam qp = Nullable<int>();
        BOOST_CHECK(qp.is_null());
    }

    {
        const Database::QueryParam qp = "test";
        BOOST_CHECK(!qp.is_null());
    }
    {
        const Database::QueryParam qp = Nullable<std::string>("test");
        BOOST_CHECK(!qp.is_null());
    }
    {
        const Database::QueryParam qp = Nullable<int>(0);
        BOOST_CHECK(!qp.is_null());
    }

    {
        Database::QueryParam qp;
        qp = Database::QPNull;
        BOOST_CHECK(qp.is_null());
    }
    {
        Database::QueryParam qp;
        qp = Nullable<std::string>();
        BOOST_CHECK(qp.is_null());
    }
    {
        Database::QueryParam qp;
        qp = Nullable<int>();
        BOOST_CHECK(qp.is_null());
    }

    {
        Database::QueryParam qp;
        qp = "test";
        BOOST_CHECK(!qp.is_null());
    }
    {
        Database::QueryParam qp;
        qp = Nullable<std::string>("test");
        BOOST_CHECK(!qp.is_null());
    }
    {
        Database::QueryParam qp;
        qp = Nullable<int>(0);
        BOOST_CHECK(!qp.is_null());
    }

    BOOST_CHECK(Database::QueryParam(Database::QPNull).is_null());
    BOOST_CHECK(Database::QueryParam(Nullable<std::string>()).is_null());
    BOOST_CHECK(Database::QueryParam(Nullable<int>()).is_null());

    BOOST_CHECK(!Database::QueryParam("test").is_null());
    BOOST_CHECK(!Database::QueryParam(Nullable<std::string>("test")).is_null());
    BOOST_CHECK(!Database::QueryParam(Nullable<int>(0)).is_null());
}

BOOST_AUTO_TEST_SUITE_END()//DbParamQuery
