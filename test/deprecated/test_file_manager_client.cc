/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <utility>

#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/deprecated/test_file_manager_client.hh"

BOOST_AUTO_TEST_SUITE(Files)

BOOST_AUTO_TEST_CASE( file_manager_client_simple )
{

    //test data
    Random::Generator rnd;

    std::string test_data_str = rnd.get_seq(Random::CharSet::letters(), 1024*1024);
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
        = fm_client.upload(in_test_data_vect,"./test_file.pdf","application/pdf",6);

    std::vector<char> out_buffer;
    fm_client.download(file_id, out_buffer);

    std::string out_test_data_str (out_buffer.begin(), out_buffer.end());

    BOOST_CHECK_EQUAL( test_data_str.compare(out_test_data_str) , 0);

}

BOOST_AUTO_TEST_SUITE_END();
