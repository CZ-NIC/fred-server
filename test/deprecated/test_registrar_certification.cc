/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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

#include <utility>

#include "test/deprecated/test_registrar_certification.hh"

BOOST_AUTO_TEST_SUITE(TestRegistrarCertification)

ModelFiles model_files;
unsigned long long model_file_id = 0;

BOOST_AUTO_TEST_CASE( test_registrar_certification_simple )
{
    //  try
    //  {

        //CORBA init
    BOOST_TEST_MESSAGE( "CORBA init" );
        FakedArgs fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

    BOOST_TEST_MESSAGE( "CorbaContainer::set_instance" );
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

        //std::cout << "ccReg::Admin::_narrow" << std::endl;
    BOOST_TEST_MESSAGE( "ccReg::Admin::_narrow" );
        ccReg::Admin_var admin_ref;
        admin_ref = ccReg::Admin::_narrow(CorbaContainer::get_instance()->nsresolve("Admin"));

    BOOST_TEST_MESSAGE( "Database::Connection");
        //get db connection
        Database::Connection conn = Database::Manager::acquire();


        //deletion of test data
        std::string query9 (
                "delete from registrar_certification "
                "where registrar_id = 1 ");
    BOOST_TEST_MESSAGE( "exec query: "<< query9 );
        conn.exec( query9 );

         //test file of type 6
        model_files.setName("test_name");
        model_files.setPath("test");
        model_files.setFilesize(5);
        model_files.setFileTypeId(6);
        model_files.insert();

        //std::cout << "admin_ref->getCertificationManager()" << std::endl;
        Registry::Registrar::Certification::Manager_var cert_manager_ref;
        cert_manager_ref = admin_ref->getCertificationManager();

        ccReg::TID cid1 =
                cert_manager_ref->createCertification(
                        1,
                        makeCorbaDate(boost::gregorian::day_clock::local_day()),
                        makeCorbaDate(boost::gregorian::day_clock::local_day() + boost::gregorian::years(1)),
                        3,
                        model_files.getId());
        std::string query8 (
                "select id from registrar_certification "
                "where registrar_id = 1  "
                "and valid_from = now()::date "
                "and valid_until = (now() + interval '1 year')::date "
                "and classification = 3 ");
    BOOST_TEST_MESSAGE( "exec query: "<< query8 );
        Database::Result res8 = conn.exec( query8 );
        BOOST_REQUIRE_EQUAL(6*res8.size() , 6);

        cert_manager_ref->updateCertification(cid1,4,model_files.getId());
        std::string query10 (
                "select id from registrar_certification "
                "where registrar_id = 1  "
                "and valid_from = now()::date "
                "and valid_until = (now() + interval '1 year')::date "
                "and classification = 4 ");
    BOOST_TEST_MESSAGE( "exec query: "<< query10 );
        Database::Result res10 = conn.exec( query10 );
        BOOST_REQUIRE_EQUAL(7*res10.size() , 7);

        cert_manager_ref->shortenCertification(cid1,
                makeCorbaDate(boost::gregorian::day_clock::local_day()));
        std::string query11 (
                "select id from registrar_certification "
                "where registrar_id = 1  "
                "and valid_from = now()::date "
                "and valid_until = now()::date "
                "and classification = 4 ");
    BOOST_TEST_MESSAGE( "exec query: "<< query11 );
        Database::Result res11 = conn.exec( query11 );
        BOOST_REQUIRE_EQUAL(8*res11.size() , 8);

        //unbounded struct sequence client side mapping
        //create owning seq

        Registry::Registrar::Certification::CertificationList_var
            cert_by_reg(new Registry::Registrar::Certification::CertificationList);
        cert_by_reg = cert_manager_ref->getCertificationsByRegistrar(1);
        const Registry::Registrar::Certification::CertificationList& cert_by_reg_ref
            = cert_by_reg.in();
        BOOST_TEST_MESSAGE( "\nCertificationsByRegistrar" );
        for (CORBA::ULong i=0; i < cert_by_reg_ref.length(); ++i)
        {
            const Registry::Registrar::Certification::CertificationData& cd = cert_by_reg_ref[i];
            BOOST_TEST_MESSAGE(" certification id: " << cd.id << "\n"
                    << " score: " << cd.score << "\n"
                    << " file id: " << cd.evaluation_file_id << "\n"
                    << " from date: " << cd.fromDate.day << ". " << cd.fromDate.month << ". " << cd.fromDate.year << "\n"
                    << " to date: " << cd.toDate.day << ". " << cd.toDate.month << ". " << cd.toDate.year << "\n\n"
                    );
        }

        CorbaContainer::destroy_instance();

/*
    }//try
    catch(CORBA::TRANSIENT&)
    {
      cerr << "Caught system exception TRANSIENT -- unable to contact the "
           << "server." << endl;
    }
    catch(CORBA::SystemException& ex)
    {
      cerr << "Caught a CORBA::" << ex._name() << endl;
    }
    catch(CORBA::Exception& ex)
    {
      cerr << "Caught CORBA::Exception: " << ex._name() << endl;
    }
    catch(omniORB::fatalException& fe)
    {
      cerr << "Caught omniORB::fatalException:" << endl;
      cerr << "  file: " << fe.file() << endl;
      cerr << "  line: " << fe.line() << endl;
      cerr << "  mesg: " << fe.errmsg() << endl;
    }
*/

        model_file_id =  model_files.getId();
        BOOST_TEST_MESSAGE( " model_file_id: " << model_file_id );


}//test_registrar_certification_simple

