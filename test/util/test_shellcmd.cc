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
#include "util/random_data_generator.hh"
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

#include "src/util/subprocess.hh"

BOOST_AUTO_TEST_SUITE(TestShellCmd)

const std::string server_name = "test-shellcmd";

static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}
#if 1 //enables single-threaded tests
BOOST_AUTO_TEST_CASE(test_shellcmd_wrapper)
{
    for(int i = 0; i < 100; ++i)
    {
        SubProcessOutput sub_output1 = ShellCmd(
            "cat | tr u -","/bin/bash",10).execute("a u a u");
        BOOST_CHECK(sub_output1.stderr.empty());
        BOOST_CHECK(sub_output1.stdout.compare("a - a -") == 0);
        //BOOST_TEST_MESSAGE(sub_output1.stdout);

        SubProcessOutput sub_output3 = ShellCmd(
            " echo kuk | grep kuk | grep -v juk | grep kuk | grep -v juk",10).execute();
    /*
        std::cout << "test_shellcmd_wrapper sub_output3.stdout: " << sub_output3.stdout
            << " sub_output3.stderr: " << sub_output3.stderr << std::endl;
    */
        BOOST_CHECK(sub_output3.stderr.empty());
        BOOST_CHECK_EQUAL(sub_output3.stdout, "kuk\n");
    }
}

BOOST_AUTO_TEST_CASE(test_exec_wrapper)
{
    for (int i = 0; i < 100; ++i)
    {
        const SubProcessOutput sub_output1 = Cmd::Data("a u a u").into("tr")("u")("-").run_with_path(10);
        BOOST_CHECK(sub_output1.succeeded());
        BOOST_CHECK(sub_output1.stderr.empty());
        BOOST_CHECK(sub_output1.stdout == "a - a -");

        const SubProcessOutput sub_output2 =
            Cmd::Data(Cmd::Data(Cmd::Data(Cmd::Data
            ("kuk\n").into("grep")("kuk").run_with_path(10).stdout)
                     .into("grep")("-v")("juk").run_with_path(10).stdout)
                     .into("grep")("kuk").run_with_path(10).stdout)
                     .into("grep")("-v")("juk").run_with_path(10);
        BOOST_CHECK(sub_output2.succeeded());
        BOOST_CHECK(sub_output2.stderr.empty());
        BOOST_CHECK_EQUAL(sub_output2.stdout, "kuk\n");
    }
}

