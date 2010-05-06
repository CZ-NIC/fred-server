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
        CorbaSingleton* cs = CorbaSingleton::instance();

        int argc = CfgArgs::instance()->fa.get_argc();

        // Initialise the ORB
        std::cout << "ORB_init" << std::endl;
        cs->cc.orb = CORBA::ORB_init( argc
                , CfgArgs::instance()->fa.get_argv());

      // Obtain a reference to the root POA.
        std::cout << "resolve_initial_references RootPOA" << std::endl;
        cs->cc.root_initial_ref
            = cs->cc.orb->resolve_initial_references("RootPOA");

        std::cout << "PortableServer::POA::_narrow" << std::endl;
        cs->cc.poa = PortableServer::POA::_narrow(cs->cc.root_initial_ref);

        if (CfgArgs::instance()
                ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
                ->nameservice_host.empty())
        {
            std::cout << "resolve_initial_references NameService" << std::endl;
            cs->cc.nameservice_ref
                = cs->cc.orb->resolve_initial_references("NameService");
        }
        else
        {
            std::cout << "string_to_object corbaname::" << CfgArgs::instance()
                ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
                    ->nameservice_host << std::endl;
            cs->cc.nameservice_ref = cs->cc.orb->string_to_object(
                ("corbaname::"
                + CfgArgs::instance()
                ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
                ->nameservice_host
                + ":"
                + boost::lexical_cast<std::string>(CfgArgs::instance()
                ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
                ->nameservice_port)
                ).c_str()
            );
        }

        std::cout << "CosNaming::NamingContext::_narrow" << std::endl;
        cs->cc.root_nameservice_context
            = CosNaming::NamingContext::_narrow(cs->cc.nameservice_ref.in());

        std::cout << "if CORBA::is_nil cs->cc.root_nameservice_context" << std::endl;
        if (CORBA::is_nil(cs->cc.root_nameservice_context))
            throw "cs->cc.root_nameservice_context";


        ccReg::Admin_var admin_ref;
        Registry::Registrar::Group::Manager_var group_manager;
        Registry::Registrar::Certification::Manager_var cert_manager;

        //Create a name object, containing the name test/context
        CosNaming::Name contextName;
        contextName.length(2);

        contextName[0].id   = CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>()
            ->nameservice_context.c_str();
        contextName[0].kind = "context";
        contextName[1].id   = "Admin";
        contextName[1].kind = "Object";

        std::cout << "root_nameservice_context->resolve contextName" << std::endl;
        cs->cc.root_nameservice_context->resolve(contextName);
        std::cout << "ccReg::Admin::_narrow" << std::endl;
        admin_ref = ccReg::Admin::_narrow(cs->cc.root_nameservice_context->resolve(contextName));

        std::cout << "admin_ref->getGroupManager()" << std::endl;
        group_manager= admin_ref->getGroupManager();
        std::cout << "admin_ref->getCertificationManager()" << std::endl;
        cert_manager = admin_ref->getCertificationManager();


       // while(cs->cc.orb->work_pending())
         //   cs->cc.orb->perform_work();//run();


      std::cout << "before orb destroy" << std::endl;
      cs->cc.orb->destroy();
      std::cout << "after orb destroy" << std::endl;
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

