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

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact_diff.h"
#include "src/contact_verification/public_request_contact_verification_impl.h"

#include "src/fredlib/public_request/public_request.h"
#include "src/fredlib/public_request/public_request_impl.h"
#include "src/fredlib/public_request/public_request_authinfo_impl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_contactverification_args.h"
//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "src/corba/mailer_manager.h"
#include "src/fredlib/contact_verification/contact.h"
#include "src/contact_verification/contact_verification_impl.h"
#include "src/fredlib/object_states.h"


BOOST_AUTO_TEST_SUITE(TestLockingTrigger)

const std::string server_name = "test-locking-trigger";

struct Case_locking_trigger_threaded_Fixture
{
    unsigned long long registrar_id;
    Fred::Contact::Verification::Contact fcvc;
    std::string registrar_handle;

    //init
    Case_locking_trigger_threaded_Fixture()
    :registrar_handle("REG-FRED_A")
    {
        //corba config
        FakedArgs fa = CfgArgs::instance()->fa;
        //conf pointers
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                    get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
                , ns_args_ptr->nameservice_host
                , ns_args_ptr->nameservice_port
                , ns_args_ptr->nameservice_context);

        unsigned long long request_id =0;
        {
            //get db connection
            Database::Connection conn = Database::Manager::acquire();

            //Database::Transaction trans(conn);

            //get registrar id
            Database::Result res_reg = conn.exec_params(
                    "SELECT id FROM registrar WHERE handle=$1::text",
                    Database::query_param_list(registrar_handle));
            if(res_reg.size() == 0) {
                throw std::runtime_error("Registrar does not exist");
            }

            registrar_id = res_reg[0][0];

            RandomDataGenerator rdg;

            //create test contact
            std::string xmark = rdg.xnumstring(6);
            fcvc.handle=std::string("TESTLT-HANDLE")+xmark;
            fcvc.name=std::string("TESTLT NAME")+xmark;
            fcvc.organization=std::string("TESTLT-ORG")+xmark;
            fcvc.street1=std::string("TESTLT-STR1")+xmark;
            fcvc.city=std::string("Praha");
            fcvc.postalcode=std::string("11150");
            fcvc.country=std::string("CZ");
            fcvc.telephone=std::string("+420.728")+xmark;
            fcvc.email=std::string("test")+xmark+"@nic.cz";
            fcvc.ssn=std::string("1980-01-01");
            fcvc.ssntype=std::string("BIRTHDAY");
            fcvc.auth_info=rdg.xnstring(8);
            //unsigned long long contact_hid =

            fcvc.disclosename = true;
            fcvc.discloseorganization = true;
            fcvc.discloseaddress = true;
            fcvc.disclosetelephone = true;
            fcvc.disclosefax = true;
            fcvc.discloseemail = true;
            fcvc.disclosevat = true;
            fcvc.discloseident = true;
            fcvc.disclosenotifyemail = true;

            Fred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);
            //trans.commit();
        }
    }

    //destroy
    virtual ~Case_locking_trigger_threaded_Fixture()
    {}
};

//simple threaded test

//synchronization using barriers
struct sync_barriers
{
    boost::barrier barrier1;
    boost::barrier barrier2;

    sync_barriers(std::size_t thread_number)
        : barrier1(thread_number)
        , barrier2(thread_number)
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
class LockingTriggerTestThreadWorker
{
public:

    LockingTriggerTestThreadWorker(unsigned number,unsigned sleep_time
            , sync_barriers* sb_ptr
            , Case_locking_trigger_threaded_Fixture* fixture_ptr
            , ThreadResultQueue* result_queue_ptr = 0, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , fixture_ptr_(fixture_ptr)
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
            //db connection with transaction
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            //std::cout << "waiting: " << number_ << std::endl;
            if(sb_ptr_) sb_ptr_->barrier1.wait();//wait for other synced threads
            //std::cout << "start: " << number_ << std::endl;

            //call some impl
            //BOOST_TEST_MESSAGE( "contact id: " << fixture_ptr_->fcvc.id );
            Fred::lock_object_state_request_lock(fixture_ptr_->fcvc.id);
            //if(sb_ptr_) sb_ptr_->barrier2.wait();//wait for other synced threads
            if(!Fred::object_has_state(fixture_ptr_->fcvc.id,
                Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT))
            {
                Fred::insert_object_state(fixture_ptr_->fcvc.id
                    , Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT );
                Fred::update_object_states(fixture_ptr_->fcvc.id);
                res.ret = 1;
            }

            tx.commit();
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
    Case_locking_trigger_threaded_Fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr; //result queue non-owning pointer
};//class LockingTriggerTestThreadWorker


BOOST_FIXTURE_TEST_CASE( test_locking_trigger_threaded, Case_locking_trigger_threaded_Fixture )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<LockingTriggerTestThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    //synchronization barriers instance
    sync_barriers sb(thread_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(LockingTriggerTestThreadWorker(i,3,&sb
            , dynamic_cast<Case_locking_trigger_threaded_Fixture*>(this)
            , &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE( "threads end result_queue.size(): " << result_queue.size() );

    unsigned long pass_counter = 0;
    for(unsigned i = 0; i < thread_number; ++i)
    {
        ThreadResult thread_result;
        if(!result_queue.try_pop(thread_result)) {
            continue;
        }

        BOOST_TEST_MESSAGE( "result.ret: " << thread_result.ret );

        if(thread_result.ret != 0)
        {
            if(thread_result.ret == 1)
            {
                ++pass_counter;
            }
            else
            {
                BOOST_FAIL( thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc);
            }
        }
    }//for i

    BOOST_CHECK(pass_counter == 1);
}


struct Locking_object_state_request_fixture
{
    Fred::OperationContext ctx;
    std::string registrar_handle;
    std::string xmark;
    std::string contact_handle;
    unsigned long long info_contact_id;
    Fred::InfoContactOutput info_contact;
    std::vector<Fred::InfoContactOutput> info_contact_history;

    //init
    Locking_object_state_request_fixture()
    : registrar_handle (static_cast<std::string>(ctx.get_conn().exec("SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]))
    , xmark(RandomDataGenerator().xnumstring(6))
    , contact_handle(std::string("TEST-OBJECT-STATE-REQUEST-CONTACT-HANDLE")+xmark)
    , info_contact_id(0)
    {
        //corba config
        FakedArgs fa = CfgArgs::instance()->fa;
        //conf pointers
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                    get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
                , ns_args_ptr->nameservice_host
                , ns_args_ptr->nameservice_port
                , ns_args_ptr->nameservice_context);

        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        Fred::CreateContact(contact_handle,registrar_handle)
            .set_name(std::string("TEST-OBJECT-STATE-REQUEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        info_contact_id = static_cast<unsigned long long>(ctx.get_conn().exec_params(
                "SELECT id FROM object_registry WHERE name=$1::text AND erdate IS NULL"
                , Database::query_param_list(contact_handle))[0][0]);

        BOOST_CHECK(info_contact_id);//expecting existing object
        info_contact = Fred::InfoContact(contact_handle, registrar_handle).exec(ctx);
        info_contact_history = Fred::InfoContactHistory(info_contact.info_contact_data.roid, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }

    //destroy
    virtual ~Locking_object_state_request_fixture()
    {
        BOOST_TEST_MESSAGE(contact_handle);
    }


};

//thread functor
class LockingObjectStateRequestThreadWorker
{
public:

    LockingObjectStateRequestThreadWorker(unsigned number,unsigned sleep_time
            , sync_barriers* sb_ptr
            , Locking_object_state_request_fixture* fixture_ptr
            , ThreadResultQueue* result_queue_ptr = 0, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , fixture_ptr_(fixture_ptr)
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
            //db connection with transaction
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            //std::cout << "waiting: " << number_ << std::endl;
            if(sb_ptr_) sb_ptr_->barrier1.wait();//wait for other synced threads
            //std::cout << "start: " << number_ << std::endl;

            //call some impl
            Fred::lock_object_state_request_lock(fixture_ptr_->info_contact_id);
            //if(sb_ptr_) sb_ptr_->barrier2.wait();//wait for other synced threads

            BOOST_TEST_MESSAGE( "states thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
            bool cic = Fred::object_has_state(fixture_ptr_->info_contact_id
                , Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);
            bool ic = Fred::object_has_state(fixture_ptr_->info_contact_id
                , Fred::ObjectState::IDENTIFIED_CONTACT);
            bool vc = Fred::object_has_state(fixture_ptr_->info_contact_id
                , Fred::ObjectState::VALIDATED_CONTACT);

            BOOST_TEST_MESSAGE( "cic: " << cic << " thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
            BOOST_TEST_MESSAGE( "ic: " << ic << " thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
            BOOST_TEST_MESSAGE( "vc: " << vc << " thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);

            if(!cic && !ic && !vc)
            {
                Fred::insert_object_state(fixture_ptr_->info_contact_id, Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT );
                BOOST_TEST_MESSAGE( "set CONDITIONALLY_IDENTIFIED_CONTACT thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
                res.ret = 1;
            }
            else if (cic && !ic && !vc)
            {
                Fred::insert_object_state(fixture_ptr_->info_contact_id, Fred::ObjectState::IDENTIFIED_CONTACT );
                BOOST_TEST_MESSAGE( "set IDENTIFIED_CONTACT thread: "<< number_ << " object_id: " << fixture_ptr_->info_contact_id);
                res.ret = 1;
            }
            else if (cic && ic && !vc)
            {
                Fred::insert_object_state(fixture_ptr_->info_contact_id, Fred::ObjectState::VALIDATED_CONTACT );
                BOOST_TEST_MESSAGE( "set VALIDATED_CONTACT thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
                res.ret = 1;
            }

            BOOST_TEST_MESSAGE( "sleep thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
            boost::this_thread::sleep( boost::posix_time::milliseconds(2000));
            BOOST_TEST_MESSAGE( "update states thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
            Fred::update_object_states(fixture_ptr_->info_contact_id);
            BOOST_TEST_MESSAGE( "commit thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
            boost::this_thread::sleep( boost::posix_time::milliseconds(2000));
            tx.commit();
            BOOST_TEST_MESSAGE( "end thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
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
    Locking_object_state_request_fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr; //result queue non-owning pointer
};//class LockingObjectStateRequestThreadWorker


BOOST_FIXTURE_TEST_CASE( test_locking_object_state_request_threaded, Locking_object_state_request_fixture )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<LockingObjectStateRequestThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    //synchronization barriers instance
    sync_barriers sb(thread_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(LockingObjectStateRequestThreadWorker(i,3,&sb
            , dynamic_cast<Locking_object_state_request_fixture*>(this)
            , &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE( "threads end result_queue.size(): " << result_queue.size() );

    unsigned long pass_counter = 0;
    for(unsigned i = 0; i < thread_number; ++i)
    {
        ThreadResult thread_result;
        if(!result_queue.try_pop(thread_result)) {
            continue;
        }

        BOOST_TEST_MESSAGE( "result.ret: " << thread_result.ret );

        if(thread_result.ret != 0)
        {
            if(thread_result.ret == 1)
            {
                ++pass_counter;
            }
            else
            {
                BOOST_FAIL( thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc);
            }
        }
    }//for i

    BOOST_TEST_MESSAGE( "pass_counter: " << pass_counter );
    BOOST_CHECK(pass_counter == 3);
}

//lock_public_request

struct Locking_public_request_fixture
{
    Fred::OperationContext ctx;
    std::string registrar_handle;
    unsigned long long registrar_id;
    std::string xmark;
    std::string contact_handle;
    unsigned long long contact_id;
    Fred::InfoContactOutput info_contact;
    std::vector<Fred::InfoContactOutput> info_contact_history;
    std::auto_ptr<Fred::Manager> registry_manager;
    std::auto_ptr<Fred::PublicRequest::Manager> request_manager;

    //init
    Locking_public_request_fixture()
    : registrar_handle (static_cast<std::string>(ctx.get_conn().exec("SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]))
    , registrar_id(static_cast<unsigned long long>(ctx.get_conn().exec_params("SELECT id FROM registrar WHERE handle = $1::text"
            , Database::query_param_list(registrar_handle))[0][0]))
    , xmark(RandomDataGenerator().xnumstring(6))
    , contact_handle(std::string("TEST-PUBLIC-REQUEST-CONTACT-HANDLE")+xmark)
    , contact_id(0)
    {
        //corba config
        FakedArgs fa = CfgArgs::instance()->fa;
        //conf pointers
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                    get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
                , ns_args_ptr->nameservice_host
                , ns_args_ptr->nameservice_port
                , ns_args_ptr->nameservice_context);

        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        Fred::CreateContact(contact_handle,registrar_handle)
            .set_name(std::string("TEST-PUBLIC-REQUEST-CONTACT NAME")+xmark)
            .set_authinfo("testauthinfo")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .set_email("test@nic.cz")
            .set_notifyemail("test-notify@nic.cz")
            .exec(ctx);

        contact_id = static_cast<unsigned long long>(ctx.get_conn().exec_params(
                "SELECT id FROM object_registry WHERE name=$1::text AND erdate IS NULL"
                , Database::query_param_list(contact_handle))[0][0]);

        BOOST_CHECK(contact_id);//expecting existing object
        info_contact = Fred::InfoContact(contact_handle, registrar_handle).exec(ctx);
        info_contact_history = Fred::InfoContactHistory(info_contact.info_contact_data.roid, registrar_handle).exec(ctx);

        ctx.commit_transaction();

        //create request
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction trans(conn);


        HandleRegistryArgs *rconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        DBSharedPtr nodb;
        boost::shared_ptr<Fred::Mailer::Manager> mailer_manager( new MailerManager(CorbaContainer::get_instance()->getNS()));;

        std::auto_ptr<Fred::Document::Manager> doc_manager;



        registry_manager.reset(Fred::Manager::create(
                    nodb,
                    rconf->restricted_handles));

        doc_manager = Fred::Document::Manager::create(
                    rconf->docgen_path,
                    rconf->docgen_template_path,
                    rconf->fileclient_path,
                    //doc_manager config dependence
                    CfgArgs::instance()->get_handler_ptr_by_type<
                        HandleCorbaNameServiceArgs>()
                            ->get_nameservice_host_port());

        request_manager.reset(Fred::PublicRequest::Manager::create(
                    registry_manager->getDomainManager(),
                    registry_manager->getContactManager(),
                    registry_manager->getNSSetManager(),
                    registry_manager->getKeySetManager(),
                    mailer_manager.get(),
                    doc_manager.get(),
                    registry_manager->getMessageManager()));

        Fred::PublicRequest::Type type = Fred::PublicRequest::PRT_AUTHINFO_POST_PIF;

        std::auto_ptr<Fred::PublicRequest::PublicRequest> new_request(
                request_manager->createRequest(type));
        new_request->setRegistrarId(registrar_id);
        new_request->setRequestId(0);
        new_request->addObject(
            Fred::PublicRequest::OID(contact_id, contact_handle, Fred::PublicRequest::OT_CONTACT));
        new_request->save();

        trans.commit();
    }

    //destroy
    virtual ~Locking_public_request_fixture()
    {
        BOOST_TEST_MESSAGE(contact_handle);
    }


};

//thread functor
class LockingPublicRequestThreadWorker
{
public:

    LockingPublicRequestThreadWorker(unsigned number,unsigned sleep_time
            , sync_barriers* sb_ptr
            , Locking_public_request_fixture* fixture_ptr
            , ThreadResultQueue* result_queue_ptr = 0, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , fixture_ptr_(fixture_ptr)
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
            //db connection with transaction
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            //std::cout << "waiting: " << number_ << std::endl;
            if(sb_ptr_) sb_ptr_->barrier1.wait();//wait for other synced threads
            //std::cout << "start: " << number_ << std::endl;

            //call some impl
            Fred::PublicRequest::lock_public_request_by_object(fixture_ptr_->contact_id);
            //if(sb_ptr_) sb_ptr_->barrier2.wait();//wait for other synced threads

            BOOST_TEST_MESSAGE( "start thread: " << number_ << " object_id: " << fixture_ptr_->contact_id);
            boost::this_thread::sleep( boost::posix_time::milliseconds(2000));

            /* check if object has given request type already active */
            unsigned long long p_req_id = Fred::PublicRequest::check_public_request(
                    fixture_ptr_->contact_id
                    , Fred::PublicRequest::PRT_AUTHINFO_EMAIL_PIF);


            fixture_ptr_->request_manager->processRequest(p_req_id,false,false);



            tx.commit();
            BOOST_TEST_MESSAGE( "end p_req_id: " << p_req_id << " thread: " << number_ << " object_id: " << fixture_ptr_->contact_id);
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
    Locking_public_request_fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr; //result queue non-owning pointer
};//class LockingPublicRequestThreadWorker


BOOST_FIXTURE_TEST_CASE( test_locking_public_request_threaded, Locking_public_request_fixture )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<LockingPublicRequestThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    //synchronization barriers instance
    sync_barriers sb(thread_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(LockingPublicRequestThreadWorker(i,3,&sb
            , dynamic_cast<Locking_public_request_fixture*>(this)
            , &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE( "threads end result_queue.size(): " << result_queue.size() );

    unsigned long pass_counter = 0;
    for(unsigned i = 0; i < thread_number; ++i)
    {
        ThreadResult thread_result;
        if(!result_queue.try_pop(thread_result)) {
            continue;
        }

        BOOST_TEST_MESSAGE( "result.ret: " << thread_result.ret );

        if(thread_result.ret != 0)
        {
            if(thread_result.ret == 1)
            {
                ++pass_counter;
            }
            else
            {
                BOOST_FAIL( thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc);
            }
        }
    }//for i

    BOOST_TEST_MESSAGE( "pass_counter: " << pass_counter );
    BOOST_CHECK(pass_counter == 1);
}

BOOST_AUTO_TEST_SUITE_END();//TestLockingTrigger