BOOST_AUTO_TEST_CASE(test_shellcmd_wrapper1)
{
    {
        SubProcessOutput sub_output1 = ShellCmd(
            "cat | tr u -","/bin/bash",10).execute("a u a u");
        BOOST_CHECK(sub_output1.stderr.empty());
        BOOST_CHECK_EQUAL(sub_output1.stdout, "a - a -");
        //BOOST_TEST_MESSAGE(sub_output1.stdout);
    }

    SubProcessOutput sub_output1 = ShellCmd("echo kuk","/bin/bash", 10).execute();
/*
    std::cout << "test_shellcmd_wrapper sub_output1.stdout: " << sub_output1.stdout
        << " sub_output1.stderr: " << sub_output1.stderr << std::endl;
*/
    BOOST_CHECK(sub_output1.stderr.empty());
    BOOST_CHECK_EQUAL(sub_output1.stdout, "kuk\n");


    SubProcessOutput sub_output2 = ShellCmd("head", "/bin/bash", 10).execute("kuk");
/*
    std::cout << "test_shellcmd_wrapper sub_output2.stdout: " << sub_output2.stdout
        << " sub_output2.stderr: " << sub_output2.stderr << std::endl;
*/
    BOOST_CHECK(sub_output2.stderr.empty());
    BOOST_CHECK_EQUAL(sub_output2.stdout, "kuk");

    {
        SubProcessOutput sub_output;
        sub_output = ShellCmd("head", "/bin/bash", 10).execute("kuk");
/*
        std::cout << "test_shellcmd_wrapper sub_output.stdout: " << sub_output.stdout
            << " sub_output.stderr: " << sub_output.stderr << std::endl;
*/
        BOOST_CHECK(sub_output.stderr.empty());
        BOOST_CHECK_EQUAL(sub_output.stdout, "kuk");
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

BOOST_AUTO_TEST_CASE(test_exec_wrapper1)
{
    {
        const SubProcessOutput sub_output = Cmd::Data("a u a u").into("tr")("u")("-").run_with_path(10);
        BOOST_CHECK(sub_output.succeeded());
        BOOST_CHECK(sub_output.stderr.empty());
        BOOST_CHECK_EQUAL(sub_output.stdout, "a - a -");
    }

    const SubProcessOutput sub_output1 = Cmd::Executable("echo")("kuk").run_with_path(10);
    BOOST_CHECK(sub_output1.succeeded());
    BOOST_CHECK(sub_output1.stderr.empty());
    BOOST_CHECK_EQUAL(sub_output1.stdout, "kuk\n");

    const SubProcessOutput sub_output2 = Cmd::Data("kuk").into("head").run_with_path(10);
    BOOST_CHECK(sub_output2.succeeded());
    BOOST_CHECK(sub_output2.stderr.empty());
    BOOST_CHECK_EQUAL(sub_output2.stdout, "kuk");

    {
        SubProcessOutput sub_output;
        sub_output = Cmd::Data("kuk").into("head").run_with_path(10);

        BOOST_CHECK(sub_output.succeeded());
        BOOST_CHECK(sub_output.stderr.empty());
        BOOST_CHECK_EQUAL(sub_output.stdout, "kuk");
    }

    //checks from notify_registered_letters_manual_send_impl
    //if rm is there
    {
        BOOST_CHECK(Cmd::Executable("which")("rm").run_with_path(10).succeeded());
    }
    //if gs is there
    {
        BOOST_CHECK(Cmd::Executable("which")("gs").run_with_path(10).succeeded());
    }
    //if base64 is there
    {
        BOOST_CHECK(Cmd::Executable("which")("base64").run_with_path(10).succeeded());
    }
    //if sendmail is there
    {
        BOOST_CHECK(Cmd::Executable("test")("-x")("/usr/sbin/sendmail").run_with_path(10).succeeded());
    }

}

BOOST_AUTO_TEST_CASE(test_shellcmd_wrapper_stdout)
{
    const std::size_t slen = 16777216;//1073741824;//536870912;//268435456;//134217728; //67108864;//33554432;//16777216;
    SubProcessOutput sub_output1 = ShellCmd("cat | tr u -", "/bin/bash", 10).execute(std::string(slen, 'u'));
    BOOST_CHECK(sub_output1.stderr.empty());
    BOOST_CHECK_EQUAL(sub_output1.stdout, std::string(slen, '-'));
    //std::cout << "sub_output1.stdout.length(): " << sub_output1.stdout.length() << " slen: " << slen << std::endl;
    BOOST_CHECK_EQUAL(sub_output1.stdout.length(), slen);
}

BOOST_AUTO_TEST_CASE(test_exec_wrapper_stdout)
{
    const std::size_t slen = 16777216;
    const SubProcessOutput sub_output1 = Cmd::Data(std::string(slen, 'u'))
        .into("tr")("u")("-").run_with_path(10);
    BOOST_CHECK(sub_output1.succeeded());
    BOOST_CHECK(sub_output1.stderr.empty());
    BOOST_CHECK_EQUAL(sub_output1.stdout, std::string(slen, '-'));
    BOOST_CHECK_EQUAL(sub_output1.stdout.length(), slen);
}

BOOST_AUTO_TEST_CASE(test_shellcmd_wrapper_stderr)
{
    const std::size_t slen = 16777216;//2147483648;//1073741824;//536870912;//268435456;//134217728; //67108864;//33554432;//16777216;
    const SubProcessOutput sub_output1 = ShellCmd("cat | tr u - 1>&2", "/bin/bash", 10).execute(std::string(slen, 'u'));
    BOOST_CHECK(sub_output1.stdout.empty());
    BOOST_CHECK_EQUAL(sub_output1.stderr, std::string(slen, '-'));
    BOOST_CHECK_EQUAL(sub_output1.stderr.length(), slen);
}

BOOST_AUTO_TEST_CASE(test_shellcmd_wrapper_timeout)
{
    static const std::size_t slen = 16;
    static const unsigned timeout_one_second = 1;
    const std::string data(slen, 'u');

    BOOST_CHECK_EXCEPTION(SubProcessOutput sub_output1 = ShellCmd(
        "cat | tr u - ; sleep 2", "/bin/bash", timeout_one_second).execute(data),
        std::exception,
        check_std_exception);

    BOOST_CHECK_EXCEPTION(SubProcessOutput sub_output1 = ShellCmd(
        "sleep 2; cat | tr u - ", "/bin/bash", timeout_one_second).execute(data),
        std::exception,
        check_std_exception);
}

BOOST_AUTO_TEST_CASE(test_exec_wrapper_timeout)
{
    static const unsigned timeout_one_second = 1;
    BOOST_CHECK_EXCEPTION(Cmd::Executable("sleep")("20").run_with_path(timeout_one_second),
        std::exception, check_std_exception);
}
#endif

//shell cmd threaded test
struct TestParams
{
    unsigned long long test_param;
    TestParams() : test_param(1)
    { }
};

struct ResultTest : TestParams
{
    unsigned long long result;
    ResultTest()
        : TestParams(),//this shall get called anyway
          result(0)
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

    TestThreadedWorker(
            unsigned number_of_threads,
            boost::barrier* sb,
            std::size_t thread_group_divisor,
            queue_type* result_queue,
            TestParams params)
    :   ThreadedTestWorker<ResultTest, TestParams>(
            number_of_threads,
            sb,
            thread_group_divisor,
            result_queue,
            params)
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
    params.test_param = 9999;

// test
    threadedTest<TestThreadedWorker>(params, &result_check);
}



//simple shell wrapper threaded test

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

typedef concurrent_queue<ThreadResult> ThreadResultQueue;

namespace {

class BoostTestMessageGuard
{
public:
    BoostTestMessageGuard()
    {
        static const int op_success = 0;
        const int retval = ::pthread_mutex_lock(&mutex_);
        if (retval != op_success)
        {
            throw std::runtime_error("pthread_mutex_lock() failure: " + std::string(std::strerror(retval)));
        }
    }
    ~BoostTestMessageGuard()
    {
        ::pthread_mutex_unlock(&mutex_);
    }
private:
    static ::pthread_mutex_t mutex_;
};

::pthread_mutex_t BoostTestMessageGuard::mutex_ = PTHREAD_MUTEX_INITIALIZER;

}//namespace {anonymous}

//thread functor
class SimpleTestThreadWorker
{
public:
    SimpleTestThreadWorker(
            unsigned thread_number,
            unsigned sleep_time,
            sync_barriers* sb_ptr,
            ThreadResultQueue *result_queue_ptr = nullptr,
            unsigned seed = 0)
        : number_(thread_number),
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
#if 0
            {
                std::ostringstream out;
                out << "waiting: " << number_ << std::endl;
                std::cout << out.str() << std::flush;
            }
#endif
            if (sb_ptr_ != nullptr)
            {
                sb_ptr_->barrier.wait();//wait for other synced threads
                ++number_of_entered_barriers;
            }
#if 0
            {
                std::ostringstream out;
                out << "start: " << number_ << std::endl;
                std::cout << out.str() << std::flush;
            }
#endif

            //some tests

            for (int cnt = 0; cnt < 1; ++cnt)
            {
                SubProcessOutput sub_output;// = ShellCmd("echo kuk | grep kuk | grep -v juk | grep kuk | grep -v juk", 10).execute();
                static const Cmd::Executable::Seconds timeout = 10;
                sub_output = ShellCmd("echo kuk | grep kuk | grep -v juk | grep kuk | grep -v juk", timeout).execute();

                if (!sub_output.stderr.empty())
                {
                    res.ret = 1;
                    res.desc = "stderr: " + sub_output.stderr;
                    break;
                }

                if (sub_output.stdout != "kuk\n")
                {
                    res.ret = 2;
                    res.desc = "expected kuk in stdout and got: " + sub_output.stdout;
                    break;
                }

                sub_output =
                    Cmd::Data(Cmd::Data(Cmd::Data(Cmd::Data
                        ("kuk\n").into("grep")("kuk").run_with_path(timeout).stdout)
                                 .into("grep")("-v")("juk").run_with_path(timeout).stdout)
                                 .into("grep")("kuk").run_with_path(timeout).stdout)
                                 .into("grep")("-v")("juk").run_with_path(timeout);

                if (!sub_output.succeeded() || !sub_output.stderr.empty())
                {
                    res.ret = 1;
                    res.desc = "stderr: " + sub_output.stderr;
                    break;
                }

                if (sub_output.stdout != "kuk\n")
                {
                    res.ret = 2;
                    res.desc = "expected kuk in stdout and got: " + sub_output.stdout;
                    break;
                }
#if 0
                //db
                Database::Connection conn = Database::Manager::acquire();

                unsigned long long number = conn.exec("select 1")[0][0];

                if(number != 1)
                {
                    res.ret = 3;
                    res.desc = std::string("select 1 failed");
                    break;
                }
#endif
            }

        }
        catch (const std::exception& e)
        {
            const BoostTestMessageGuard guard;
            BOOST_TEST_MESSAGE("exception 1 in operator() thread number: " << number_ << " reason: " << e.what());
            res.ret = 134217728;
            res.desc = e.what();
        }
        catch (...)
        {
            const BoostTestMessageGuard guard;
            BOOST_TEST_MESSAGE("exception 2 in operator() thread number: " << number_);
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
#if 0
        {
            std::ostringstream out;
            out << "end: " << number_ << std::endl;
            std::cout << out.str() << std::flush;
        }
#endif
    }

private:
    //need only defaultly constructible members here
    unsigned    number_;//thread identification
    unsigned    sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr_; //result queue non-owning pointer
};


BOOST_AUTO_TEST_CASE( test_shellcmd_threaded )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    const std::size_t number_of_threads = thread_args_ptr->number_of_threads;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<SimpleTestThreadWorker> tw_vector;
    tw_vector.reserve(number_of_threads);

    //synchronization barriers instance
    sync_barriers sb(number_of_threads);

    //thread container
    boost::thread_group threads;
    for (unsigned idx = 0; idx < number_of_threads; ++idx)
    {
        static const unsigned default_sleep_time = 3;
        tw_vector.push_back(SimpleTestThreadWorker(idx, default_sleep_time, &sb, &result_queue));
        threads.create_thread(tw_vector.at(idx));
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

//#endif

BOOST_AUTO_TEST_SUITE_END();//TestShellCmd
