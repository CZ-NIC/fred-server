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

#define BOOST_TEST_MODULE Test Conversions

#include "test-conv.h"



//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
;

#include "cfg/test_custom_main.h"



BOOST_AUTO_TEST_CASE( test_exec )
{
    int ret = 0;
    try
    {
        //current utc ptime
        boost::posix_time::ptime act_time
            = boost::posix_time::second_clock::universal_time();

         boost::posix_time::ptime lact_time;

        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
                "SELECT $1 at time zone 'Europe/Prague' "
                , Database::query_param_list (act_time));
        if (res.size() == 1)
        {
            lact_time = res[0][0];
        }

        std::ostringstream buf;
        buf.imbue(std::locale(std::locale(""),new date_facet("%x")));
        buf << lact_time.date();

        std::ostringstream buf_cz;
        buf_cz.imbue(std::locale(std::locale("cs_CZ.utf8"),new date_facet("%x")));
        //buf.imbue(std::locale("cs_CZ.utf8"));
        buf_cz << lact_time.date();


        std::cout
        << "\nact_time: " << act_time
        << "\ndate: " << act_time.date()
        << "\nltime: " << lact_time	
        << "\nldate: " << buf.str()
        << "\nldate cz: " << buf_cz.str()	
        << std::endl;


    }
    catch(...)
    {
        ret = 1;//error
    }
    BOOST_REQUIRE_EQUAL(ret , 0);
}
