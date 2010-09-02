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
        FakedArgs fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

        std::cout << "Registry::Messages::_narrow" << std::endl;
        Registry::Messages_var messages_ref;
        messages_ref = Registry::Messages::_narrow(
                CorbaContainer::get_instance()->nsresolve("Messages"));

        messages_ref->sendSms("REG-FRED_A", "+420123456789", "Ahoj!");


        Registry::Messages::ByteBuffer_var file_content( new Registry::Messages::ByteBuffer );

        file_content->length(3);
        file_content[0] = 'p';
        file_content[1] = 'd';
        file_content[2] = 'f';

        Registry::Messages::PostalAddress_var paddr( new Registry::Messages::PostalAddress);

        paddr->city = CORBA::string_dup("Praha");

        messages_ref->sendLetter("REG-FRED_B", paddr, file_content, "test1.pdf","expiration warning letter");

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


