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
//not using UTF defined main
#define BOOST_TEST_NO_MAIN


#include "random_data_generator.h"
#include "faked_args.h"
#include "concurrent_queue.h"

// Sun CC doesn't handle boost::iterator_adaptor yet
#if !defined(__SUNPRO_CC) || (__SUNPRO_CC > 0x530)
#include <boost/generator_iterator.hpp>
#endif

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
  using ::time;
}
#endif


//compose args processing
class CmdLineArgHandlers
{
    //nonowning container of handlers
    typedef std::vector<HandleArgs*> HandlerVector;
    HandlerVector handler;
public:
    HandleGeneralArgs general_args;
    HandleDatabaseArgs database_args;
    HandleThreadGroupArgs thread_group_args;

    CmdLineArgHandlers()
    {
        //order of arguments processing
        handler.push_back(&general_args);
        handler.push_back(&database_args);
        handler.push_back(&thread_group_args);

        //gater options_descriptions for help print
        for(HandlerVector::iterator i = handler.begin(); i != handler.end(); ++i )
            general_args.po_description.push_back((*i)->get_options_description());
    }

    FakedArgs fa;
    FakedArgs handle( int argc, char* argv[])
    {
        //initial fa
        fa.prealocate_for_argc(argc);
        for (int i = 0; i < argc ; ++i)
            fa.add_argv(argv[i]);

        for(HandlerVector::iterator i = handler.begin(); i != handler.end(); ++i )
        {
            FakedArgs fa_out;
            (*i)->handle( fa.get_argc(), fa.get_argv(), fa_out);
            fa=fa_out;//last output to next input
        }
        return fa;
    }

}cmdlinehandlers;


#include "test-registrar-certification-group.h"

BOOST_AUTO_TEST_CASE( test_corba )
{
    try
    {
        CorbaSingleton* cs = CorbaSingleton::instance();

        int argc = cmdlinehandlers.fa.get_argc();

        // Initialise the ORB

        cs->cc.orb = CORBA::ORB_init( argc
                , cmdlinehandlers.fa.get_argv());

      // Obtain a reference to the root POA.
        cs->cc.root_initial_ref = cs->cc.orb->resolve_initial_references("RootPOA");

        cs->cc.poa = PortableServer::POA::_narrow(cs->cc.root_initial_ref);

      while(cs->cc.orb->work_pending())
          cs->cc.orb->perform_work();//run();


      std::cout << "before orb destroy" << std::endl;
      cs->cc.orb->destroy();
      std::cout << "after orb destroy" << std::endl;

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


}//test_corba

BOOST_AUTO_TEST_CASE( test_registrar_certification )
{
    BOOST_REQUIRE_EQUAL(registrar_certification_test() , 0);
    //BOOST_REQUIRE_EXCEPTION( test(), std::exception , check_std_exception_nodatafound);

}


int main( int argc, char* argv[] )
{
    //processing of additional program options
    //producing faked args with unrecognized ones
    FakedArgs fa;
    try
    {
        fa = cmdlinehandlers.handle(argc, argv);
    }
    catch(const ReturnFromMain&)
    {
        return 0;
    }

//fn init_unit_test_suite added in 1.35.0
#if ( BOOST_VERSION > 103401 )

    // prototype for user's unit test init function
#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    extern bool init_unit_test();

    boost::unit_test::init_unit_test_func init_func = &init_unit_test;
#else
    extern ::boost::unit_test::test_suite* init_unit_test_suite( int argc, char* argv[] );

    boost::unit_test::init_unit_test_func init_func = &init_unit_test_suite;
#endif

    return ::boost::unit_test::unit_test_main( init_func, fa.get_argc(), fa.get_argv() );//using fake args
#else //1.34.1 and older
    return ::boost::unit_test::unit_test_main(  fa.get_argc(), fa.get_argv() );//using fake args
#endif //1.35.0 and newer

}

