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

#ifndef EXCEPTIONTEST_H_
#define EXCEPTIONTEST_H_
#include <exception>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>


struct TestEx : virtual std::exception
{
    TestEx() throw(){}//ctor
    TestEx(const TestEx&) throw() {} //copy
    virtual ~TestEx() throw() {} //dtor
    virtual const char* what() const throw()
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

                puts("ExceptionTest::exception_test() throw");
                char iteration_counter_str[40]={0};
                char path_counter_str[40]={0};
                snprintf(iteration_counter_str, sizeof(iteration_counter_str)
                        ,"iteration_counter: %llu", iteration_counter);
                snprintf(path_counter_str,sizeof(path_counter_str)
                        , "path_counter: %llu", path_counter);
                puts(iteration_counter_str);
                puts(path_counter_str);
                printf("PID of this process: %d\n", getpid());
                printf("The ID of this thread is: %u\n", (unsigned int)pthread_self());


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



#endif // EXCEPTIONTEST_H_
