#include "src/corba/MojeID2.hh"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <netinet/in.h>

CORBA::Object_ptr get_object_reference(CORBA::ORB_ptr orb,
                                       const std::string &nameservice_host,
                                       ::in_port_t nameservice_port,
                                       const std::string &nameservice_context);

//////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
    try {
        CORBA::ORB_var               orb    = CORBA::ORB_init(argc, argv);
        CORBA::Object_var            obj    = get_object_reference(orb, "localhost", 11111, "fred");
        Registry::MojeID::Server_var server = Registry::MojeID::Server::_narrow(obj);
        server->generate_email_messages();
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
    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////

CORBA::Object_ptr get_object_reference(CORBA::ORB_ptr orb,
                                       const std::string &nameservice_host,
                                       ::in_port_t nameservice_port,
                                       const std::string &nameservice_context)
{
    CosNaming::NamingContext_var rootContext;
    try {
        // Obtain a reference to the root context of the Name service:
        std::ostringstream nameservice_uri;
        nameservice_uri << "corbaname::" << nameservice_host <<
                                     ":" << nameservice_port;
        CORBA::Object_var obj = orb->string_to_object(nameservice_uri.str().c_str());
        // Narrow the reference returned.
        rootContext = CosNaming::NamingContext::_narrow(obj);
        if (CORBA::is_nil(rootContext)) {
            std::cerr << "Failed to narrow the root naming context." << std::endl;
            return CORBA::Object::_nil();
        }
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
    name[1].id   = "Registry";
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
        std::cerr << "Context not found." << std::endl;
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
