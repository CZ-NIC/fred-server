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
#ifndef EXCEPTION_TEST_HH_06ECEA65269043B79A1CA9A1FB27CFAE
#define EXCEPTION_TEST_HH_06ECEA65269043B79A1CA9A1FB27CFAE
#include <exception>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>


struct TestEx : virtual std::exception
{
    TestEx() noexcept{}//ctor
    TestEx(const TestEx&) noexcept {} //copy
    virtual ~TestEx() {} //dtor
    virtual const char* what() const noexcept
    {
        return "test exception";
    }
};

bool check_test_std_exception(std::exception const & ex);
bool check_std_exception(std::exception const & ex);

class ExceptionTest
{
    bool do_iteration;
    bool do_test;
    unsigned long long path_counter;
    unsigned long long iteration_counter;

    ExceptionTest()
    : do_iteration(false)
    , do_test(false)
    , path_counter(0)
    , iteration_counter(0)
    {}

public:

    static ExceptionTest& instance();

    void count()
    {
        do_iteration = true;
        do_test = false;
        iteration_counter = 0;
        path_counter = 1;
    }


    void start_test()
    {
        //puts("ExceptionTest::start_test() start");
        do_iteration = true;
        do_test = true;
        iteration_counter = 0;
        path_counter = 1;
        //puts("ExceptionTest::start_test() end");
    }
    void end_test()
    {
        //puts("ExceptionTest::end_test() start");
        do_iteration = false;
        do_test = false;
        iteration_counter = 0;
        path_counter = 1;
        //puts("ExceptionTest::end_test() end");
    }
    template <class TestException> void exception_test( TestException ex)
    {

        if(do_iteration)
        {
            //puts("ExceptionTest::exception_test() do_iteration");
            ++iteration_counter;
            if(do_test && ((path_counter % iteration_counter) == 0))
            {

                //puts("ExceptionTest::exception_test() throw");
                //char iteration_counter_str[40]={0};
                //char path_counter_str[40]={0};
                //snprintf(iteration_counter_str, sizeof(iteration_counter_str),"iteration_counter: %llu", iteration_counter);
                //snprintf(path_counter_str,sizeof(path_counter_str), "path_counter: %llu", path_counter);
                //puts(iteration_counter_str);
                //puts(path_counter_str);
                //printf("PID of this process: %d\n", getpid());
                //printf("The ID of this thread is: %u\n", (unsigned int)pthread_self());


                ++path_counter;
                throw ex;
            }
        }
    }
    void next_path()
    {
        ++path_counter;
    }

    unsigned long long get_iteration_count() const
    {
        return iteration_counter;
    }

    unsigned long long get_path_count() const
    {
        return path_counter;
    }


};//class ExceptionTest



#endif
