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

        std::cout
        << "\nact_time: " << act_time
        << "\ndate: " << act_time.date()
        << "\nltime: " << lact_time	
        << "\nldate: " << buf.str()
        //<< "\nldate cz: " << StrConvert<date>().to(lact_time.date())
        << "\nl stringify not a date: " << stringify(boost::gregorian::date())
        << "\n test: " << stringify( birthdate_from_string_to_date("  2009 -1- 15  "))
        << "\n 1522006: " << stringify(birthdate_from_string_to_date("  1522006  "))
        << std::endl;
    }

    //yyyy-mm-dd
    BOOST_REQUIRE_EQUAL(stringify( birthdate_from_string_to_date("  2009 -1- 15  ")).compare("15.01.2009") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("1970-01-15  ")).compare("15.01.1970") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("  2009-02-29  ")).compare(""), 0);

    //yyyy/mm/dd
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("  2009/ 2 /28  ")).compare("28.02.2009") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("1962/02/28  ")).compare("28.02.1962") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("  2009/02/29  ")).compare(""), 0);

    //yyyymmdd
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("  20080229  ")).compare("29.02.2008") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("1963/02/28  ")).compare("28.02.1963") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("  20090229  ")).compare(""), 0);

    //ddmmyyyy
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("  29022004  ")).compare("29.02.2004") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("28021964")).compare("28.02.1964") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("29022005")).compare(""), 0);

    //dd.mm.yyyy
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("29.02.2000")).compare("29.02.2000") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("28.02.1965")).compare("28.02.1965") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("29.2.1999")).compare(""), 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("29.2.2100")).compare(""), 0);

    //dd/mm/yyyy
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("15/12/2000")).compare("15.12.2000") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("15/12/1965")).compare("15.12.1965") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("32/12/1999")).compare(""), 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("11/2/2100")).compare(""), 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("111/2/2100")).compare(""), 0);

    //yymmdd
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("001215")).compare("15.12.2000") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("661215")).compare("15.12.1966") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("991232")).compare(""), 0);

    //ddmmyy
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("151200")).compare("15.12.2000") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("151266")).compare("15.12.1966") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("321299")).compare(""), 0);

    //yyyymmdd000000
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("20011215000000")).compare("15.12.2001") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("19671215000000")).compare("15.12.1967") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("19671315000000")).compare(""), 0);

    //ddmyyyy
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("  1522008  ")).compare("15.02.2008") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("2821966")).compare("28.02.1966") , 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("2922005")).compare(""), 0);
    BOOST_REQUIRE_EQUAL(stringify(birthdate_from_string_to_date("2005229")).compare(""), 0);

}
