#include "domain_browser_i.h"

#include <iostream>
#include <string>

#include <boost/assign/list_of.hpp>
#include <utility>

#include "src/fredlib/db_settings.h"
#include "util/corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"
#include "src/corba/connection_releaser.h"

#include "setup_server.h"

#include "cfg/config_handler.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_mojeid_args.h"
#include "cfg/handle_domainbrowser_args.h"


using namespace std;

const string server_name = "fred-dbifd";

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
    (HandleArgsPtr(new HandleServerArgs))
    (HandleArgsPtr(new HandleLoggingArgs))
    (HandleArgsPtr(new HandleDatabaseArgs))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs))
    (HandleArgsPtr(new HandleRegistryArgs))
    (HandleArgsPtr(new HandleMojeIDArgs))
    (HandleArgsPtr(new HandleDomainBrowserArgs));



int main(int argc, char *argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        setup_logging(CfgArgs::instance());

        Logging::Context ctx(server_name);

        // config dump
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump)
        {
            for (std::string config_item = AccumulatedConfig::get_instance().pop_front();
                !config_item.empty(); config_item = AccumulatedConfig::get_instance().pop_front())
            {
                Logging::Manager::instance_ref().get(PACKAGE).debug(config_item);
            }
        }

        //CORBA init
        corba_init();

        //MojeID registrar used for updates in domain browser
        std::string update_registrar_handle = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle;

        //domain list chunk size
        unsigned int domain_list_limit = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->domain_list_limit;

        //nsset list chunk size
        unsigned int nsset_list_limit = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->nsset_list_limit;

        //keyset list chunk size
        unsigned int keyset_list_limit = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->keyset_list_limit;

        //contact list chunk size
        unsigned int contact_list_limit = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleDomainBrowserArgs>()->contact_list_limit;

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()
            ->register_server(new Registry::DomainBrowser::Server_i(server_name, update_registrar_handle,
                    domain_list_limit, nsset_list_limit, keyset_list_limit, contact_list_limit)
            , "DomainBrowser");
        run_server(CfgArgs::instance(), CorbaContainer::get_instance());

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
    catch(exception& ex)
    {
        cerr << "Error: " << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        cerr << "Unknown Error" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
