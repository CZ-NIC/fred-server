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

#ifndef TESTDB_H_
#define TESTDB_H_

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <queue>
#include <sys/time.h>
#include <time.h>


#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_of.hpp>

#include "db_settings.h"
#include "log/logger.h"
#include "log/context.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"

#include "handle_general_args.h"
#include "handle_database_args.h"


#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
  using ::time;
}
#endif


unsigned exec_params_test()
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
            std::cout << "test string: " << std::string(res[0][3]) << std::endl;
        }//if res size
        else ret+=8;
        if (ret != 0 ) std::cerr << "exec_params_test ret: "<< ret << std::endl;


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

            std::cout << "test null: " << std::string(qres[0][5])
                      <<std::endl;

        }//if qres size
        else ret+=128;
        if (ret != 0 ) std::cerr << "exec_params_test ret: "<< ret << std::endl;


    }
    catch(std::exception& ex)
    {
        std::cerr << "exec_params_test exception reason: "<< ex.what() << std::endl;
        ret+=256;
        throw;
    }
    catch(...)
    {
        std::cerr << "exec_params_test exception returning"<< std::endl;
        ret+=512;
        if (ret != 0 ) std::cerr << "exec_params_test ret: "<< ret << std::endl;
    }

    return ret;
}

#endif // TESTDB_H_
