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

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/setup_server_decl.hh"
#include "src/util/time_clock.hh"
#include "src/util/random_data_generator.hh"
#include "src/util/concurrent_queue.hh"

#include "src/libfred/registry.hh"
#include "src/libfred/registrable_object/contact/create_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact_output.hh"
#include "src/libfred/registrable_object/contact/info_contact_diff.hh"
#include "src/backend/contact_verification/public_request_contact_verification_impl.hh"

#include "src/libfred/public_request/public_request.hh"
#include "src/libfred/public_request/public_request_impl.hh"
#include "src/libfred/public_request/public_request_authinfo_impl.hh"
#include "src/libfred/public_request/create_public_request.hh"

#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/config_handler_decl.hh"

#include "src/libfred/contact_verification/contact.hh"
#include "src/backend/contact_verification/contact_verification_impl.hh"
#include "src/libfred/object_states.hh"
#include "src/bin/corba/mailer_manager.hh"

#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include <utility>
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

BOOST_AUTO_TEST_SUITE(TestLockingTrigger)

const std::string server_name = "test-locking-trigger";

struct Case_locking_trigger_threaded_Fixture
{
    Case_locking_trigger_threaded_Fixture()
        : registrar_handle("REG-FRED_A")
    {
        //corba config
        FakedArgs fa = CfgArgs::instance()->fa;
        //conf pointers
        const HandleCorbaNameServiceArgs* const ns_args_ptr =
                CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(
                fa.get_argc(),
                fa.get_argv(),
                ns_args_ptr->nameservice_host,
                ns_args_ptr->nameservice_port,
                ns_args_ptr->nameservice_context);

        unsigned long long request_id = 0;
        {
            //get db connection
            Database::Connection conn = Database::Manager::acquire();

            //Database::Transaction trans(conn);

            //get registrar id
            const Database::Result res_reg = conn.exec_params(
                    "SELECT id FROM registrar WHERE handle=$1::text",
                    Database::query_param_list(registrar_handle));
            if (res_reg.size() == 0)
            {
                throw std::runtime_error("Registrar does not exist");
            }

            registrar_id = static_cast<unsigned long long>(res_reg[0][0]);

            RandomDataGenerator rdg;

            //create test contact
            const std::string xmark = rdg.xnumstring(6);
            fcvc.handle = "TESTLT-HANDLE" + xmark;
            fcvc.name = "TESTLT NAME" + xmark;
            fcvc.organization = "TESTLT-ORG" + xmark;
            fcvc.street1 = "TESTLT-STR1" + xmark;
            fcvc.city = "Praha";
            fcvc.postalcode = "11150";
            fcvc.country = "CZ";
            fcvc.telephone = "+420.728" + xmark;
            fcvc.email = "test" + xmark + "@nic.cz";
            fcvc.ssn = "1980-01-01";
            fcvc.ssntype = "BIRTHDAY";
            fcvc.auth_info = rdg.xnstring(8);
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

            ::LibFred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);
            //trans.commit();
        }
    }

    unsigned long long registrar_id;
    ::LibFred::Contact::Verification::Contact fcvc;
    std::string registrar_handle;
};

//simple threaded test

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
class LockingTriggerTestThreadWorker
{
public:
    LockingTriggerTestThreadWorker(
            unsigned number,
            unsigned sleep_time,
            sync_barriers* sb_ptr,
            Case_locking_trigger_threaded_Fixture* fixture_ptr,
            ThreadResultQueue* result_queue_ptr = nullptr,
            unsigned seed = 0)
        : number_(number),
          sleep_time_(sleep_time),
          sb_ptr_(sb_ptr),
          fixture_ptr_(fixture_ptr),
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
            //db connection with transaction
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            //std::cout << "waiting: " << number_ << std::endl;
            if (sb_ptr_ != nullptr)
            {
                sb_ptr_->barrier.wait();//wait for other synced threads
                ++number_of_entered_barriers;
            }
            //std::cout << "start: " << number_ << std::endl;

            //call some impl
            //BOOST_TEST_MESSAGE( "contact id: " << fixture_ptr_->fcvc.id );
            ::LibFred::lock_object_state_request_lock(fixture_ptr_->fcvc.id);
            if (!LibFred::object_has_state(fixture_ptr_->fcvc.id,
                                           ::LibFred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT))
            {
                ::LibFred::insert_object_state(fixture_ptr_->fcvc.id,
                                               ::LibFred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);
                ::LibFred::update_object_states(fixture_ptr_->fcvc.id);
                res.ret = 1;
            }

            tx.commit();
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
    Case_locking_trigger_threaded_Fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr_; //result queue non-owning pointer
};

