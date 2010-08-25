//Messages.cc
#include "messages.h"

using namespace std;

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of

(HandleArgsPtr(new HandleHelpArg("\nUsage: fred-msgd <switches>\n")))
(HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs));

//implementational code for IDL interface Registry::Messages
Registry_Messages_i::Registry_Messages_i()
{
  // add extra constructor code here
}
Registry_Messages_i::~Registry_Messages_i()
{
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
void Registry_Messages_i::sendSms(const char* contact_handle
        , const char* phone
        , const char* content)
{
    try
    {
        Registry::MessagesImpl::send_sms_impl(contact_handle,phone, content);//call of impl
    }//try
    catch(std::exception& ex)
    {
        throw Registry::Messages::ErrorReport(ex.what());
    }
    catch(...)
    {
        throw Registry::Messages::ErrorReport("unknown exception");
    }
}//Registry_Messages_i::sendSms

void Registry_Messages_i::sendLetter(const char* contact_handle
        , const Registry::Messages::PostalAddress& address
        , const Registry::Messages::ByteBuffer& file_content
        , const char* file_type)
{
    try
    {
        Registry::MessagesImpl::PostalAddress address_impl;
        address_impl.name = address.name;
        address_impl.org = address.org;
        address_impl.street1 = address.street1;
        address_impl.street2 = address.street2;
        address_impl.street3 = address.street3;
        address_impl.city = address.city;
        address_impl.state = address.state;
        address_impl.code = address.code;
        address_impl.county = address.county;

        Registry::MessagesImpl::ByteBuffer buffer_impl;

        CORBA::ULong file_content_length = file_content.length();
        for(CORBA::ULong i = 0; i < file_content_length; ++i)
            buffer_impl.push_back(buffer_impl[i]);

        Registry::MessagesImpl::send_letter_impl(contact_handle
                , address_impl
                , buffer_impl
                , file_type );//call of impl
    }//try
    catch(std::exception& ex)
    {
        throw Registry::Messages::ErrorReport(ex.what());
    }
    catch(...)
    {
        throw Registry::Messages::ErrorReport("unknown exception");
    }
}//Registry_Messages_i::sendLetter

int main(int argc, char** argv)
{
    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {   //config
        fa = CfgArgs::instance<HandleHelpArg>(global_hpv)->handle(argc, argv);

        //db connection
        Database::Connection conn = Database::Manager::acquire();

        //CORBA init
        FakedArgs fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
              get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
          , ns_args_ptr->nameservice_host
          , ns_args_ptr->nameservice_port
          , ns_args_ptr->nameservice_context);

        //create server
        Registry_Messages_i* myRegistry_Messages_i = new Registry_Messages_i();
        PortableServer::ObjectId_var msgObjectId
            = PortableServer::string_to_ObjectId("Messages");
        CorbaContainer::get_instance()->poa_persistent
            ->activate_object_with_id(msgObjectId, myRegistry_Messages_i);
        CORBA::Object_var msgObj = myRegistry_Messages_i->_this();
        myRegistry_Messages_i->_remove_ref();
        CorbaContainer::get_instance()->getNS()->bind("Messages",msgObj);
        CorbaContainer::get_instance()->poa_persistent->the_POAManager()
            ->activate();

        //run server
        daemonize();//TODO: config options for daemonize and pidfile
        PidFileNS::PidFileS::writePid(getpid(), "msgdpid");
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
}//main
