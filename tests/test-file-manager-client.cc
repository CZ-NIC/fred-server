/*
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

 // test-file-manager-client.cc


#define BOOST_TEST_MODULE Test file manager client

#include "test-file-manager-client.h"

//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs))
;

#include "test_custom_main.h"

#include "file_manager_client.h"


BOOST_AUTO_TEST_CASE( test_fmc_simple )
{

    //test data
    RandomDataGenerator rdg(0);

    std::string test_data_str = rdg.xstring(1024*1024);
    std::vector<char> in_test_data_vect(test_data_str.begin(), test_data_str.end());


    //corba config
    FakedArgs fa = CfgArgs::instance()->fa;
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
        , ns_args_ptr->nameservice_host
        , ns_args_ptr->nameservice_port
        , ns_args_ptr->nameservice_context);

    FileManagerClient fm_client(
            CorbaContainer::get_instance()->getNS());
    unsigned long long file_id
        //= fm_client.upload("./test-file.pdf","application/pdf",6);
        = fm_client.upload(in_test_data_vect,"./test-file.pdf","application/pdf",6);

    std::vector<char> out_buffer;
    fm_client.download(file_id, out_buffer);

    std::string out_test_data_str (out_buffer.begin(), out_buffer.end());

    BOOST_REQUIRE_EQUAL( test_data_str.compare(out_test_data_str) , 0);

}
