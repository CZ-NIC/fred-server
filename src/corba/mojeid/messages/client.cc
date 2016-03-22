#include "src/corba/MojeID2.hh"
#include "src/corba/mojeid/service_name.h"
#include "config.h"
#include "util/cfg/config_handler.h"
#include "util/cfg/config_handler_decl.h"
#include "util/cfg/handle_general_args.h"
#include "util/cfg/handle_corbanameservice_args.h"

#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <set>
#include <boost/assign/list_of.hpp>

struct CommChannel
{
    enum Value
    {
        SMS,
        EMAIL,
        LETTER,
    };
};

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of
    (HandleArgsPtr(new HandleHelpArg("\nUsage: fred-mojeid-msggen <switches>\n")))
    (HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
    (HandleArgsPtr(new HandleCorbaNameServiceArgs));

CORBA::Object_ptr get_object_reference(CORBA::ORB_ptr orb,
                                       const std::string &nameservice_host,
                                       ::in_port_t nameservice_port,
                                       const std::string &nameservice_context);

//////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
    try {
        typedef std::set< CommChannel::Value > SetOfChannels;
        SetOfChannels channels;
        for (char **arg_ptr = argv + 1; *arg_ptr != NULL; ++arg_ptr) {
            static const std::string opt[] = { "-c", "--channel" };
            if ((*arg_ptr == opt[0]) ||
                (*arg_ptr == opt[1]))
            {
                ++arg_ptr;
                if (*arg_ptr == NULL) {
                    break;
                }
                const std::string channel = *arg_ptr;
                if (channel == "sms") {
                    channels.insert(CommChannel::SMS);
                }
                else if (channel == "email") {
                    channels.insert(CommChannel::EMAIL);
                }
                else if (channel == "letter") {
                    channels.insert(CommChannel::LETTER);
                }
                else {
                    throw std::invalid_argument("unknown channel '" + channel + "'");
                }
            }
        }
        FakedArgs fa = CfgArgs::init< HandleHelpArg >(global_hpv)->handle(argc, argv);
        if (channels.empty()) {
            throw std::invalid_argument("no channel specified");
        }
        const HandleCorbaNameServiceArgs *const nameservice_conf_ptr = CfgArgs::instance()->
            get_handler_ptr_by_type< HandleCorbaNameServiceArgs >();
        int arg_cnt = fa.get_argc();
        CORBA::ORB_var orb = CORBA::ORB_init(arg_cnt, fa.get_argv());
        CORBA::Object_var obj = get_object_reference(orb,
            nameservice_conf_ptr->nameservice_host,
            nameservice_conf_ptr->nameservice_port,
            nameservice_conf_ptr->nameservice_context);
        Registry::MojeID::Server_var server = Registry::MojeID::Server::_narrow(obj);
        if (CORBA::is_nil(server)) {
            std::cerr << "Registry::MojeID::Server::_narrow failure: NULL" << std::endl;
            return EXIT_FAILURE;
        }
        for (SetOfChannels::const_iterator channel_ptr = channels.begin(); channel_ptr != channels.end(); ++channel_ptr) {
            switch (*channel_ptr) {
            case CommChannel::SMS:
                server->generate_sms_messages();
                break;
            case CommChannel::EMAIL:
                server->generate_email_messages();
                break;
            case CommChannel::LETTER:
                server->generate_letter_messages();
                break;
            }
        }
        orb->destroy();
    }
    catch (const CORBA::TRANSIENT&) {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the server." << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::SystemException &e) {
        std::cerr << "Caught a CORBA::" << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::Exception &e) {
        std::cerr << "Caught CORBA::Exception: " << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const ReturnFromMain&) {
        std::cout << "Channel specification:\n"
                     "  -c [ --channel ] arg  generates messages into given channel (sms|email|letter)" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////

CORBA::Object_ptr get_object_reference(CORBA::ORB_ptr orb,
                                       const std::string &nameservice_host,
                                       ::in_port_t nameservice_port,
                                       const std::string &nameservice_context)
{
    if (CORBA::is_nil(orb)) {
        std::cerr << "orb is NULL" << std::endl;
        return CORBA::Object::_nil();
    }
    CosNaming::NamingContext_var rootContext;
    try {
        // Obtain a reference to the root context of the Name service:
        std::ostringstream nameservice_uri;
        nameservice_uri << "corbaname::" << nameservice_host <<
                                     ":" << nameservice_port;
        CORBA::Object_var obj = orb->string_to_object(nameservice_uri.str().c_str());
        // Narrow the reference returned.
        rootContext = CosNaming::NamingContext::_narrow(obj);
    }
    catch (const CORBA::NO_RESOURCES&) {
        std::cerr << "Caught NO_RESOURCES exception. You must configure omniORB with the location" << std::endl
                  << "of the naming service." << std::endl;
        return CORBA::Object::_nil();
    }
    catch (const CORBA::ORB::InvalidName&) {
        // This should not happen!
        std::cerr << "Service required is invalid [does not exist]." << std::endl;
        return CORBA::Object::_nil();
    }
    // Create a name object, containing the name test/context:
    static const char* const cos_naming_kind_context = "context";
    static const char* const cos_naming_kind_object = "Object";
    CosNaming::Name name;
    name.length(2);

    name[0].kind = cos_naming_kind_context;
    name[0].id   = nameservice_context.c_str();
    
    name[1].kind = cos_naming_kind_object;
    name[1].id   = Registry::MojeID::service_name.c_str();
    // Note on kind: The kind field is used to indicate the type
    // of the object. This is to avoid conventions such as that used
    // by files (name.type -- e.g. test.ps = postscript etc.)
    try {
        // Resolve the name to an object reference.
        return rootContext->resolve(name);
    }
    catch (const CosNaming::NamingContext::NotFound&) {
        // This exception is thrown if any of the components of the
        // path [contexts or the object] aren’t found:
        std::cerr << "Context '" << nameservice_context <<"' not found." << std::endl;
    }
    catch (const CORBA::TRANSIENT&) {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the naming service." << std::endl
                  << "Make sure the naming server is running and that omniORB is configured correctly." << std::endl;
    }
    catch (const CORBA::SystemException &e) {
        std::cerr << "Caught a CORBA::" << e._name() << " while using the naming service." << std::endl;
    }
    return CORBA::Object::_nil();
}
