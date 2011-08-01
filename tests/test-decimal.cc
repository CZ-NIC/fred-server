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
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "test-common-threaded.h"

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

bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
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
            , check_std_exception);//Infinity

    BOOST_CHECK_EXCEPTION((Decimal("0") / Decimal("0")).is_nan()
            , std::exception
            , check_std_exception);

    BOOST_CHECK_EXCEPTION((Decimal("1") / Decimal("-0")).is_infinite()
            , std::exception
            , check_std_exception
            );//-Infinity

    BOOST_CHECK(Decimal("Infinity").is_infinite());//Infinity
    BOOST_CHECK(Decimal("-Infinity").is_infinite());//Infinity
    BOOST_CHECK(Decimal("NaN").is_nan());//NaN
    BOOST_CHECK(Decimal("-NaN").is_nan());//-NaN

    BOOST_CHECK(Decimal().is_nan());//default ctor init check
    BOOST_CHECK(Decimal().get_string().compare("") == 0);//default ctor init check

    BOOST_CHECK(Decimal(std::string()).is_nan());//string ctor
    BOOST_CHECK(Decimal(std::string()).get_string().compare("") == 0);//string ctor

    BOOST_CHECK(Decimal(std::string("1.1")) == Decimal("1.1"));//string ctor


    BOOST_CHECK_EXCEPTION(Decimal("Infinity") == Decimal("Infinity")
            , std::exception
            , check_std_exception);
    BOOST_CHECK_EXCEPTION(Decimal("Infinity") != Decimal("-Infinity")
            , std::exception
            , check_std_exception);
    BOOST_CHECK_EXCEPTION(Decimal("NaN") != Decimal("NaN")
            , std::exception
            , check_std_exception);
    BOOST_CHECK_EXCEPTION(Decimal("NaN") != Decimal("-NaN")
            , std::exception
            , check_std_exception);

    BOOST_CHECK_EXCEPTION( (Decimal("Infinity") + Decimal("-Infinity")).is_nan()
            , std::exception
            , check_std_exception);

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

    Decimal dec_2;
    Decimal dec_3("111.222");
    Decimal dec_4("111.222");
    dec_2.swap(dec_3);

    BOOST_CHECK(dec_2 == dec_4);

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

    //this have to be disabled by: supported_types_of_Decimal_<T>::Type()
    //Decimal d0(0);
    //Decimal d1(1);

}

//decimal threaded test and threaded test framework test
struct TestParams
{
    unsigned long long test_param;
    TestParams() : test_param(1)
    { }
};

struct ResultTest : TestParams {
    unsigned long long result;
    ResultTest()
    : TestParams()//this shall get called anyway
    , result(0)
    { }
};

