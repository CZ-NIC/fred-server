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

#include "faked_args.h"

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
            " where registrar_group.short_name = 'testgroup1'");
        Database::Result res6 = conn.exec( query6 );
        BOOST_REQUIRE_EQUAL(2*res6.size() , 4);





        std::cout << "admin_ref->getCertificationManager()" << std::endl;
        Registry::Registrar::Certification::Manager_var cert_manager_ref;
        cert_manager_ref = admin_ref->getCertificationManager();

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

        BOOST_REQUIRE_EQUAL(registrar_certification_test() , 0);

}//test_registrar_certification_simple
