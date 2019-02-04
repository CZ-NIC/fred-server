#include "src/bin/corba/connection_releaser.hh"
#include "src/bin/corba/mojeid/server_i.hh"
#include "libfred/db_settings.hh"
#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/corba_wrapper.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"
#include "src/util/setup_server.hh"

#include <boost/assign/list_of.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <utility>

const std::string server_name = "fred-mifd";

//config args processing
HandlerPtrVector global_hpv =
        boost::assign::list_of
                // clang-format off
                (HandleArgsPtr(new HandleHelpArg("\nUsage: " + server_name + " <switches>\n")))
                (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
                (HandleArgsPtr(new HandleServerArgs))
                (HandleArgsPtr(new HandleLoggingArgs))
                (HandleArgsPtr(new HandleDatabaseArgs))
                (HandleArgsPtr(new HandleCorbaNameServiceArgs))
                (HandleArgsPtr(new HandleRegistryArgs))
                (HandleArgsPtr(new HandleMojeIdArgs))
                (HandleArgsPtr(new HandleContactVerificationArgs));
                // clang-format on


int main(int argc, char* argv[])
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    { //config
        fa = CfgArgs::init<HandleHelpArg>(global_hpv)->handle(argc, argv);

        // setting up logger
        setup_logging(CfgArgs::instance());

        Logging::Context ctx(server_name);

        // config dump
        if (CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump)
        {
            for (std::string config_item = AccumulatedConfig::get_instance().pop_front();
                    !config_item.empty();
                    config_item = AccumulatedConfig::get_instance().pop_front())
            {
                Logging::Manager::instance_ref().debug(config_item);
            }
        }

        //CORBA init
        corba_init();

        //create server object with poa and nameservice registration
        CorbaContainer::get_instance()->register_server(
                new Registry::MojeId::Server_i(server_name),
                Registry::MojeId::service_name);
        run_server(CfgArgs::instance(), CorbaContainer::get_instance());
        return EXIT_SUCCESS;
    } //try
    catch (const CORBA::TRANSIENT&)
    {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the server." << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::SystemException& e)
    {
        std::cerr << "Caught a CORBA::" << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::Exception& e)
    {
        std::cerr << "Caught CORBA::Exception: " << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const omniORB::fatalException& e)
    {
        std::cerr << "Caught omniORB::fatalException:" << std::endl
                  << "  file: " << e.file() << std::endl
                  << "  line: " << e.line() << std::endl
                  << "  mesg: " << e.errmsg() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }
}
