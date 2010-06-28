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
        std::string query = "select 1 as id, $1::bigint as data1, $2::bigint as data2";

        std::vector<std::string> params = boost::assign::list_of("2")("3");


        Database::Result res = conn.exec_params( query, params );
        if ((res.size() > 0) && (res[0].size() == 3))
        {
        	//check data
            if(1 != static_cast<unsigned long long>(res[0][0]) ) ret+=1;
            if(2 != static_cast<unsigned long long>(res[0][1])) ret+=2;
            if(3 != static_cast<unsigned long long>(res[0][2])) ret+=4;
        }//if res size
        else ret+=8;
        if (ret != 0 ) std::cerr << "exec_params_test ret: "<< ret << std::endl;

    }
    catch(std::exception& ex)
    {
        std::cerr << "exec_params_test exception reason: "<< ex.what() << std::endl;
        ret+=16;
        throw;
    }
    catch(...)
    {
        std::cerr << "exec_params_test exception returning"<< std::endl;
        ret+=32;
        if (ret != 0 ) std::cerr << "exec_params_test ret: "<< ret << std::endl;
    }

    return ret;
}

#endif // TESTDB_H_