ResultTest testWorker(unsigned long long worker_param)
{
    ResultTest ret;

    ret.result = 0;

    for(unsigned long long i = 0 ; i < worker_param ; ++i)
    {
        if(!(Decimal("1") < Decimal("2"))) ret.result += 1;
        if(!(Decimal("1") <= Decimal("2"))) ret.result += 2;
        if(!(Decimal("1.1") < Decimal("1.2"))) ret.result += 4;
        if(!(Decimal("1000") > Decimal("100"))) ret.result += 8;
        if(!(Decimal("1000") >= Decimal("100"))) ret.result += 16;
        if(!(Decimal("1000") >= Decimal("1000"))) ret.result += 32;
        if(!(Decimal("1.1") == Decimal("1.1"))) ret.result += 64;
        if(!(Decimal("1.1") != Decimal("1.2"))) ret.result += 128;
        if(!(Decimal("1.11") > Decimal("-1.11"))) ret.result += 256;
        if(!(((Decimal("1.111") + Decimal("1.222"))) == Decimal("2.333"))) ret.result += 512;
        if(!(((Decimal("2.333") - Decimal("1.111"))) == Decimal("1.222"))) ret.result += 1024;
        if(!(((Decimal("2.333") - Decimal("1.111"))) != Decimal("1.223"))) ret.result += 2048;
        if(!(((Decimal("1.111") * Decimal("1.222"))) == Decimal("1.357642"))) ret.result += 4096;
        if(!((Decimal("1.222") / Decimal("1.111")).round(19, MPD_ROUND_HALF_UP)
            .round(9, MPD_ROUND_HALF_UP) == Decimal("1.099909991"))) ret.result += 8192;
        if(!((Decimal("13").integral_division(Decimal("3"))) == Decimal("4"))) ret.result += 16384;
        if(!((Decimal("13").integral_division_remainder(Decimal("3"))) == Decimal("1"))) ret.result += 32768;
        if(!(Decimal("-1").abs() == Decimal("1"))) ret.result += 65536;
        if(!(Decimal("1").abs() == Decimal("1"))) ret.result += 131072;
        if(!(Decimal("-1").is_negative())) ret.result += 262144;
        if(!(Decimal().get_string().compare("") == 0)) ret.result += 524288;//default ctor init check
        if(!(Decimal(std::string()).is_nan())) ret.result += 1048576;//string ctor
        if(!(Decimal(std::string()).get_string().compare("") == 0)) ret.result += 2097152;//string ctor
        if(!(Decimal(std::string("1.1")) == Decimal("1.1"))) ret.result += 4194304;//string ctor

        Decimal a("1234567890.123456789", 500);
        Decimal b = a;//copy ctor
        if(!(b == a)) ret.result += 8388608;
        if(!(b.get_precision() == 500)) ret.result += 16777216;

        Decimal c;
        c = a;//assignment operator
        if(!(c == a)) ret.result += 33554432;
        if(!(c.get_precision() == 500)) ret.result += 67108864;

        //stream operators
        Decimal dstream1("111.333");
        Decimal dstream2;
        std::stringstream sstr;
        sstr << dstream1;
        sstr >> dstream2;
        if(!(dstream1 == dstream2)) ret.result += 134217728;

        Decimal dec_2;
        Decimal dec_3("111.222");
        Decimal dec_4("111.222");
        dec_2.swap(dec_3);

        if(!(dec_2 == dec_4)) ret.result += 268435456;

        //format and round money
        if(!(Decimal("10").get_string(".2f").compare("10.00") == 0)) ret.result +=    536870912ULL;//string ctor
        if(!(Decimal("10.1").get_string(".2f").compare("10.10") == 0)) ret.result += 1073741824ULL;//string ctor
        if(!(Decimal("-10").get_string(".2f").compare("-10.00") == 0)) ret.result += 2147483648ULL;//string ctor
        if(!(Decimal("-10.1").get_string(".2f").compare("-10.10") == 0)) ret.result += 4294967296ULL;//string ctor

        if(!(Decimal("10.005").get_string(".2f").compare("10.01") == 0)) ret.result +=              8589934592ULL;//string ctor
        if(!(Decimal("10000000.1").get_string(".2f").compare("10000000.10") == 0)) ret.result +=   17179869184ULL;//string ctor
        if(!(Decimal("-10.005").get_string(".2f").compare("-10.01") == 0)) ret.result +=           34359738368ULL;//string ctor
        if(!(Decimal("-10000000.1").get_string(".2f").compare("-10000000.10") == 0)) ret.result += 68719476736ULL;//string ctor
        if(!(Decimal("30000000.1").get_string(".2f").compare("30000000.10") == 0)) ret.result += 137438953472ULL;//string ctor
        if(!(Decimal("-30000000.1").get_string(".2f").compare("-30000000.10") == 0)) ret.result += 274877906944ULL;//string ctor

        if(!(Decimal("30000000.001").get_string(".2f").compare("30000000.00") == 0)) ret.result += 549755813888ULL;//string ctor
        if(!(Decimal("-30000000.001").get_string(".2f").compare("-30000000.00") == 0)) ret.result += 1099511627776ULL;//string ctor
        if(!(Decimal("30000000.007").get_string(".2f").compare("30000000.01") == 0)) ret.result += 2199023255552ULL;//string ctor
        if(!(Decimal("-30000000.007").get_string(".2f").compare("-30000000.01") == 0)) ret.result += 4398046511104ULL;//string ctor

        if(ret.result != 0) break;
    }
    //some tests
    //ret.result = worker_param;

    return ret;
}

class TestThreadedWorker : public ThreadedTestWorker<ResultTest, TestParams>
{
public:
    typedef ThreadedTestWorker<ResultTest, TestParams>::ThreadedTestResultQueue queue_type;

    TestThreadedWorker(unsigned number
             , boost::barrier* sb
             , std::size_t thread_group_divisor
             , queue_type* result_queue
             , TestParams params)
        : ThreadedTestWorker<ResultTest, TestParams>(number, sb, thread_group_divisor, result_queue, params)
    { }

    ResultTest run(const TestParams &p)
    {
       ResultTest ret = testWorker(p.test_param);
       return ret;
    }
};

void result_check(const ResultTest &res)
{
    BOOST_CHECK(res.result == 0 );
}

BOOST_AUTO_TEST_CASE( test_decimal_wrapper_threads )
{
    TestParams params;
    params.test_param = 9999 ;

// test
    threadedTest< TestThreadedWorker> (params, &result_check);
}

BOOST_AUTO_TEST_SUITE_END();//TestDecimal