BOOST_FIXTURE_TEST_CASE(test_locking_trigger_threaded, Case_locking_trigger_threaded_Fixture)
{
    const HandleThreadGroupArgs* const thread_args_ptr = CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    const std::size_t number_of_threads = thread_args_ptr->number_of_threads;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<LockingTriggerTestThreadWorker> tw_vector;
    tw_vector.reserve(number_of_threads);

    //synchronization barriers instance
    sync_barriers sb(number_of_threads);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        tw_vector.push_back(LockingTriggerTestThreadWorker(
                i,
                3,
                &sb,
                dynamic_cast<Case_locking_trigger_threaded_Fixture*>(this),
                &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " << result_queue.unguarded_access().size());

    unsigned long pass_counter = 0;
    while (!result_queue.unguarded_access().empty())
    {
        const auto thread_result = result_queue.unguarded_access().pop();

        BOOST_TEST_MESSAGE("result.ret: " << thread_result.ret);

        if (thread_result.ret != 0)
        {
            if (thread_result.ret == 1)
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
    }

    BOOST_CHECK_EQUAL(pass_counter, 1);
}

struct Locking_object_state_request_fixture
{
    ::LibFred::OperationContextCreator ctx;
    std::string registrar_handle;
    std::string xmark;
    std::string contact_handle;
    unsigned long long info_contact_id;
    ::LibFred::InfoContactOutput info_contact;
    std::vector<::LibFred::InfoContactOutput> info_contact_history;

    Locking_object_state_request_fixture()
        : registrar_handle(static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle "
                "FROM registrar "
                "WHERE system "
                "ORDER BY id LIMIT 1")[0][0])),
          xmark(RandomDataGenerator().xnumstring(6)),
          contact_handle("TEST-OBJECT-STATE-REQUEST-CONTACT-HANDLE" + xmark),
          info_contact_id(0)
    {
        //corba config
        FakedArgs fa = CfgArgs::instance()->fa;
        //conf pointers
        const HandleCorbaNameServiceArgs* const ns_args_ptr =
                CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(
                fa.get_argc(),
                fa.get_argv(),
                ns_args_ptr->nameservice_host,
                ns_args_ptr->nameservice_port,
                ns_args_ptr->nameservice_context);

        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = "STR1" + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        ::LibFred::CreateContact(contact_handle,registrar_handle)
            .set_name("TEST-OBJECT-STATE-REQUEST-CONTACT NAME" + xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        info_contact_id = static_cast<unsigned long long>(ctx.get_conn().exec_params(
                "SELECT id "
                "FROM object_registry "
                "WHERE name=UPPER($1::text) AND erdate IS NULL",
                Database::query_param_list(contact_handle))[0][0]);

        BOOST_CHECK(info_contact_id != 0);//expecting existing object
        info_contact = ::LibFred::InfoContactByHandle(contact_handle).exec(ctx);
        info_contact_history = ::LibFred::InfoContactHistoryByRoid(info_contact.info_contact_data.roid).exec(ctx);
        ctx.commit_transaction();
    }

    ~Locking_object_state_request_fixture()
    {
        BOOST_TEST_MESSAGE(contact_handle);
    }
};

//thread functor
class LockingObjectStateRequestThreadWorker
{
public:
    LockingObjectStateRequestThreadWorker(
            unsigned number,
            unsigned sleep_time,
            sync_barriers* sb_ptr,
            Locking_object_state_request_fixture* fixture_ptr,
            ThreadResultQueue* result_queue_ptr = nullptr,
            unsigned seed = 0)
        : number_(number),
          sleep_time_(sleep_time),
          sb_ptr_(sb_ptr),
          fixture_ptr_(fixture_ptr),
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
            //db connection with transaction
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            //std::cout << "waiting: " << number_ << std::endl;
            if (sb_ptr_ != nullptr)
            {
                sb_ptr_->barrier.wait();//wait for other synced threads
                ++number_of_entered_barriers;
            }
            //std::cout << "start: " << number_ << std::endl;

            //call some impl
            ::LibFred::lock_object_state_request_lock(fixture_ptr_->info_contact_id);

            //BOOST_TEST_MESSAGE("states thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);

            if (!LibFred::object_has_state(
                    fixture_ptr_->info_contact_id,
                    ::LibFred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT))
            {
                ::LibFred::insert_object_state(
                        fixture_ptr_->info_contact_id,
                        ::LibFred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);
                ::LibFred::update_object_states(fixture_ptr_->info_contact_id);//update will unset state if by other thred set in the future according to start of the transaction timestamp
                //BOOST_TEST_MESSAGE("set CONDITIONALLY_IDENTIFIED_CONTACT thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
                res.ret = 1;
            }
            //BOOST_TEST_MESSAGE("commit thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
            //boost::this_thread::sleep( boost::posix_time::milliseconds(2000));
            tx.commit();
            //BOOST_TEST_MESSAGE("end thread: " << number_ << " object_id: " << fixture_ptr_->info_contact_id);
        }
        catch (const std::exception& e)
        {
            //BOOST_TEST_MESSAGE("exception 1 in operator() thread number: " << number_ << " reason: " << e.what() );
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
    //need only default constructible members here
    unsigned number_;//thread identification
    unsigned sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    Locking_object_state_request_fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr_; //result queue non-owning pointer
};


BOOST_FIXTURE_TEST_CASE(test_locking_object_state_request_threaded, Locking_object_state_request_fixture)
{
    const HandleThreadGroupArgs* const thread_args_ptr =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleThreadGroupArgs>();

    const std::size_t number_of_threads = thread_args_ptr->number_of_threads;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<LockingObjectStateRequestThreadWorker> tw_vector;
    tw_vector.reserve(number_of_threads);

    //synchronization barriers instance
    sync_barriers sb(number_of_threads);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        tw_vector.push_back(LockingObjectStateRequestThreadWorker(
                i,
                3,
                &sb,
                dynamic_cast<Locking_object_state_request_fixture*>(this),
                &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " << result_queue.unguarded_access().size());

    unsigned long pass_counter = 0;
    while (!result_queue.unguarded_access().empty())
    {
        const auto thread_result = result_queue.unguarded_access().pop();

        BOOST_TEST_MESSAGE("result.ret: " << thread_result.ret);

        if (thread_result.ret != 0)
        {
            if (thread_result.ret == 1)
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
    }

    BOOST_CHECK_EQUAL(pass_counter, 1);
}

namespace PublicRequestType = ::Fred::Backend::PublicRequest::Type;

class Locking_public_request_fixture
{
public:
    Locking_public_request_fixture()
        : registrar_handle(static_cast<std::string>(ctx_.get_conn().exec(
                "SELECT handle "
                "FROM registrar "
                "WHERE system "
                "ORDER BY id LIMIT 1")[0][0])),
          registrar_id(static_cast<unsigned long long>(ctx_.get_conn().exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle))[0][0])),
          xmark(RandomDataGenerator().xnumstring(6)),
          contact_handle("TEST-PUBLIC-REQUEST-CONTACT-HANDLE" + xmark),
          contact_id(0),
          preq_id(0)
    {
        //corba config
        FakedArgs fa = CfgArgs::instance()->fa;
        //conf pointers
        const HandleCorbaNameServiceArgs* const ns_args_ptr =
                CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(
                fa.get_argc(),
                fa.get_argv(),
                ns_args_ptr->nameservice_host,
                ns_args_ptr->nameservice_port,
                ns_args_ptr->nameservice_context);

        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = "STR1" + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";

        ::LibFred::CreateContact(contact_handle, registrar_handle)
            .set_name("TEST-PUBLIC-REQUEST-CONTACT NAME" + xmark)
            .set_authinfo("testauthinfo")
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .set_email("test@nic.cz")
            .set_notifyemail("test-notify@nic.cz")
            .exec(ctx_);

        contact_id = static_cast<unsigned long long>(ctx_.get_conn().exec_params(
                "SELECT id FROM object_registry WHERE name=UPPER($1::text) AND erdate IS NULL",
                Database::query_param_list(contact_handle))[0][0]);

        BOOST_CHECK(contact_id != 0);//expecting existing object
        const auto info_contact = ::LibFred::InfoContactByHandle(contact_handle).exec(ctx_);
        ::LibFred::InfoContactHistoryByRoid(info_contact.info_contact_data.roid).exec(ctx_);

        ctx_.commit_transaction();

        const HandleRegistryArgs* const rconf =
                    CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        DBSharedPtr nodb;

        registry_manager.reset(::LibFred::Manager::create(nodb, rconf->restricted_handles));

        doc_manager = ::LibFred::Document::Manager::create(
                rconf->docgen_path,
                rconf->docgen_template_path,
                rconf->fileclient_path,
                //doc_manager config dependence
                CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()->get_nameservice_host_port());

        mailer_manager = std::make_shared<MailerManager>(CorbaContainer::get_instance()->getNS());

        request_manager.reset(
                ::LibFred::PublicRequest::Manager::create(
                        registry_manager->getDomainManager(),
                        registry_manager->getContactManager(),
                        registry_manager->getNssetManager(),
                        registry_manager->getKeysetManager(),
                        mailer_manager.get(),
                        doc_manager.get(),
                        registry_manager->getMessageManager()));

        //create request
        {
            ::LibFred::OperationContextCreator ctx;
            ::LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, contact_id);
            ::LibFred::CreatePublicRequest create_public_request;
            create_public_request.set_reason("reason");
            create_public_request.set_email_to_answer("email_to_answer@nic.cz");
            create_public_request.set_registrar_id(registrar_id);
            try
            {
                preq_id = create_public_request.exec(
                        locked_contact,
                        PublicRequestType::get_iface_of<PublicRequestType::AuthinfoEmail>());
            }
            catch (const std::exception& e)
            {
                BOOST_TEST_MESSAGE(e.what());
                throw;
            }
            ctx.commit_transaction();
        }
    }

    ~Locking_public_request_fixture()
    {
        BOOST_TEST_MESSAGE(contact_handle);
    }
private:
    ::LibFred::OperationContextCreator ctx_;
    std::string registrar_handle;
    unsigned long long registrar_id;
    std::string xmark;
    std::string contact_handle;
    std::unique_ptr<::LibFred::Manager> registry_manager;
    std::unique_ptr<::LibFred::Document::Manager> doc_manager;
    std::shared_ptr<::LibFred::Mailer::Manager> mailer_manager;
public:
    std::unique_ptr<::LibFred::PublicRequest::Manager> request_manager;
    unsigned long long contact_id;
    unsigned long long preq_id;
};

//thread functor
class LockingPublicRequestThreadWorker
{
public:
    LockingPublicRequestThreadWorker(
            unsigned number,
            unsigned sleep_time,
            sync_barriers* sb_ptr,
            Locking_public_request_fixture* fixture_ptr,
            ThreadResultQueue* result_queue_ptr = nullptr,
            unsigned seed = 0)
        : number_(number),
          sleep_time_(sleep_time),
          sb_ptr_(sb_ptr),
          fixture_ptr_(fixture_ptr),
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
            //db connection with transaction
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            //std::cout << "waiting: " << number_ << std::endl;
            if (sb_ptr_ != nullptr)
            {
                sb_ptr_->barrier.wait();//wait for other synced threads
                ++number_of_entered_barriers;
            }
            //std::cout << "start: " << number_ << std::endl;
            //Fred::PublicRequest::lock_public_request_by_object(fixture_ptr_->contact_id);
            //BOOST_TEST_MESSAGE( "start thread: " << number_ << " object_id: " << fixture_ptr_->contact_id);
            //boost::this_thread::sleep( boost::posix_time::milliseconds(2000));

            /* check if object has given request type already active */
            const unsigned long long p_req_id =
                    ::LibFred::PublicRequest::check_public_request(
                            fixture_ptr_->contact_id,
                            ::LibFred::PublicRequest::PRT_AUTHINFO_EMAIL_PIF);

            if (fixture_ptr_->preq_id == p_req_id)//if not processed, process request
            {
                fixture_ptr_->request_manager->processRequest(fixture_ptr_->preq_id, false, true);
                res.ret = 1;
            }

            tx.commit();
            //BOOST_TEST_MESSAGE( "fixture_ptr_->preq_id: " << fixture_ptr_->preq_id << " thread: " << number_ << " object_id: " << fixture_ptr_->contact_id);
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
    Locking_public_request_fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr_; //result queue non-owning pointer
};


BOOST_FIXTURE_TEST_CASE(test_locking_public_request_threaded, Locking_public_request_fixture)
{
    const HandleThreadGroupArgs* const thread_args_ptr =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleThreadGroupArgs>();

    const std::size_t number_of_threads = thread_args_ptr->number_of_threads;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<LockingPublicRequestThreadWorker> tw_vector;
    tw_vector.reserve(number_of_threads);

    //synchronization barriers instance
    sync_barriers sb(number_of_threads);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        tw_vector.push_back(LockingPublicRequestThreadWorker(
                i,
                3,
                &sb,
                dynamic_cast<Locking_public_request_fixture*>(this),
                &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " << result_queue.unguarded_access().size());

    unsigned long pass_counter = 0;
    while (!result_queue.unguarded_access().empty())
    {
        const auto thread_result = result_queue.unguarded_access().pop();

        BOOST_TEST_MESSAGE("result.ret: " << thread_result.ret);

        if (thread_result.ret != 0)
        {
            if (thread_result.ret == 1)
            {
                ++pass_counter;
            }
            else
            {
                BOOST_FAIL(thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc);
            }
        }
    }

    BOOST_TEST_MESSAGE("pass_counter: " << pass_counter);
    BOOST_CHECK_EQUAL(pass_counter, 1);
}

BOOST_AUTO_TEST_SUITE_END()//TestLockingTrigger
