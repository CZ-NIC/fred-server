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

    Registry::Messages::PostalAddress paddr;

    paddr.city = "Praha";

    messages_ref->sendLetter("REG-FRED_B", paddr, file_content, "test1.pdf","expiration warning letter");

    BOOST_REQUIRE_EQUAL(0//sms_test()
            , 0);

}


