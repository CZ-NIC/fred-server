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

#include "test/deprecated/test_registrar_group.hh"

BOOST_AUTO_TEST_SUITE(TestRegistrarGroup)

ccReg::TID gid3 = 0;

BOOST_AUTO_TEST_CASE( test_registrar_group_simple )
{
    //  try
    //  {

        //CORBA init
        FakedArgs fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

        //std::cout << "ccReg::Admin::_narrow" << std::endl;
        ccReg::Admin_var admin_ref;
        admin_ref = ccReg::Admin::_narrow(CorbaContainer::get_instance()->nsresolve("Admin"));

        //get db connection
        Database::Connection conn = Database::Manager::acquire();


        //deletion of test data

        std::string query3 (
                "delete from registrar_group_map "
                "where registrar_group_map.registrar_group_id "
                "in (select id from registrar_group "
                " where registrar_group.short_name "
                "in ('testgroup1', 'testgroup2', 'testgroup3', 'testgroup4'"
                ", 'testgroup5'))"
                );
        conn.exec( query3 );

        std::string query1 = str(boost::format(
                "delete from registrar_group where short_name = '%1%'"
                " or short_name = '%2%' or short_name = '%3%' or short_name = '%4%' ")
                % "testgroup1" % "testgroup2" % "testgroup3" % "testgroup4");
        conn.exec( query1 );


        //group simple test
        BOOST_TEST_MESSAGE( "admin_ref->getGroupManager()" );
        Registry::Registrar::Group::Manager_var group_manager_ref;
        group_manager_ref= admin_ref->getGroupManager();
        ccReg::TID
        gid1 = group_manager_ref->createGroup("testgroup1");
        ccReg::TID gid2 =
                group_manager_ref->createGroup("testgroup2");
        //ccReg::TID
        gid3 = group_manager_ref->createGroup("testgroup3");
        group_manager_ref->deleteGroup(gid2);

        std::string query4 ("select short_name, cancelled from registrar_group "
                " where short_name = 'testgroup2' and cancelled is not null");
        Database::Result res = conn.exec( query4 );
        BOOST_REQUIRE_EQUAL(res.size() , 1);

        //membership simple test
        //ccReg::TID mid1 =
                group_manager_ref->addRegistrarToGroup(1,gid1);
        //ccReg::TID mid2 =
                group_manager_ref->addRegistrarToGroup(2,gid1);

        std::string query2 ("select * from registrar_group_map "
            "join registrar_group "
            "on registrar_group_map.registrar_group_id = registrar_group.id "
            " where registrar_group.short_name = 'testgroup1'");
        Database::Result res2 = conn.exec( query2 );
        BOOST_REQUIRE_EQUAL(res2.size() , 2);

        group_manager_ref->updateGroup(gid3, "testgroup4");
        std::string query5 ("select * from registrar_group "
            " where registrar_group.short_name = 'testgroup4'");
        Database::Result res5 = conn.exec( query5 );
        BOOST_REQUIRE_EQUAL(3*res5.size() , 3);

        group_manager_ref->removeRegistrarFromGroup(1, gid1);
        std::string query6 ("select * from registrar_group_map "
            "join registrar_group "
            "on registrar_group_map.registrar_group_id = registrar_group.id "
            " where registrar_group.short_name = 'testgroup1' "
            " and registrar_group_map.member_until is null "    );
        Database::Result res6 = conn.exec( query6 );
        BOOST_REQUIRE_EQUAL(4*res6.size() , 4);

        group_manager_ref->addRegistrarToGroup(1,gid1);
        std::string query7 ("select * from registrar_group_map "
            "join registrar_group "
            "on registrar_group_map.registrar_group_id = registrar_group.id "
            " where registrar_group.short_name = 'testgroup1' "
            " and registrar_group_map.member_until is null "    );
        Database::Result res7 = conn.exec( query7 );
        BOOST_REQUIRE_EQUAL(2*res7.size() +1 , 5);

        //unbounded struct sequence client side mapping
        //create owning seq
        Registry::Registrar::Group::GroupList_var
            gr_list (new Registry::Registrar::Group::GroupList);

        //read into seq
        gr_list =  group_manager_ref->getGroups();

        //iterate over seq using reference and read structs
        const Registry::Registrar::Group::GroupList&  gr_list_ref
            = gr_list.in();
        BOOST_TEST_MESSAGE("\nGroupList" );
        for (CORBA::ULong i=0; i < gr_list_ref.length(); ++i)
        {
            const Registry::Registrar::Group::GroupData& gd = gr_list_ref[i];
            BOOST_TEST_MESSAGE("group name: " << gd.name );
        }

        Registry::Registrar::Group::MembershipByGroupList_var
            mem_by_grp(new Registry::Registrar::Group::MembershipByGroupList);
        mem_by_grp = group_manager_ref->getMembershipsByGroup(gid1);

        /*print
        const Registry::Registrar::Group::MembershipByGroupList& mem_by_grp_ref
            = mem_by_grp.in();
        std::cout << "\nMembershipByGroup" << std::endl;
        for (CORBA::ULong i=0; i < mem_by_grp_ref.length(); ++i)
        {
            const Registry::Registrar::Group::MembershipByGroup& mbg = mem_by_grp_ref[i];
            std::cout << " membership id: " << mbg.id << "\n"
                    << " registrar_id: " << mbg.registrar_id << "\n"
                    << " from date: " << mbg.fromDate.day << ". " << mbg.fromDate.month << ". " << mbg.fromDate.year << "\n"
                    << " to date: " << mbg.toDate.day << ". " << mbg.toDate.month << ". " << mbg.toDate.year << "\n"
                    << std::endl;
        }
        */

        Registry::Registrar::Group::MembershipByRegistrarList_var
            mem_by_reg (new Registry::Registrar::Group::MembershipByRegistrarList);
        mem_by_reg = group_manager_ref->getMembershipsByRegistar(1);

        /*print
        const Registry::Registrar::Group::MembershipByRegistrarList& mem_by_reg_ref
            = mem_by_reg.in();
        std::cout << "\nMembershipByRegistrar" << std::endl;
        for (CORBA::ULong i=0; i < mem_by_reg_ref.length(); ++i)
        {
            const Registry::Registrar::Group::MembershipByRegistrar& mbr = mem_by_reg_ref[i];
            std::cout << " membership id: " << mbr.id << "\n"
                    << " group_id: " << mbr.group_id << "\n"
                    << " from date: " << mbr.fromDate.day << ". " << mbr.fromDate.month << ". " << mbr.fromDate.year << "\n"
                    << " to date: " << mbr.toDate.day << ". " << mbr.toDate.month << ". " << mbr.toDate.year << "\n"
                    << std::endl;
        }
         */

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



}//test_registrar_group_simple

