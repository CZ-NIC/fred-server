/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include "setup_server_decl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "fredlib/registrar.h"
#include "fredlib/invoicing/invoice.h"
#include "time_clock.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestInvoice)

const std::string server_name = "test-invoice";

BOOST_AUTO_TEST_CASE( test_inv )
{
    // setting up logger
    setup_logging(CfgArgs::instance());
    //db
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    //current time into char string
    std::string time_string(TimeStamp::microsec());
    std::string registrar_handle(std::string("REG-FRED_INV")+time_string);

    registrar->setHandle(registrar_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    registrar->save();

    unsigned long long registrar_inv_id = registrar->getId();

    std::string zone_registrar_credit_query =
            "select COALESCE(SUM(credit), 0) "
            " from invoice "
            " where zone = $1::bigint and registrarid =$2::bigint "
            " group by registrarid, zone ";
/*
    {
        //get registrar credit
        long registrar_credit = conn.exec_params(
                zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_inv_id)
                )[0][0];

        std::cout << "test_inv registrar model: id " <<  registrar->getId()
                << " credit0 " << registrar->getCredit(0) << " credit1 " << registrar->getCredit(1)
                << " registrar_credit " << registrar_credit
                << std::endl;
    }
*/
    //manager
    std::auto_ptr<Fred::Invoicing::Manager>
        invMan(Fred::Invoicing::Manager::create());

    try{
    invMan->insertInvoicePrefix(
             zone_cz_id//zoneId
            , 0//type
            , 2010//year
            , 50000//prefix
            );
    }catch(...){}

    try{
    invMan->insertInvoicePrefix(
            zone_cz_id//zoneId
            , 1//type
            , 2010//year
            , 60000//prefix
            );
    }catch(...){}

    unsigned long long invoiceid =
    invMan->createDepositInvoice(Database::Date(2010,12,31)//taxdate
            , zone_cz_id//zone
            , conn.exec(std::string("select id from registrar where handle='")+registrar_handle+"'")[0][0]//registrar
            , 20000);//price


    Fred::Registrar::Registrar::AutoPtr registrar_after = regMan->getRegistrarByHandle(registrar->getHandle());
/*
    {
        //get registrar credit
        long registrar_credit = conn.exec_params(
                zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_inv_id)
                )[0][0];

        std::cout << "test_inv registrar model: id " <<  registrar_after->getId()
              << " credit0 " << registrar_after->getCredit(0) << " credit1 " << registrar_after->getCredit(1)
              << " registrar_credit " << registrar_credit
              << std::endl;
    }
*/

    BOOST_CHECK_EQUAL(invoiceid != 0,true);
}

BOOST_AUTO_TEST_SUITE_END();//TestInv
