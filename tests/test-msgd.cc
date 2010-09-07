/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
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

#define BOOST_TEST_MODULE Test msgd

#include "test-msgd.h"

//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs))
;

#include "cfg/test_custom_main.h"



BOOST_AUTO_TEST_CASE( test_exec )
{
    try
    {
        //CORBA init
        FakedArgs orb_fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

        std::cout << "Registry::Messages::_narrow" << std::endl;
        Registry::Messages_var messages_ref;
        messages_ref = Registry::Messages::_narrow(
                CorbaContainer::get_instance()->nsresolve("Messages"));

        for (int i = 0; i < 1000000; ++i)
        {
            //sms test
            CORBA::String_var sms_contact = CORBA::string_dup("REG-FRED_A");
            CORBA::String_var phone = CORBA::string_dup("+420123456789");
            CORBA::String_var content = CORBA::string_dup("Ahoj!");
            CORBA::String_var sms_message_type = CORBA::string_dup("password_reset");

            messages_ref->sendSms(sms_contact, phone , content, sms_message_type, 1, 1);

            //letter test
            CORBA::String_var letter_contact = CORBA::string_dup("REG-FRED_B");

            Registry::Messages::PostalAddress_var paddr( new Registry::Messages::PostalAddress);

            paddr->name = CORBA::string_dup("");
            paddr->org = CORBA::string_dup("");
            paddr->street1 = CORBA::string_dup("");
            paddr->street2 = CORBA::string_dup("");
            paddr->street3 = CORBA::string_dup("");
            paddr->city = CORBA::string_dup("");
            paddr->state = CORBA::string_dup("");
            paddr->code = CORBA::string_dup("");
            paddr->city = CORBA::string_dup("Praha");
            paddr->country = CORBA::string_dup("");

            Registry::Messages::ByteBuffer_var file_content( new Registry::Messages::ByteBuffer(3) );//prealocate
            file_content->length(3);//set/alocate
            CORBA::Octet* data = file_content->get_buffer();
            data[0] = 'p';
            data[1] = 'd';
            data[2] = 'f';

            CORBA::String_var file_name = CORBA::string_dup("test1.pdf");

            CORBA::String_var file_type = CORBA::string_dup("expiration warning letter");
            CORBA::String_var letter_message_type = CORBA::string_dup("password_reset");

            messages_ref->sendLetter(letter_contact, paddr.in()
                    , file_content, file_name,file_type
                    , letter_message_type, 1, 1);
        }
        BOOST_REQUIRE_EQUAL(0//sms_test()
                , 0);
    }
    catch(Registry::Messages::ErrorReport& er)
    {
        std::cerr << "Caught exception ErrorReport: "
             << er.reason.in() << std::endl;

    }

    catch(CORBA::TRANSIENT&)
    {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the "
             << "server." << std::endl;
    }
    catch(CORBA::SystemException& ex)
    {
        std::cerr << "Caught a CORBA::" << ex._name() << std::endl;
    }
    catch(CORBA::Exception& ex)
    {
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
    }
    catch(omniORB::fatalException& fe)
    {
        std::cerr << "Caught omniORB::fatalException:" << std::endl;
        std::cerr << "  file: " << fe.file() << std::endl;
        std::cerr << "  line: " << fe.line() << std::endl;
        std::cerr << "  mesg: " << fe.errmsg() << std::endl;

    }
    catch(std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;

    }
    catch(...)
    {
        std::cout << "Unknown Error" << std::endl;

    }


}