//synchronization using barriers
struct sync_barriers
{
    boost::barrier group_barrier;

    sync_barriers(std::size_t thread_number)
        : group_barrier(thread_number)
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

        try
        {
            if(number_%tgd_)//if synchronized thread
            {
                //std::cout << "waiting: " << number_ << std::endl;
                if(sb_ptr_)
                    sb_ptr_->group_barrier.wait();//wait for other synced threads
            }
            else
            {//non-synchronized thread
                //std::cout << "NOwaiting: " << number_ << std::endl;
            }
            //std::cout << "start: " << number_ << std::endl;

            DBSharedPtr nodb;
            ::LibFred::Registrar::Manager::AutoPtr regman(
                    ::LibFred::Registrar::Manager::create(nodb));
            ///create membership of registrar in group
            regman->createRegistrarGroupMembership(
                    1
                    , gid3
                    , Database::Date(Database::NOW)
                    , Database::Date(Database::POS_INF));
        }
        catch(const std::exception& ex)
        {
            BOOST_TEST_MESSAGE("exception 1 in operator() thread number: " << number_
                    << " reason: " << ex.what() );
            res.ret = 1;
            res.desc = std::string(ex.what());
            return;
        }
        catch(...)
        {
            BOOST_TEST_MESSAGE("exception 2 in operator() thread number: " << number_ );
            res.ret = 2;
            res.desc = std::string("unknown exception");
            return;
        }

        try
        {
            DBSharedPtr nodb;
            ::LibFred::Registrar::Manager::AutoPtr regman(
                    ::LibFred::Registrar::Manager::create(nodb));
            //delete group
            regman->cancelRegistrarGroup(gid3);
        }
        catch(const std::exception& )
        {
            //ok
        }
        catch(...)
        {
            BOOST_TEST_MESSAGE("exception 7 in operator() thread number: " << number_ );
            res.ret = 7;
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

BOOST_AUTO_TEST_CASE( test_group_threaded )
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

    BOOST_TEST_MESSAGE( "thread barriers:: "
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
                    << " description: " << thread_result.desc);
        }
    }//for i
}

BOOST_AUTO_TEST_SUITE_END();
