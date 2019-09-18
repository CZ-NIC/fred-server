/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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

#include "src/util/concurrent_queue.hh"

#include "test/util/exception_test.hh"

#include "util/decimal/decimal.hh"

ExceptionTest& ExceptionTest::instance()
{
    static ExceptionTest instance;
    return instance;//with g++ it is thread safe
}

void test_decimal_wrapper_exceptions_fun()
{

    (void)(Decimal("1") < Decimal("2"));
    (void)(Decimal("1") <= Decimal("2"));
    (void)(Decimal("1.1") < Decimal("1.2"));
    (void)(Decimal("1000") > Decimal("100"));
    (void)(Decimal("1000") >= Decimal("100"));
    (void)(Decimal("1000") >= Decimal("1000"));
    (void)(Decimal("1.1") == Decimal("1.1"));
    (void)(Decimal("1.1") != Decimal("1.2"));
    (void)(Decimal("1.11") > Decimal("-1.11"));
    (void)((Decimal("1.111") + Decimal("1.222")) == Decimal("2.333"));
    (void)((Decimal("2.333") - Decimal("1.111")) == Decimal("1.222"));
    (void)((Decimal("2.333") - Decimal("1.111")) != Decimal("1.223"));
    (void)((Decimal("1.111") * Decimal("1.222")) == Decimal("1.357642"));

    (void)((Decimal("1.222") / Decimal("1.111"))
            .round_half_up(19)
            .round_half_up(9) == Decimal("1.099909991"));

    (void)((Decimal("13").integral_division(Decimal("3"))) == Decimal("4"));
    (void)((Decimal("13").integral_division_remainder(Decimal("3"))) == Decimal("1"));

    (void)(Decimal("-1").abs() == Decimal("1"));
    (void)(Decimal("1").abs() == Decimal("1"));

    (Decimal("-1").is_negative());

    (Decimal("1 000").is_nan());
    (Decimal("1 000").is_special());
    (Decimal("1,1").is_nan());
    (Decimal("1,1").is_special());

    (Decimal("Infinity").is_infinite());//Infinity
    (Decimal("-Infinity").is_infinite());//Infinity
    (Decimal("NaN").is_nan());//NaN
    (Decimal("-NaN").is_nan());//-NaN

    (Decimal().is_nan());//default ctor init check
    (void)(Decimal().get_string().compare("") == 0);//default ctor init check

    (Decimal(std::string()).is_nan());//string ctor
    (void)(Decimal(std::string()).get_string().compare("") == 0);//string ctor

    (void)(Decimal(std::string("1.1")) == Decimal("1.1"));//string ctor

    Decimal a("1234567890.123456789", 500);

    Decimal b = a;//copy ctor
    (void)(b == a);
    (void)(b.get_precision() == 500);

    Decimal c;
    c = a;//assignment operator
    (void)(c == a);
    (void)(c.get_precision() == 500);

    //stream operators
    Decimal dstream1("111.333");
    Decimal dstream2;
    std::stringstream sstr;
    sstr << dstream1;
    sstr >> dstream2;
    (void)(dstream1 == dstream2);

    Decimal dec_2;
    Decimal dec_3("111.222");
    Decimal dec_4("111.222");
    dec_2.swap(dec_3);

    (void)(dec_2 == dec_4);

    //format and round money
    (void)(Decimal("10").get_string(".2f").compare("10.00") == 0);//string ctor
    (void)(Decimal("10.1").get_string(".2f").compare("10.10") == 0);//string ctor
    (void)(Decimal("-10").get_string(".2f").compare("-10.00") == 0);//string ctor
    (void)(Decimal("-10.1").get_string(".2f").compare("-10.10") == 0);//string ctor

    (void)(Decimal("10.005").get_string(".2f").compare("10.01") == 0);//string ctor
    (void)(Decimal("10000000.1").get_string(".2f").compare("10000000.10") == 0);//string ctor
    (void)(Decimal("-10.005").get_string(".2f").compare("-10.01") == 0);//string ctor
    (void)(Decimal("-10000000.1").get_string(".2f").compare("-10000000.10") == 0);//string ctor
    (void)(Decimal("30000000.1").get_string(".2f").compare("30000000.10") == 0);//string ctor
    (void)(Decimal("-30000000.1").get_string(".2f").compare("-30000000.10") == 0);//string ctor

    (void)(Decimal("30000000.001").get_string(".2f").compare("30000000.00") == 0);//string ctor
    (void)(Decimal("-30000000.001").get_string(".2f").compare("-30000000.00") == 0);//string ctor
    (void)(Decimal("30000000.007").get_string(".2f").compare("30000000.01") == 0);//string ctor
    (void)(Decimal("-30000000.007").get_string(".2f").compare("-30000000.01") == 0);//string ctor

    //this have to be disabled by: supported_types_of_Decimal_<T>::Type()
    //Decimal d0(0);
    //Decimal d1(1);

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
            //std::cout << "ex_count: " << ex_count << std::endl;
        }//for i

        ExceptionTest::instance().start_test();//end count, start test

        for(unsigned long long i = 0; i < ex_count; ++i)
        {
            //puts("test_decimal_wrapper_exceptions for start");
            try
            {
                test_decimal_wrapper_exceptions_fun();
            }
            catch(const std::exception& ex)
            {
                if(strcmp(ex.what(),"std::bad_alloc") != 0)//expecting only bad alloc exeption
                {
                    puts("ERR catch std::exception");
                    puts(ex.what());
                }
            }

            /* this is not working - Boost.Test framework internal error: unknown reason
            BOOST_CHECK_EXCEPTION( test_decimal_wrapper_exceptions_fun();
                , std::exception, check_std_exception); */

            ExceptionTest::instance().next_path();
            //puts("test_decimal_wrapper_exceptions for end");
        }//fro i

        unsigned long long iterations = ExceptionTest::instance().get_iteration_count();
        unsigned long long paths = ExceptionTest::instance().get_path_count();
        ExceptionTest::instance().end_test();//end test

        std::cout
        << "iterations: " << iterations
        << " paths: " << paths
                << " OK" <<std::endl;
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
void* operator new(std::size_t size)
{
    ExceptionTest::instance().exception_test(std::bad_alloc());
    void* ptr = std::malloc(size);
    if (ptr == 0) throw std::bad_alloc();
    return ptr;
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    return std::malloc(size);
}

void* operator new[](std::size_t size)
{
    ExceptionTest::instance().exception_test(std::bad_alloc());
    return operator new(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    return operator new(size, std::nothrow);
}

void operator delete(void* ptr) noexcept
{
    std::free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    operator delete(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    operator delete(ptr);
}



int main()
{
    test_decimal_wrapper_exceptions test_decimal_wrapper_exceptions_instance;
    return 0;
}
