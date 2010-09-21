#include "mojeid_impl.h"

#include <iostream>
#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "register/db_settings.h"
#include "corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"
#include "corba/connection_releaser.h"

#include "pidfile.h"
#include "daemonize.h"

#include "cfg/config_handler.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"


using namespace std;

const std::string server_name = "fred-mifd";

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
    (HandleArgsPtr(new HandleServerArgs))
    (HandleArgsPtr(new HandleLoggingArgs))
    (HandleArgsPtr(new HandleDatabaseArgs))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs));



int main(int argc, char *argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::instance<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        Logging::Log::Type  log_type = static_cast<Logging::Log::Type>(CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleLoggingArgs>()->log_type);

        boost::any param;
        if (log_type == Logging::Log::LT_FILE) param = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleLoggingArgs>()->log_file;

        if (log_type == Logging::Log::LT_SYSLOG) param = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleLoggingArgs>()
            ->log_syslog_facility;

        Logging::Manager::instance_ref().get(PACKAGE)
            .addHandler(log_type, param);

        Logging::Manager::instance_ref().get(PACKAGE).setLevel(
                static_cast<Logging::Log::Level>(
                CfgArgs::instance()->get_handler_ptr_by_type
                <HandleLoggingArgs>()->log_level));

        Logging::Context ctx(server_name);

        //CORBA init
        FakedArgs orb_fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
              get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
          , ns_args_ptr->nameservice_host
          , ns_args_ptr->nameservice_port
          , ns_args_ptr->nameservice_context);

        //create server
        Registry::MojeIDImpl* server = new Registry::MojeIDImpl(server_name);
        PortableServer::ObjectId_var server_obj_id
            = PortableServer::string_to_ObjectId("MojeID");
        CorbaContainer::get_instance()->poa_persistent
            ->activate_object_with_id(server_obj_id, server);
        CORBA::Object_var server_obj = server->_this();
        server->_remove_ref();
        CorbaContainer::get_instance()->getNS()->bind("MojeID", server_obj);
        CorbaContainer::get_instance()->poa_persistent->the_POAManager()
            ->activate();

        //run server
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleServerArgs>()
                ->do_daemonize)
            daemonize();
        std::string pidfile_name
            = CfgArgs::instance()->get_handler_ptr_by_type<HandleServerArgs>()
                                ->pidfile_name;
        if (!pidfile_name.empty())
            PidFileNS::PidFileS::writePid(getpid(), pidfile_name);



        CorbaContainer::get_instance()->orb->run();
        CorbaContainer::get_instance()->orb->destroy();

    }//try
    catch(CORBA::TRANSIENT&)
    {
        cerr << "Caught system exception TRANSIENT -- unable to contact the "
             << "server." << endl;
        return EXIT_FAILURE;
    }
    catch(CORBA::SystemException& ex)
    {
        cerr << "Caught a CORBA::" << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch(CORBA::Exception& ex)
    {
        cerr << "Caught CORBA::Exception: " << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch(omniORB::fatalException& fe)
    {
        cerr << "Caught omniORB::fatalException:" << endl;
        cerr << "  file: " << fe.file() << endl;
        cerr << "  line: " << fe.line() << endl;
        cerr << "  mesg: " << fe.errmsg() << endl;
        return EXIT_FAILURE;
    }

    catch(const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch(std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        std::cout << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
