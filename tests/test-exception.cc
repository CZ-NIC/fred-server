/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include "random_data_generator.h"
#include "concurrent_queue.h"

#include "exception-test.h"

#include "decimal/decimal.h"
/*
class ElapsedTimeFixture
{
    ElapsedTime et_;
public:
    ElapsedTimeFixture()
    : et_("elapsed time: ", cout_print())
    {}
} ElapsedTimeFixture_instance;
*/

ExceptionTest& ExceptionTest::instance()
{
    static ExceptionTest instance;
    return instance;//with g++ it is thread safe
}

void test_decimal_wrapper_exceptions_fun()
{
    /*
    std::string a("test");
    a+="test";
    a+="test";
*/
//#if 0

    (Decimal("1") < Decimal("2"));
    (Decimal("1") <= Decimal("2"));
    (Decimal("1.1") < Decimal("1.2"));
    (Decimal("1000") > Decimal("100"));
    (Decimal("1000") >= Decimal("100"));
    (Decimal("1000") >= Decimal("1000"));
    (Decimal("1.1") == Decimal("1.1"));
    (Decimal("1.1") != Decimal("1.2"));
    (Decimal("1.11") > Decimal("-1.11"));
    ((Decimal("1.111") + Decimal("1.222")) == Decimal("2.333"));
    ((Decimal("2.333") - Decimal("1.111")) == Decimal("1.222"));
    ((Decimal("2.333") - Decimal("1.111")) != Decimal("1.223"));
    ((Decimal("1.111") * Decimal("1.222")) == Decimal("1.357642"));

    ((Decimal("1.222") / Decimal("1.111"))
            .round_half_up(19)
            .round_half_up(9) == Decimal("1.099909991"));

    ((Decimal("13").integral_division(Decimal("3"))) == Decimal("4"));
    ((Decimal("13").integral_division_remainder(Decimal("3"))) == Decimal("1"));

    (Decimal("-1").abs() == Decimal("1"));
    (Decimal("1").abs() == Decimal("1"));

    (Decimal("-1").is_negative());

    (Decimal("1 000").is_nan());
    (Decimal("1 000").is_special());
    (Decimal("1,1").is_nan());
    (Decimal("1,1").is_special());
/*
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
*/
    (Decimal("Infinity").is_infinite());//Infinity
    (Decimal("-Infinity").is_infinite());//Infinity
    (Decimal("NaN").is_nan());//NaN
    (Decimal("-NaN").is_nan());//-NaN

    (Decimal().is_nan());//default ctor init check
    (Decimal().get_string().compare("") == 0);//default ctor init check

    (Decimal(std::string()).is_nan());//string ctor
    (Decimal(std::string()).get_string().compare("") == 0);//string ctor

    (Decimal(std::string("1.1")) == Decimal("1.1"));//string ctor

/*
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
*/

    Decimal a("1234567890.123456789", 500);

    Decimal b = a;//copy ctor
    (b == a);
    (b.get_precision() == 500);

    Decimal c;
    c = a;//assignment operator
    (c == a);
    (c.get_precision() == 500);

    //stream operators
    Decimal dstream1("111.333");
    Decimal dstream2;
    std::stringstream sstr;
    sstr << dstream1;
    sstr >> dstream2;
    (dstream1 == dstream2);

    Decimal dec_2;
    Decimal dec_3("111.222");
    Decimal dec_4("111.222");
    dec_2.swap(dec_3);

    (dec_2 == dec_4);

    //format and round money
    (Decimal("10").get_string(".2f").compare("10.00") == 0);//string ctor
    (Decimal("10.1").get_string(".2f").compare("10.10") == 0);//string ctor
    (Decimal("-10").get_string(".2f").compare("-10.00") == 0);//string ctor
    (Decimal("-10.1").get_string(".2f").compare("-10.10") == 0);//string ctor

    (Decimal("10.005").get_string(".2f").compare("10.01") == 0);//string ctor
    (Decimal("10000000.1").get_string(".2f").compare("10000000.10") == 0);//string ctor
    (Decimal("-10.005").get_string(".2f").compare("-10.01") == 0);//string ctor
    (Decimal("-10000000.1").get_string(".2f").compare("-10000000.10") == 0);//string ctor
    (Decimal("30000000.1").get_string(".2f").compare("30000000.10") == 0);//string ctor
    (Decimal("-30000000.1").get_string(".2f").compare("-30000000.10") == 0);//string ctor

    (Decimal("30000000.001").get_string(".2f").compare("30000000.00") == 0);//string ctor
    (Decimal("-30000000.001").get_string(".2f").compare("-30000000.00") == 0);//string ctor
    (Decimal("30000000.007").get_string(".2f").compare("30000000.01") == 0);//string ctor
    (Decimal("-30000000.007").get_string(".2f").compare("-30000000.01") == 0);//string ctor

    //this have to be disabled by: supported_types_of_Decimal_<T>::Type()
    //Decimal d0(0);
    //Decimal d1(1);
//#endif
}



