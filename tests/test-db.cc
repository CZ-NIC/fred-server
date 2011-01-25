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


#include "test-db.h"
BOOST_AUTO_TEST_SUITE(DbParamQuery)
BOOST_AUTO_TEST_CASE( init_params_test )
{
    const Database::QueryParam qp1 = Database::QPNull;
    BOOST_TEST_MESSAGE( "QPNull: " << qp1.print_buffer() );

    const Database::QueryParam qp2 = Database::QueryParam(true,Database::QueryParamData("abc"));
    BOOST_TEST_MESSAGE( "QueryParam binary: " << qp2.print_buffer() );

    const Database::QueryParam qp3 = "abc";
    BOOST_TEST_MESSAGE("QueryParam text: " << qp3.print_buffer() );
}
BOOST_AUTO_TEST_CASE( exec_params_test )
{


    {
        unsigned ret=0;
        try
        {   //test param data in strings
            Database::Connection conn = Database::Manager::acquire();
            std::string query = "select 1 as id, $1::bigint as data1, $2::bigint as data2, $3::text as data3";

            std::vector<std::string> params = boost::assign::list_of("2")("3")("Kuk");


            Database::Result res = conn.exec_params( query, params );
            if ((res.size() > 0) && (res[0].size() == 4))
            {
                //check data
                if(1 != static_cast<unsigned long long>(res[0][0]) ) ret+=1;
                if(2 != static_cast<unsigned long long>(res[0][1])) ret+=2;
                if(3 != static_cast<unsigned long long>(res[0][2])) ret+=4;
                BOOST_TEST_MESSAGE("test string: " << std::string(res[0][3]));
            }//if res size
            else ret+=8;
            if (ret != 0 ) BOOST_TEST_MESSAGE( "exec_params_test ret: "<< ret );


            std::string qquery = "select $1::int as id, $2::bigint as data1"
                ", $3::int as data2, $4::text as data3, $4::text = 'Kuk' as data4"
                ", $5::int as data5"
                    ;

            Database::QueryParams qparams = Database::query_param_list
                (1)//$1
                (1ll)//$2
                (-1l)//$3
                ("Kuk")//$4
                (Database::QPNull)//$5
                ;

            qparams[2].print_buffer();
            qparams[3].print_buffer();

            //test simple binary params
            Database::Result qres = conn.exec_params( qquery, qparams );

            if ((qres.size() > 0) && (qres[0].size() == 6))
            {
                //check data
                if(1 != static_cast<long>(qres[0][0]) ) ret+=16;
                if(1 != static_cast<long long>(qres[0][1])) ret+=32;
                if(-1 != static_cast<long>(qres[0][2])) ret+=64;

                //std::cout << "test string: " << std::string(qres[0][3])
                //<< " data4: " << std::string(qres[0][4]) <<std::endl;

                if(std::string(qres[0][3]).compare("Kuk")!=0) ret+=1024;
                if(true != static_cast<bool>(qres[0][4])) ret+=2048;

                BOOST_TEST_MESSAGE( "test null: " << std::string(qres[0][5]));

            }//if qres size
            else ret+=128;
            if (ret != 0 ) BOOST_TEST_MESSAGE("exec_params_test ret: "<< ret);


        }
        catch(std::exception& ex)
        {
            BOOST_TEST_MESSAGE("exec_params_test exception reason: "<< ex.what() );
            ret+=256;
            throw;
        }
        catch(...)
        {
            BOOST_TEST_MESSAGE("exec_params_test exception returning");
            ret+=512;
            if (ret != 0 ) BOOST_TEST_MESSAGE("exec_params_test ret: "<< ret);
        }

        BOOST_CHECK_EQUAL(ret , 0);
    }




}

BOOST_AUTO_TEST_CASE(test_timeout_exception)
{
    BOOST_TEST_MESSAGE("Check whether long queries yield an exception with expected message. ");

    Database::Connection conn = Database::Manager::acquire();

    // timeout in miliseconds
    conn.setQueryTimeout(900);

    bool thrown = false;
    // now try to sleep for 1 second, it should yield the exception
    try {
        conn.exec("select pg_sleep(1)");
    } catch (Database::Exception &ex) {
        thrown = true;
        std::string message = ex.what();
        // check if the exception contains the text rely on
        if (message.find(Database::Connection::getTimeoutString())
                != std::string::npos) {
            // ok
        } else {
            BOOST_FAIL("Exception doesn't contain expected message. ");
        }
    }

    BOOST_REQUIRE_MESSAGE(thrown, "No timeout (or any other) exception thrown, wrong behaviour. ");

}

BOOST_AUTO_TEST_SUITE_END();


