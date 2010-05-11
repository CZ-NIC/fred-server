/*
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE Test registrar certification group

#include "config_handler.h"

//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleThreadGroupArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs));

#include "test_custom_main.h"

#include "corba_wrapper.h"

#include "random_data_generator.h"
#include "concurrent_queue.h"
#include "common.h"

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
  using ::time;
}
#endif

#include "test-registrar-certification-group.h"

BOOST_AUTO_TEST_CASE( test_registrar_certification_group_simple )
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

        std::cout << "ccReg::Admin::_narrow" << std::endl;
        ccReg::Admin_var admin_ref;
        admin_ref = ccReg::Admin::_narrow(CorbaContainer::get_instance()->nsresolve("Admin"));

        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //deletion of test data
        std::string query9 (
                "delete from registrar_certification "
                "where registrar_id = 1 ");
        conn.exec( query9 );

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
        std::cout << "admin_ref->getGroupManager()" << std::endl;
        Registry::Registrar::Group::Manager_var group_manager_ref;
        group_manager_ref= admin_ref->getGroupManager();
        ccReg::TID gid1 =
                group_manager_ref->createGroup("testgroup1");
        ccReg::TID gid2 =
                group_manager_ref->createGroup("testgroup2");
        ccReg::TID gid3 =
                group_manager_ref->createGroup("testgroup3");
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

        std::cout << "admin_ref->getCertificationManager()" << std::endl;
        Registry::Registrar::Certification::Manager_var cert_manager_ref;
        cert_manager_ref = admin_ref->getCertificationManager();

        ccReg::TID cid1 =
                cert_manager_ref->createCertification(1
                , makeCorbaDate(boost::gregorian::date(2010, 1, 30))
                ,makeCorbaDate(boost::gregorian::date(2011, 1, 30)),3,0);
        std::string query8 (
                "select * from registrar_certification "
                "where registrar_id = 1  "
                "and valid_from = to_date('2010-01-30','YYYY-MM-DD') "
                "and valid_until = to_date('2011-01-30','YYYY-MM-DD') "
                "and classification = 3 ");
        Database::Result res8 = conn.exec( query8 );
        BOOST_REQUIRE_EQUAL(6*res8.size() , 6);

        cert_manager_ref->updateCertification(cid1,4,0);
        std::string query10 (
                "select * from registrar_certification "
                "where registrar_id = 1  "
                "and valid_from = to_date('2010-01-30','YYYY-MM-DD') "
                "and valid_until = to_date('2011-01-30','YYYY-MM-DD') "
                "and classification = 4 ");
        Database::Result res10 = conn.exec( query10 );
        BOOST_REQUIRE_EQUAL(7*res10.size() , 7);

        cert_manager_ref->shortenCertification(cid1
                , makeCorbaDate(boost::gregorian::date(2010, 1, 30)));
        std::string query11 (
                "select * from registrar_certification "
                "where registrar_id = 1  "
                "and valid_from = to_date('2010-01-30','YYYY-MM-DD') "
                "and valid_until = to_date('2010-01-30','YYYY-MM-DD') "
                "and classification = 4 ");
        Database::Result res11 = conn.exec( query11 );
        BOOST_REQUIRE_EQUAL(8*res10.size() , 8);

        //unbounded struct sequence client side mapping
        //create owning seq
        Registry::Registrar::Group::GroupList_var
            gr_list (new Registry::Registrar::Group::GroupList);

        //read into seq
        gr_list =  group_manager_ref->getGroups();

        //iterate over seq using reference and read structs
        const Registry::Registrar::Group::GroupList&  gr_list_ref
            = gr_list.in();
        std::cout << "\nGroupList" << std::endl;
        for (CORBA::ULong i=0; i < gr_list_ref.length(); ++i)
        {
            const Registry::Registrar::Group::GroupData& gd = gr_list_ref[i];
            std::cout << "group name: " << gd.name << std::endl;
        }


        Registry::Registrar::Group::MembershipByGroupList_var
            mem_by_grp(new Registry::Registrar::Group::MembershipByGroupList);
        mem_by_grp = group_manager_ref->getMembershipsByGroup(gid1);
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

        Registry::Registrar::Group::MembershipByRegistrarList_var
            mem_by_reg (new Registry::Registrar::Group::MembershipByRegistrarList);
        mem_by_reg = group_manager_ref->getMembershipsByRegistar(1);
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

        Registry::Registrar::Certification::CertificationList_var
            cert_by_reg(new Registry::Registrar::Certification::CertificationList);
        cert_by_reg = cert_manager_ref->getCertificationsByRegistrar(1);
        const Registry::Registrar::Certification::CertificationList& cert_by_reg_ref
            = cert_by_reg.in();
        std::cout << "\nCertificationsByRegistrar" << std::endl;
        for (CORBA::ULong i=0; i < cert_by_reg_ref.length(); ++i)
        {
            const Registry::Registrar::Certification::CertificationData& cd = cert_by_reg_ref[i];
            std::cout << " certification id: " << cd.id << "\n"
                    << " score: " << cd.score << "\n"
                    << " file id: " << cd.evaluation_file_id << "\n"
                    << " from date: " << cd.fromDate.day << ". " << cd.fromDate.month << ". " << cd.fromDate.year << "\n"
                    << " to date: " << cd.toDate.day << ". " << cd.toDate.month << ". " << cd.toDate.year << "\n"
                    << std::endl;
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


}//test_registrar_certification_simple
