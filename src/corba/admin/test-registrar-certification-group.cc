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

HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleThreadGroupArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs));

#include "test_custom_main.h"

#include "random_data_generator.h"
#include "concurrent_queue.h"

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
  using ::time;
}
#endif

#include "test-registrar-certification-group.h"


BOOST_AUTO_TEST_CASE( test_config )
{
    std::cout << CfgArgs::instance()
        ->get_handler_by_type<HandleGeneralArgs>()<< std::endl;
}//test_config

BOOST_AUTO_TEST_CASE( test_corba )
{
    //  try
    //  {

    CorbaContainer::set_instance(CfgArgs::instance()->fa
        , CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
            ->nameservice_host
        , CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
            ->nameservice_port
        , CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
            ->nameservice_context
        );

        std::cout << "ccReg::Admin::_narrow" << std::endl;
        ccReg::Admin_var admin_ref;
        admin_ref = ccReg::Admin::_narrow(CorbaContainer::get_instance()->nsresolve("Admin"));

        std::cout << "admin_ref->getGroupManager()" << std::endl;
        Registry::Registrar::Group::Manager_var group_manager;
        group_manager= admin_ref->getGroupManager();

        std::cout << "admin_ref->getCertificationManager()" << std::endl;
        Registry::Registrar::Certification::Manager_var cert_manager;
        cert_manager = admin_ref->getCertificationManager();

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

}//test_corba

BOOST_AUTO_TEST_CASE( test_registrar_certification )
{
    BOOST_REQUIRE_EQUAL(registrar_certification_test() , 0);
    //BOOST_REQUIRE_EXCEPTION( test(), std::exception , check_std_exception_nodatafound);
}

