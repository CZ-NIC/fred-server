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

#define BOOST_TEST_MODULE Test db

#include "test-db.h"



//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
;

#include "cfg/test_custom_main.h"



BOOST_AUTO_TEST_CASE( test_exec )
{
    const Database::QueryParam qp1 = Database::QPNull;
    std::cerr << "QPNull: " << qp1.print_buffer() << std::endl;

    const Database::QueryParam qp2 = Database::QueryParam(true,"abc");
    std::cerr << "QueryParam binary: " << qp2.print_buffer() << std::endl;

    const Database::QueryParam qp3 = "abc";
    std::cerr << "QueryParam text: " << qp3.print_buffer() << std::endl;


    BOOST_REQUIRE_EQUAL(exec_params_test() , 0);

}


