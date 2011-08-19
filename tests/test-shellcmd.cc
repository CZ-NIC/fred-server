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

#include "subprocess.h"

BOOST_AUTO_TEST_SUITE(TestShellCmd)

const std::string server_name = "test-shellcmd";


static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

BOOST_AUTO_TEST_CASE( test_shellcmd_wrapper )
{

    SubProcessOutput sub_output1 = ShellCmd(
        "echo kuk",10).execute();
/*
    std::cout << "test_shellcmd_wrapper sub_output1.stdout: " << sub_output1.stdout
        << " sub_output1.stderr: " << sub_output1.stderr << std::endl;
*/
    BOOST_CHECK(sub_output1.stderr.empty());
    BOOST_CHECK(sub_output1.stdout.compare("kuk\n") == 0);


    SubProcessOutput sub_output2 = ShellCmd(
        "head", "/bin/bash",10).execute("kuk");
/*
    std::cout << "test_shellcmd_wrapper sub_output2.stdout: " << sub_output2.stdout
        << " sub_output2.stderr: " << sub_output2.stderr << std::endl;
*/
    BOOST_CHECK(sub_output2.stderr.empty());
    BOOST_CHECK(sub_output2.stdout.compare("kuk\n") == 0);

    {
        SubProcessOutput sub_output;
        try
        {
            sub_output = ShellCmd(
                    "head", "/bin/bash",10).execute("kuk");
        }
        catch(const std::exception& ex)
        {
            std::cout << "std::exception: " << ex.what() << std::endl;
        }
/*
        std::cout << "test_shellcmd_wrapper sub_output.stdout: " << sub_output.stdout
            << " sub_output.stderr: " << sub_output.stderr << std::endl;
*/
        BOOST_CHECK(sub_output.stderr.empty());
        BOOST_CHECK(sub_output.stdout.compare("kuk\n") == 0);
    }

    //checks from notify_registered_letters_manual_send_impl
    //if rm is there
    {
      SubProcessOutput sub_output = ShellCmd("rm --version", 10).execute();
      BOOST_CHECK(sub_output.stderr.empty());
    }
    //if gs is there
    {
      SubProcessOutput sub_output = ShellCmd("gs --version", 10).execute();
      BOOST_CHECK(sub_output.stderr.empty());
    }
    //if base64 is there
    {
      SubProcessOutput sub_output = ShellCmd("base64 --version", 10).execute();
      BOOST_CHECK(sub_output.stderr.empty());
    }
    //if sendmail is there
    {
      SubProcessOutput sub_output = ShellCmd("ls /usr/sbin/sendmail", 10).execute();
      BOOST_CHECK(sub_output.stderr.empty());
    }

}

//TODO: test with threads
#if 0
//shell cmd threaded test
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

BOOST_AUTO_TEST_CASE( test_shellcmd_wrapper_threads )
{
    TestParams params;
    params.test_param = 9999 ;

// test
    threadedTest< TestThreadedWorker> (params, &result_check);
}


//simple shell wrapper threaded test

//synchronization using barriers
struct sync_barriers
{
    boost::barrier barrier;

    sync_barriers(std::size_t thread_number)
        : barrier(thread_number)
    {}
};

struct ThreadResult
{
    unsigned number;//thread number
    unsigned ret;//return code
    std::string desc;//some closer description
    ThreadResult()
    : number(0)
      , ret(std::numeric_limits<unsigned>::max())
      , desc("empty result")
      {}
};

typedef concurrent_queue<ThreadResult > ThreadResultQueue;

//thread functor
class SimpleTestThreadWorker
{
public:

    SimpleTestThreadWorker(unsigned number,unsigned sleep_time
            , sync_barriers* sb_ptr
            , ThreadResultQueue* result_queue_ptr = 0, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , rdg_(seed)
            , rsq_ptr (result_queue_ptr)
    {}

    void operator()()
    {
        ThreadResult res;
        res.number = number_;
        res.ret = 0;
        res.desc = std::string("ok");

        try
        {
            //std::cout << "waiting: " << number_ << std::endl;
            if(sb_ptr_)
                sb_ptr_->barrier.wait();//wait for other synced threads
            //std::cout << "start: " << number_ << std::endl;


            //some tests

        }
        catch(const std::exception& ex)
        {
            BOOST_TEST_MESSAGE("exception 1 in operator() thread number: " << number_
                    << " reason: " << ex.what() );
            res.ret = 134217728;
            res.desc = std::string(ex.what());
            return;
        }
        catch(...)
        {
            BOOST_TEST_MESSAGE("exception 2 in operator() thread number: " << number_ );
            res.ret = 268435456;
            res.desc = std::string("unknown exception");
            return;
        }

        if(rsq_ptr) rsq_ptr->push(res);
        //std::cout << "end: " << number_ << std::endl;
    }

private:
    //need only defaultly constructible members here
    unsigned    number_;//thred identification
    unsigned    sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr; //result queue non-owning pointer
};//class SimpleTestThreadWorker


BOOST_AUTO_TEST_CASE( simple_test_shellcmd_threaded )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<SimpleTestThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    //synchronization barriers instance
    sync_barriers sb(thread_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(SimpleTestThreadWorker(i,3,&sb, &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE( "threads end result_queue.size(): " << result_queue.size() );

    for(unsigned i = 0; i < thread_number; ++i)
    {
        ThreadResult thread_result;
        if(!result_queue.try_pop(thread_result)) {
            continue;
        }

        if(thread_result.ret != 0)
        {
            BOOST_FAIL( thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc);
        }
    }//for i
}

#endif

BOOST_AUTO_TEST_SUITE_END();//TestShellCmd
