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

#include "src/util/setup_server_decl.hh"
#include "src/util/time_clock.hh"
#include "src/util/random_data_generator.hh"
#include "src/util/concurrent_queue.hh"


#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

#include "test/setup/test_common_threaded.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>
#include <utility>

#include "src/util/decimal/decimal.hh"

BOOST_AUTO_TEST_SUITE(TestDecimal)

const std::string server_name = "test-decimal";


/*
static bool check_std_exception_MPD(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("MPD")) != std::string::npos);
}
*/
static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}


BOOST_AUTO_TEST_CASE( test_division )
{
    Decimal payment("50000");
    Decimal price("12240");
    Decimal vat_coef("0.1667");
    Decimal price_vat = price * vat_coef / (Decimal("1") - vat_coef);//current alg.

    Decimal test_payment ("18000");

    for(unsigned long long i = 0; i < 1000000; ++i)
    {
        Decimal test_payment_vat = test_payment*vat_coef;
        Decimal test_price_vat = price * test_payment_vat / (test_payment - test_payment_vat);

        BOOST_CHECK(price_vat == test_price_vat);

        if(price_vat != test_price_vat)
        {
            std::cout << "test_payment: " << test_payment.get_string()
                    << " price_vat: " << price_vat.get_string()
                    << " test_price_vat: " << test_price_vat.get_string()
                    << std::endl;
        }

        test_payment+=Decimal("111.11");
    }//for
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
            .round_half_up(19)
            .round_half_up(9) == Decimal("1.099909991"));

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


    BOOST_CHECK_EXCEPTION((void)(Decimal("Infinity") == Decimal("Infinity"))
            , std::exception
            , check_std_exception);
    BOOST_CHECK_EXCEPTION((void)(Decimal("Infinity") != Decimal("-Infinity"))
            , std::exception
            , check_std_exception);
    BOOST_CHECK_EXCEPTION((void)(Decimal("NaN") != Decimal("NaN"))
            , std::exception
            , check_std_exception);
    BOOST_CHECK_EXCEPTION((void)(Decimal("NaN") != Decimal("-NaN"))
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

static ResultTest testWorker(unsigned long long worker_param)
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
        if(!((Decimal("1.222") / Decimal("1.111")).round_half_up(19)
            .round_half_up(9) == Decimal("1.099909991"))) ret.result += 8192;
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

static void result_check(const ResultTest &res)
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


//simple Decimal caclulation threaded test

//synchronization using barriers
struct sync_barriers
{
    boost::barrier barrier;

    sync_barriers(std::size_t number_of_threads)
        : barrier(number_of_threads)
    {}
};

struct ThreadResult
{
    unsigned number;//thread number
    unsigned ret;//return code
    std::string desc;//some closer description
    ThreadResult()
        : number(0),
          ret(std::numeric_limits<unsigned>::max()),
          desc("empty result")
    {}
};

typedef concurrent_queue<ThreadResult > ThreadResultQueue;

//thread functor
class SimpleTestThreadWorker
{
public:
    SimpleTestThreadWorker(
            unsigned number,
            unsigned sleep_time,
            sync_barriers* sb_ptr,
            ThreadResultQueue* result_queue_ptr = nullptr,
            unsigned seed = 0)
        : number_(number),
          sleep_time_(sleep_time),
          sb_ptr_(sb_ptr),
          rdg_(seed),
          rsq_ptr_(result_queue_ptr)
    {}

    void operator()()
    {
        ThreadResult res;
        res.number = number_;
        res.ret = 0;
        res.desc = "ok";
        int number_of_entered_barriers = 0;

        try
        {
            //std::cout << "waiting: " << number_ << std::endl;
            if (sb_ptr_ != nullptr)
            {
                sb_ptr_->barrier.wait();//wait for other synced threads
                ++number_of_entered_barriers;
            }
            //std::cout << "start: " << number_ << std::endl;

            for (unsigned long long i = 0 ; i < 10000 ; ++i)
            {
                if (!(Decimal("1") < Decimal("2"))) res.ret += 1;
                if (!(Decimal("1") <= Decimal("2"))) res.ret += 2;
                if (!(Decimal("1.1") < Decimal("1.2"))) res.ret += 4;
                if (!(Decimal("1000") > Decimal("100"))) res.ret += 8;
                if (!(Decimal("1000") >= Decimal("100"))) res.ret += 16;
                if (!(Decimal("1000") >= Decimal("1000"))) res.ret += 32;
                if (!(Decimal("1.1") == Decimal("1.1"))) res.ret += 64;
                if (!(Decimal("1.1") != Decimal("1.2"))) res.ret += 128;
                if (!(Decimal("1.11") > Decimal("-1.11"))) res.ret += 256;
                if (!(((Decimal("1.111") + Decimal("1.222"))) == Decimal("2.333"))) res.ret += 512;
                if (!(((Decimal("2.333") - Decimal("1.111"))) == Decimal("1.222"))) res.ret += 1024;
                if (!(((Decimal("2.333") - Decimal("1.111"))) != Decimal("1.223"))) res.ret += 2048;
                if (!(((Decimal("1.111") * Decimal("1.222"))) == Decimal("1.357642"))) res.ret += 4096;
                if (!((Decimal("1.222") / Decimal("1.111")).round_half_up(19).round_half_up(9) == Decimal("1.099909991")))
                {
                    res.ret += 8192;
                }
            }


        }
        catch (const std::exception& e)
        {
            //BOOST_TEST_MESSAGE("exception 1 in operator() thread number: " << number_ << " reason: " << e.what());
            res.ret = 134217728;
            res.desc = e.what();
        }
        catch (...)
        {
            //BOOST_TEST_MESSAGE("exception 2 in operator() thread number: " << number_);
            res.ret = 268435456;
            res.desc = "unknown exception";
        }

        if (sb_ptr_ != nullptr)
        {
            while (number_of_entered_barriers < 1)
            {
                sb_ptr_->barrier.wait();//wait for other synced threads
                ++number_of_entered_barriers;
            }
        }
        if (rsq_ptr_ != nullptr)
        {
            rsq_ptr_->guarded_access().push(res);
        }
        //std::cout << "end: " << number_ << std::endl;
    }
private:
    //need only defaultly constructible members here
    unsigned number_;//thred identification
    unsigned sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr_; //result queue non-owning pointer
};//class SimpleTestThreadWorker


BOOST_AUTO_TEST_CASE(simple_test_decimal_threaded)
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const number_of_threads = thread_args_ptr->number_of_threads;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<SimpleTestThreadWorker> tw_vector;
    tw_vector.reserve(number_of_threads);

    //synchronization barriers instance
    sync_barriers sb(number_of_threads);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        tw_vector.push_back(SimpleTestThreadWorker(i,3,&sb, &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " << result_queue.unguarded_access().size());

    while (!result_queue.unguarded_access().empty())
    {
        const auto thread_result = result_queue.unguarded_access().pop();

        if (thread_result.ret != 0)
        {
            BOOST_FAIL( thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestDecimal
