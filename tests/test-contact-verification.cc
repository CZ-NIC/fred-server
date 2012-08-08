/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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

#include "mailer_manager.h"
#include "contact_verification/contact_verification_impl.h"

//test-contact-verification.cc

BOOST_AUTO_TEST_SUITE(TestContactVerification)

const std::string server_name = "test-contact-verification";



static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}


BOOST_AUTO_TEST_CASE( test_createConditionalIdentification_1 )
{
    //CORBA init
    FakedArgs orb_fa = CfgArgs::instance()->fa;
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
        , ns_args_ptr->nameservice_host
        , ns_args_ptr->nameservice_port
        , ns_args_ptr->nameservice_context);

    const std::auto_ptr<Registry::Contact::Verification::ContactVerificationImpl> cv(
        new Registry::Contact::Verification::ContactVerificationImpl(server_name
            , boost::shared_ptr<Fred::Mailer::Manager>(
                new MailerManager(CorbaContainer::get_instance()
                ->getNS()))));

    unsigned long long request_id =0;

    //get db connection
    Database::Connection conn = Database::Manager::acquire();

    //get registrar id
    std::string registrar_handle = "REG-FRED_A";
    Database::Result res_reg = conn.exec_params(
            "SELECT id FROM registrar WHERE handle=$1::text",
            Database::query_param_list(registrar_handle));
    if(res_reg.size() == 0) {
        throw std::runtime_error("Registrar does not exist");
    }
    unsigned long long registrar_id = res_reg[0][0];

    Fred::Contact::Verification::Contact fcvc;
    RandomDataGenerator rdg;

    //create test contact
    std::string xmark = rdg.xnumstring(6);
    fcvc.handle=std::string("TESTCV-HANDLE")+xmark;
    fcvc.name=std::string("TESTCV-NAME")+xmark;
    fcvc.organization=std::string("TESTCV-ORG")+xmark;
    fcvc.street1=std::string("TESTCV-STR1")+xmark;
    fcvc. city=std::string("Praha");
    fcvc.postalcode=std::string("11150");
    fcvc.country=std::string("CZ");
    fcvc.telephone=std::string("728")+xmark;
    fcvc.email=std::string("test")+xmark+"@nic.cz";
    fcvc.ssn=std::string("1980-01-01");
    fcvc.ssntype=std::string("BIRTHDAY");
    fcvc.auth_info=rdg.xnstring(8);
    //unsigned long long contact_hid =
    Fred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);

    std::string another_request_id;
    cv->createConditionalIdentification(fcvc.handle, registrar_handle
            , request_id, another_request_id);

    cv->processConditionalIdentification(another_request_id
            , fcvc.auth_info, request_id);

    cv->processIdentification(fcvc.handle, fcvc.auth_info, request_id);

    BOOST_CHECK(1==1);

/*
    const std::string& get_server_name();

    unsigned long long createConditionalIdentification(
            const std::string & contact_handle
            , const std::string & registrar_handle
            , const unsigned long long log_id
            , std::string & request_id);

    unsigned long long processConditionalIdentification(
            const std::string & request_id
            , const std::string & password
            , const unsigned long long log_id);

    unsigned long long processIdentification(
            const std::string & contact_handle
            , const std::string & password
            , const unsigned long long log_id);

    std::string getRegistrarName(const std::string & registrar_handle);
*/
}

BOOST_AUTO_TEST_CASE( test_ex )
{
    /*
    BOOST_CHECK_EXCEPTION(Decimal("Infinity") != Decimal("-Infinity")
            , std::exception
            , check_std_exception);
    */
}

//threaded test and threaded test framework test
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
      //some tests
    ret.result = worker_param;
    }

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

BOOST_AUTO_TEST_CASE( test_some_threads )
{
    TestParams params;
    params.test_param = 9999 ;

// test
    threadedTest< TestThreadedWorker> (params, &result_check);
}


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

            for(unsigned long long i = 0 ; i < 10000 ; ++i)
            {

            }


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


BOOST_AUTO_TEST_CASE( simple_test_some_threaded )
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













BOOST_AUTO_TEST_SUITE_END();//TestDecimal