struct test_decimal_wrapper_exceptions
{
    test_decimal_wrapper_exceptions()
    {
    unsigned long long ex_count = 0;
    for(unsigned long long i = 0; i < ex_count+1; ++i)
    {
        ExceptionTest::instance().count();//start count
        test_decimal_wrapper_exceptions_fun();
        if( ex_count< ExceptionTest::instance().get_iteration_count())
            ex_count = ExceptionTest::instance().get_iteration_count();
        ExceptionTest::instance().end_test();
        std::cout << "ex_count: " << ex_count << std::endl;
    }//for i

    ExceptionTest::instance().start_test();//end count, start test

    for(unsigned long long i = 0; i < ex_count; ++i)
    {
        puts("test_decimal_wrapper_exceptions for start");
        try
        {
            test_decimal_wrapper_exceptions_fun();
        }
        catch(const std::exception& ex)
        {
            puts("catch std::exception");
            puts(ex.what());
        }

        /* this is not working - Boost.Test framework internal error: unknown reason
        BOOST_CHECK_EXCEPTION( test_decimal_wrapper_exceptions_fun();
            , std::exception, check_std_exception); */

        ExceptionTest::instance().next_path();
        puts("test_decimal_wrapper_exceptions for end");
    }//fro i

    unsigned long long iterations = ExceptionTest::instance().get_iteration_count();
    unsigned long long paths = ExceptionTest::instance().get_path_count();
    ExceptionTest::instance().end_test();//end test

    std::cout
    << "iterations: " << iterations
    << " paths: " << paths
            << std::endl;
    }
    virtual ~test_decimal_wrapper_exceptions(){}

};

bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

bool check_test_std_exception(std::exception const & ex)
{
    puts(ex.what());
    unsigned what_len = strlen(ex.what());
    char look_for[]="test exception";
    unsigned look_for_len = strlen(look_for);
    if(what_len != look_for_len ) return false;
    for(unsigned i = 0; i < look_for_len; ++i)
        if(look_for[i] != ex.what()[i] ) return false;
    return true;
}

//global operator new and delete overload
void* operator new(std::size_t size) throw(std::bad_alloc)
{
    ExceptionTest::instance().exception_test(std::bad_alloc());
    void* ptr = std::malloc(size);
    if (ptr == 0) throw std::bad_alloc();
    return ptr;
}

void* operator new(std::size_t size, const std::nothrow_t&) throw()
{
    return std::malloc(size);
}

void* operator new[](std::size_t size) throw(std::bad_alloc)
{
    ExceptionTest::instance().exception_test(std::bad_alloc());
    return operator new(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) throw()
{
    return operator new(size, std::nothrow);
}

void operator delete(void* ptr) throw()
{
    std::free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) throw()
{
    std::free(ptr);
}

void operator delete[](void* ptr) throw()
{
    operator delete(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) throw()
{
    operator delete(ptr);
}



int main(int argc, char * argv[])
{
    test_decimal_wrapper_exceptions test_decimal_wrapper_exceptions_instance;
    return 0;
}