//synchronization using barriers
struct sync_barriers
{
    boost::barrier cert_barrier;
    sync_barriers(std::size_t thread_number)
        : cert_barrier(thread_number)
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
class TestThreadWorker
{
public:

    TestThreadWorker(unsigned number,unsigned sleep_time
            , sync_barriers* sb_ptr, std::size_t thread_group_divisor
            , ThreadResultQueue* result_queue_ptr = 0, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , rdg_(seed)
            , tgd_(thread_group_divisor)
            , rsq_ptr (result_queue_ptr)
    {}

    void operator()()
    {
        ThreadResult res;
        res.number = number_;
        res.ret = 0;
        res.desc = std::string("ok");

        ::LibFred::TID cert_id=0;
        try
        {
            if(number_%tgd_)//if synchronized thread
            {
                if(sb_ptr_)
                    sb_ptr_->cert_barrier.wait();//wait for other synced threads
            }

            //try to create multiple registrar certifications in same time
            //created should by only one

            //get db connection
            Database::Connection conn = Database::Manager::acquire();

            std::string query1 (
                    "select * from registrar_certification "
                    "where registrar_id = 1  "
                    "and classification = 3 ");
            Database::Result res1 = conn.exec( query1 );

            DBSharedPtr nodb;
            ::LibFred::Registrar::Manager::AutoPtr regman(
                    ::LibFred::Registrar::Manager::create(nodb));

            cert_id = regman->createRegistrarCertification(
                    1
                    , Database::Date(
                            boost::posix_time::second_clock::local_time().date() //from now
                            )
                    , Database::Date(
                            date(boost::posix_time::second_clock::local_time().date()
                                  + boost::gregorian::date_duration( 365 ))
                                  )

                    , static_cast<::LibFred::Registrar::RegCertClass>(3)//score
                    , 0//evaluation_file_id
                    );

            Database::Result res2 = conn.exec( query1 );

            if((res2.size() - res1.size()) > 1 )
            {
                res.ret = 3;
                res.desc = std::string("create multiple registrar certifications in same time failed");
            }

        }
        catch(const std::exception&)
        {
            //ok
        }
        catch(...)
        {
            BOOST_TEST_MESSAGE("exception 4 in operator() thread number: " << number_ );
            res.ret = 4;
            res.desc = std::string("unknown exception");
            return;
        }

        //edit certification
        try
        {

            DBSharedPtr nodb;
            ::LibFred::Registrar::Manager::AutoPtr regman(
                    ::LibFred::Registrar::Manager::create(nodb));

            regman->shortenRegistrarCertification(cert_id
                    , Database::Date(
                            date(boost::posix_time::second_clock::local_time().date()
                                  + boost::gregorian::date_duration( 7 ))));

            regman->updateRegistrarCertification(cert_id
                    , static_cast<::LibFred::Registrar::RegCertClass>(2)//score
                    , 0//evaluation_file_id
                    );
        }
        catch(const std::exception& ex)
        {
            BOOST_TEST_MESSAGE( "exception 5 in operator() thread number: " << number_
                    << " reason: " << ex.what() );
            res.ret = 5;
            res.desc = std::string(ex.what());
            return;
        }
        catch(...)
        {
            BOOST_TEST_MESSAGE( "exception 6 in operator() thread number: " << number_ );
            res.ret = 6;
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
    std::size_t tgd_;//thread group divisor
    ThreadResultQueue* rsq_ptr; //result queue non-owning pointer
};//class TestThreadWorker

BOOST_AUTO_TEST_CASE( test_certification_threaded )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    std::size_t const thread_group_divisor = thread_args_ptr->thread_group_divisor;
    // int(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
    // - thread_number / thread_group_divisor) is number of synced threads

    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<TestThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    BOOST_TEST_MESSAGE("thread barriers:: "
            <<  (thread_number - (thread_number % thread_group_divisor ? 1 : 0)
                    - thread_number/thread_group_divisor)
            );

    //synchronization barriers instance
    sync_barriers sb(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
            - thread_number/thread_group_divisor);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(TestThreadWorker(i,3,&sb
                , thread_group_divisor, &result_queue));
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
                    << " description: " << thread_result.desc
                    );
        }
    }//for i
}
BOOST_AUTO_TEST_SUITE_END();
