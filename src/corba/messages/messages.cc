/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @messages.cc
 *  corba server implementation of registry messages
 */

#include "messages.h"
#include "messages_filemanager.h"

using namespace std;

//config args processing
HandlerPtrVector global_hpv =
boost::assign::list_of

(HandleArgsPtr(new HandleHelpArg("\nUsage: fred-msgd <switches>\n")))
(HandleArgsPtr(new HandleConfigFileArgs(CONFIG_FILE) ))
(HandleArgsPtr(new HandleServerArgs))
(HandleArgsPtr(new HandleLoggingArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs));

//implementational code for IDL interface Registry::Messages
Registry_Messages_i::Registry_Messages_i(Register::Messages::ManagerPtr msgmgr)
    : msgmgr_(msgmgr)
{
  // add extra constructor code here
}
Registry_Messages_i::~Registry_Messages_i()
{
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
CORBA::ULongLong Registry_Messages_i::sendSms(const char* contact_handle
        , const char* phone
        , const char* content
        , const char* message_type
        , CORBA::ULongLong contact_object_registry_id
        , CORBA::ULongLong contact_history_historyid
       )
{
    Logging::Context ctx(server_name);
    ConnectionReleaser releaser;

    try
    {
        return msgmgr_->send_sms(contact_handle,phone, content
                , message_type
                , contact_object_registry_id
                , contact_history_historyid
                );//call of impl
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

CORBA::ULongLong Registry_Messages_i::sendLetter(const char* contact_handle
        , const Registry::Messages::PostalAddress& address
        , const Registry::Messages::ByteBuffer& file_content
        , const char* file_name
        , const char* file_type
        , const char* message_type
        , CORBA::ULongLong contact_object_registry_id
        , CORBA::ULongLong contact_history_historyid
        )
{
    Logging::Context ctx(server_name);
    ConnectionReleaser releaser;

    try
    {
        LOGGER(PACKAGE).debug(boost::format(
                  "Registry_Messages_i::sendLetter"
                " contact_handle: %1%")
            % contact_handle);

        Register::Messages::PostalAddress address_impl;
        address_impl.name = std::string(address.name.in());
        address_impl.org = std::string(address.org.in());
        address_impl.street1 = std::string(address.street1.in());
        address_impl.street2 = std::string(address.street2.in());
        address_impl.street3 = std::string(address.street3.in());
        address_impl.city = std::string(address.city.in());
        address_impl.state = std::string(address.state.in());
        address_impl.code = std::string(address.code.in());
        address_impl.country = std::string(address.country.in());

        std::size_t file_content_size= file_content.length();
        unsigned char* file_content_buffer
            = const_cast<unsigned char*>(file_content.get_buffer());

        unsigned long long filetype_id = Register::Messages::get_filetype_id(file_type);

        std::vector<char> file_buffer(file_content_buffer, file_content_buffer+file_content_size) ;

        unsigned long long file_id = 0;

        //call filemanager client
        file_id = save_file(file_buffer
                , file_name
                , "application/pdf"
                , filetype_id );

        return msgmgr_->send_letter(contact_handle
                , address_impl
                , file_id
                , message_type
                , contact_object_registry_id
                , contact_history_historyid
                );//call of impl

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

const char* server_name = "msgd";//for logging contxt

int main(int argc, char** argv)
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


        //db connection
        Database::Connection conn = Database::Manager::acquire();

        //CORBA init
        FakedArgs orb_fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
              get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
          , ns_args_ptr->nameservice_host
          , ns_args_ptr->nameservice_port
          , ns_args_ptr->nameservice_context);

        //Messages Manager
        Register::Messages::ManagerPtr msgmgr
            = Register::Messages::create_manager();

        //create server
        Registry_Messages_i* myRegistry_Messages_i = new Registry_Messages_i(msgmgr);
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
}//main
