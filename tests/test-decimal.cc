/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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

#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "setup_server_decl.h"
#include "time_clock.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "util/decimal/decimal.h"

BOOST_AUTO_TEST_SUITE(TestDecimal)

const std::string server_name = "test-decimal";



bool check_std_exception_MPD(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("MPD")) != std::string::npos);
}


BOOST_AUTO_TEST_CASE( test_decimal_wrapper )
{

    BOOST_CHECK(Decimal("1") < Decimal("2"));
    BOOST_CHECK(Decimal("1") <= Decimal("2"));
    BOOST_CHECK(Decimal("1.1") < Decimal("1.2"));
    BOOST_CHECK(Decimal("1000") > Decimal("100"));
    BOOST_CHECK(Decimal("1000") >= Decimal("100"));
    BOOST_CHECK(Decimal("1000") >= Decimal("1000"));
    BOOST_CHECK(Decimal("1.1") == Decimal("1.1"));
    BOOST_CHECK(Decimal("1.1") != Decimal("1.2"));
    BOOST_CHECK(Decimal("1.11") > Decimal("-1.11"));
    BOOST_CHECK((Decimal("1.111") + Decimal("1.222")) == Decimal("2.333"));
    BOOST_CHECK((Decimal("2.333") - Decimal("1.111")) == Decimal("1.222"));
    BOOST_CHECK((Decimal("2.333") - Decimal("1.111")) != Decimal("1.223"));
    BOOST_CHECK((Decimal("1.111") * Decimal("1.222")) == Decimal("1.357642"));
    BOOST_CHECK((Decimal("1.222") / Decimal("1.111"))
            .round(19, MPD_ROUND_HALF_UP)
            .round(9, MPD_ROUND_HALF_UP) == Decimal("1.099909991"));

    BOOST_CHECK((Decimal("13").integral_division(Decimal("3"))) == Decimal("4"));
    BOOST_CHECK((Decimal("13").integral_division_remainder(Decimal("3"))) == Decimal("1"));

    BOOST_CHECK(Decimal("-1").abs() == Decimal("1"));
    BOOST_CHECK(Decimal("1").abs() == Decimal("1"));

    BOOST_CHECK(Decimal("-1").is_negative());

    BOOST_CHECK(Decimal("1 000").is_nan());
    BOOST_CHECK(Decimal("1 000").is_special());
    BOOST_CHECK(Decimal("1,1").is_nan());
    BOOST_CHECK(Decimal("1,1").is_special());

    BOOST_CHECK_EXCEPTION((Decimal("1") / Decimal("0")).is_infinite()
            , std::exception
            , check_std_exception_MPD);//Infinity

    BOOST_CHECK((Decimal("0") / Decimal("0")).is_nan());//NaN

    BOOST_CHECK_EXCEPTION((Decimal("1") / Decimal("-0")).is_infinite()
            , std::exception
            , check_std_exception_MPD
            );//-Infinity

    BOOST_CHECK(Decimal("Infinity").is_infinite());//Infinity
    BOOST_CHECK(Decimal("-Infinity").is_infinite());//Infinity
    BOOST_CHECK(Decimal("NaN").is_nan());//NaN
    BOOST_CHECK(Decimal("-NaN").is_nan());//-NaN

    BOOST_CHECK(Decimal().is_nan());//default ctor init check
    BOOST_CHECK(Decimal().get_string().compare("NaN") == 0);//default ctor init check

    BOOST_CHECK(Decimal(std::string()).is_nan());//string ctor
    BOOST_CHECK(Decimal(std::string()).get_string().compare("NaN") == 0);//string ctor

    BOOST_CHECK(Decimal(std::string("1.1")) == Decimal("1.1"));//string ctor


    BOOST_CHECK(Decimal("Infinity") == Decimal("Infinity"));
    BOOST_CHECK(Decimal("Infinity") != Decimal("-Infinity"));
    BOOST_CHECK(Decimal("NaN") != Decimal("NaN"));
    BOOST_CHECK(Decimal("NaN") != Decimal("-NaN"));

    BOOST_CHECK( (Decimal("Infinity") + Decimal("-Infinity")).is_nan());

    Decimal a("1234567890.123456789", 500);

    Decimal b = a;//copy ctor
    BOOST_CHECK(b == a);
    BOOST_CHECK(b.get_precision() == 500);

    Decimal c;
    c = a;//assignment operator
    BOOST_CHECK(c == a);
    BOOST_CHECK(c.get_precision() == 500);

    //stream operators
    Decimal dstream1("111.333");
    Decimal dstream2;
    std::stringstream sstr;
    sstr << dstream1;
    sstr >> dstream2;
    BOOST_CHECK(dstream1 == dstream2);

    //format and round money
    BOOST_CHECK(Decimal("10").get_string(".2f").compare("10.00") == 0);//string ctor
    BOOST_CHECK(Decimal("10.1").get_string(".2f").compare("10.10") == 0);//string ctor
    BOOST_CHECK(Decimal("-10").get_string(".2f").compare("-10.00") == 0);//string ctor
    BOOST_CHECK(Decimal("-10.1").get_string(".2f").compare("-10.10") == 0);//string ctor

    BOOST_CHECK(Decimal("10.005").get_string(".2f").compare("10.01") == 0);//string ctor
    BOOST_CHECK(Decimal("10000000.1").get_string(".2f").compare("10000000.10") == 0);//string ctor
    BOOST_CHECK(Decimal("-10.005").get_string(".2f").compare("-10.01") == 0);//string ctor
    BOOST_CHECK(Decimal("-10000000.1").get_string(".2f").compare("-10000000.10") == 0);//string ctor
    BOOST_CHECK(Decimal("30000000.1").get_string(".2f").compare("30000000.10") == 0);//string ctor
    BOOST_CHECK(Decimal("-30000000.1").get_string(".2f").compare("-30000000.10") == 0);//string ctor

    BOOST_CHECK(Decimal("30000000.001").get_string(".2f").compare("30000000.00") == 0);//string ctor
    BOOST_CHECK(Decimal("-30000000.001").get_string(".2f").compare("-30000000.00") == 0);//string ctor
    BOOST_CHECK(Decimal("30000000.007").get_string(".2f").compare("30000000.01") == 0);//string ctor
    BOOST_CHECK(Decimal("-30000000.007").get_string(".2f").compare("-30000000.01") == 0);//string ctor

}

BOOST_AUTO_TEST_SUITE_END();//TestDecimal
