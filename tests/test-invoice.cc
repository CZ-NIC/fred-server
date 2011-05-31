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
#include <vector>
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
#include "cfg/handle_registry_args.h"

#include "fredlib/registrar.h"
#include "fredlib/invoicing/invoice.h"
#include "mailer_manager.h"
#include "time_clock.h"
#include "file_manager_client.h"
#include "fredlib/banking/bank_common.h"


//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>






BOOST_AUTO_TEST_SUITE(TestInvoice)

class InvoiceFixture {
public:
    InvoiceFixture() {
        // setting up logger
        setup_logging(CfgArgs::instance());
    }

};


BOOST_GLOBAL_FIXTURE(InvoiceFixture)

using namespace Fred::Invoicing;

const std::string server_name = "test-invoice";


struct registrar_credit_item
{
  int year;
  double credit_from_query;
  int vat;
  double koef;
  double price;
  Database::Date taxdate;
};

static std::string zone_registrar_credit_query (
        "select COALESCE(SUM(credit), 0) from invoice "
        " where zone = $1::bigint and registrarid =$2::bigint "
        " group by registrarid, zone ");

// TODO duplicity from invoice.cc - compilation problem
cent_amount get_price(const std::string &str)
{
  std::string t_str = boost::algorithm::trim_copy(str);//remove whitespaces
  std::size_t delimiter = t_str.find_first_of(".,");//find delimiter
  if (delimiter != std::string::npos)//if found
  {
        t_str.erase(delimiter,1);//erase delimiter
        if(t_str.length() > (delimiter + 2))//if there are more than two chars after delimiter
            t_str.erase(delimiter+2);//remove rest of the string
        else if (t_str.length() == (delimiter + 1))//if there is only one char after delimiter
            t_str+="0";//append second char after delimiter
        else if (t_str.length() == delimiter)//if there is no char after delimiter
          t_str+="00";//append first and second char after delimiter
   }

    long price = boost::lexical_cast<long>(t_str);//try convert
    LOGGER(PACKAGE).debug( boost::format("get_price from string[%1%] -> %2% hal") % str % price );
    return price;

    //return ::get_price(str.c_str());
}


bool check_std_exception_invoice_prefix(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("Missing invoice prefix")) != std::string::npos);
}

bool check_std_exception_out_of_range(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("Out of range")) != std::string::npos);
}

bool check_std_exception_archiveInvoices(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("archiveInvoices")) != std::string::npos);
}


BOOST_AUTO_TEST_CASE( getCreditByZone_noregistrar_nozone)
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    //current time into char string
    std::string time_string(TimeStamp::microsec());
    std::string registrar_handle(std::string("REG-FRED_NOINV")+time_string);//not created registrar

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    BOOST_CHECK((invMan->getCreditByZone(registrar_handle,zone_cz_id) == 0));//noregistrar
    BOOST_CHECK((invMan->getCreditByZone(registrar_handle,0) == 0));//no registrar no zone
}

BOOST_AUTO_TEST_CASE( insertInvoicePrefix_nozone )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    int year = boost::gregorian::day_clock::universal_day().year()+20;

    conn.exec_params("delete from invoice_prefix where year = $1::bigint", Database::query_param_list(year));

    BOOST_CHECK(
    (invMan->insertInvoicePrefix(
            0//no zoneId
            , 0//type
            , year//year
            , year*10000//prefix
            )==false));

    BOOST_CHECK_EXCEPTION(
    (invMan->insertInvoicePrefix(
            "com"//zone
            , 1//type
            , year//year
            , year*10000 + 1000//prefix
            ))
            , std::exception, check_std_exception_out_of_range);


}//insertInvoicePrefix


BOOST_AUTO_TEST_CASE( insertInvoicePrefix )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    
    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    int year = boost::gregorian::day_clock::universal_day().year()+20;
    
    conn.exec_params("delete from invoice_prefix where year = $1::bigint", Database::query_param_list(year));

    BOOST_CHECK(
    (invMan->insertInvoicePrefix(
            zone_cz_id//zoneId
            , 0//type
            , year//year
            , year*10000//prefix
            )));

    BOOST_CHECK((conn.exec_params(
        "select * from invoice_prefix where zone=$1::bigint and typ=0 and "
        " year=$2::integer and prefix=$3::bigint"
        , Database::query_param_list(zone_cz_id)(year)(year*10000)).size() == 1));

    BOOST_CHECK(
    (invMan->insertInvoicePrefix(
            "cz"//zone
            , 1//type
            , year//year
            , year*10000 + 1000//prefix
            )));

    BOOST_CHECK((conn.exec_params(
        "select * from invoice_prefix where zone=$1::bigint and typ=1 and "
        " year=$2::integer and prefix=$3::bigint"
        , Database::query_param_list(zone_cz_id)(year)(year*10000 + 1000)).size() == 1));

}//insertInvoicePrefix

BOOST_AUTO_TEST_CASE( createDepositInvoice_nozone )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    //current time into char string
    std::string time_string(TimeStamp::microsec());
    std::string registrar_novat_handle(std::string("REG-FRED_NOVAT_INV")+time_string);

    registrar->setHandle(registrar_novat_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    registrar->save();

    unsigned long long registrar_inv_id = registrar->getId();

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    {
        int year = boost::gregorian::day_clock::universal_day().year();

        Database::Date taxdate (year,1,1);
	
        BOOST_CHECK_EXCEPTION(
        invMan->createDepositInvoice(taxdate//taxdate
                , 0//no zone
                , registrar_inv_id//registrar
                , 20000)//price
    , std::exception, check_std_exception_invoice_prefix);
    
    }
}//BOOST_AUTO_TEST_CASE( createDepositInvoice_nozone )


BOOST_AUTO_TEST_CASE( createDepositInvoice_novat_noprefix )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    conn.exec("delete from invoice_prefix where year >= 1500 and year < 1505");

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar_novat = regMan->createRegistrar();

    //current time into char string
    std::string time_string(TimeStamp::microsec());
    std::string registrar_novat_handle(std::string("REG-FRED_NOVAT_INV")+time_string);

    registrar_novat->setHandle(registrar_novat_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar_novat->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar_novat->setVat(false);
    registrar_novat->save();

    unsigned long long registrar_novat_inv_id = registrar_novat->getId();

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    for (int year = 1500; year < 1505 ; ++year)
    {
        {
            Database::Date taxdate (year,1,1);
            BOOST_CHECK_EXCEPTION(
            invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_novat_inv_id//registrar
                    , 20000)//price
                    , std::exception
                    , check_std_exception_invoice_prefix);

        }
    }//for createDepositInvoice
}//BOOST_AUTO_TEST_CASE( createDepositInvoice_novat_noprefix )

BOOST_AUTO_TEST_CASE( createDepositInvoice )
{
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

    std::vector<registrar_credit_item> registrar_credit_vect;

    {//get registrar credit
        registrar_credit_item ci={1400,0.0,0,0.0, 0.0, Database::Date(1400,1,1)};

        Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_inv_id));
        if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = credit_res[0][0];

        Database::Date taxdate (1400,1,1);
        Database::Result vat_details = conn.exec_params(
            "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
            , Database::query_param_list(taxdate.get()));
        if(vat_details.size() == 1 && vat_details[0].size() == 2)
        {
            ci.vat = vat_details[0][0];
            ci.koef = vat_details[0][1];
	    ci.taxdate = taxdate;
        }

        registrar_credit_vect.push_back(ci);//save credit
    }

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    //insertInvoicePrefix
    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        try{
        invMan->insertInvoicePrefix(
                 zone_cz_id//zoneId
                , 0//type
                , year//year
                , year*10000//prefix
                );
        }catch(...){}

        try{
        invMan->insertInvoicePrefix(
                zone_cz_id//zoneId
                , 1//type
                , year//year
                , year*10000 + 1000//prefix
                );
        }catch(...){}

    }//for insertInvoicePrefix

    unsigned long long invoiceid = 0;

    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        {
            Database::Date taxdate (year,1,1);
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_inv_id//registrar
                    , 20000);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);

            //get registrar credit
            registrar_credit_item ci={year,0.0,0,0.0, 200.0, Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = credit_res[0][0];

            Database::Result vat_details = conn.exec_params(
                "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(taxdate.get()));
            if(vat_details.size() == 1 && vat_details[0].size() == 2)
            {
                ci.vat = vat_details[0][0];
                ci.koef = vat_details[0][1];
		ci.taxdate= taxdate;
            }

            registrar_credit_vect.push_back(ci);//save credit
        }

        {
            Database::Date taxdate (year,12,31);
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_inv_id//registrar
                    , 20000);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);

            //get registrar credit
            registrar_credit_item ci={year,0.0,0,0.0, 200.0, Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = credit_res[0][0];

            Database::Result vat_details = conn.exec_params(
                "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(taxdate.get()));
            if(vat_details.size() == 1 && vat_details[0].size() == 2)
            {
                ci.vat = vat_details[0][0];
                ci.koef = vat_details[0][1];
		ci.taxdate = taxdate;
            }

            registrar_credit_vect.push_back(ci);//save credit
        }
    }//for createDepositInvoice

    std::string dec_credit_query("select 0::numeric ");
    Database::QueryParams dec_credit_query_params;

    std::string test_credit_str;

    for (std::size_t i = 0 ; i < registrar_credit_vect.size(); ++i)
    {
        dec_credit_query_params.push_back(registrar_credit_vect.at(i).price);
        dec_credit_query_params.push_back(registrar_credit_vect.at(i).koef);
        dec_credit_query += std::string(" + $")
                + boost::lexical_cast<std::string>(dec_credit_query_params.size()-1)
                +  "::numeric - ( $"+ boost::lexical_cast<std::string>(dec_credit_query_params.size())
                +"::numeric * $"+ boost::lexical_cast<std::string>(dec_credit_query_params.size()-1)
                +"::numeric ) ";

        std::string fred_credit_str ( str(boost::format("%1$.2f") % registrar_credit_vect.at(i).credit_from_query));
        test_credit_str = std::string(conn.exec_params(
                std::string("select (")+dec_credit_query+")::numeric(10,2)"//round to 2 places
                , dec_credit_query_params)[0][0]);

        BOOST_CHECK(fred_credit_str.compare(test_credit_str)==0);

        if(fred_credit_str.compare(test_credit_str) != 0 )
        {
            std::cout << "taxdate: " << registrar_credit_vect.at(i).taxdate
		<< " year: " << registrar_credit_vect.at(i).year
                << " price: " << registrar_credit_vect.at(i).price
                << " credit: " <<  fred_credit_str
                << " test_credit: " << test_credit_str
                << " vat koef: " << registrar_credit_vect.at(i).koef
                << " vat : " << registrar_credit_vect.at(i).vat
                << std::endl;
        }//if not equal
    }

    Database::Money test_get_credit_by_zone = invMan->getCreditByZone(registrar_handle,zone_cz_id);
    BOOST_CHECK((test_get_credit_by_zone.to_string().compare(test_credit_str) == 0));
    
    if(test_get_credit_by_zone.to_string().compare(test_credit_str) != 0)
    {
        std::cout << "test_get_credit_by_zone: " << test_get_credit_by_zone 
	    << " test_credit_str: " << test_credit_str << std::endl;
    }

}//BOOST_AUTO_TEST_CASE( createDepositInvoice )

BOOST_AUTO_TEST_CASE( createDepositInvoice_novat )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar_novat = regMan->createRegistrar();

    //current time into char string
    std::string time_string(TimeStamp::microsec());
    std::string registrar_novat_handle(std::string("REG-FRED_NOVAT_INV")+time_string);

    registrar_novat->setHandle(registrar_novat_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar_novat->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar_novat->setVat(false);
    registrar_novat->save();

    unsigned long long registrar_novat_inv_id = registrar_novat->getId();

    std::vector<registrar_credit_item> registrar_novat_credit_vect;

    {//get registrar novat credit
        registrar_credit_item ci={1400,0.0,0,0.0, 0.0};

        Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_novat_inv_id));
        if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = credit_res[0][0];

        registrar_novat_credit_vect.push_back(ci);//save credit
    }

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    //insertInvoicePrefix
    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        try{
        invMan->insertInvoicePrefix(
                 zone_cz_id//zoneId
                , 0//type
                , year//year
                , year*10000//prefix
                );
        }catch(...){}

        try{
        invMan->insertInvoicePrefix(
                zone_cz_id//zoneId
                , 1//type
                , year//year
                , year*10000 + 1000//prefix
                );
        }catch(...){}

    }//for insertInvoicePrefix

    unsigned long long invoiceid = 0;

    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        {
            Database::Date taxdate (year,1,1);
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_novat_inv_id//registrar
                    , 20000);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);

            //get registrar credit
            registrar_credit_item ci={year,0.0,0,0.0, 200.0};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_novat_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = credit_res[0][0];

            registrar_novat_credit_vect.push_back(ci);//save credit
        }

        {
            Database::Date taxdate (year,12,31);
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_novat_inv_id//registrar
                    , 20000);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);

            //get registrar credit
            registrar_credit_item ci={year,0.0,0,0.0, 200.0};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_novat_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = credit_res[0][0];

            registrar_novat_credit_vect.push_back(ci);//save credit
        }
    }//for createDepositInvoice

    std::string dec_credit_query("select 0::numeric ");
    Database::QueryParams dec_credit_query_params;

    for (std::size_t i = 0 ; i < registrar_novat_credit_vect.size(); ++i)
    {
        dec_credit_query_params.push_back(registrar_novat_credit_vect.at(i).price);
        dec_credit_query_params.push_back(registrar_novat_credit_vect.at(i).koef);
        dec_credit_query += std::string(" + $")
                + boost::lexical_cast<std::string>(dec_credit_query_params.size()-1)
                +  "::numeric - ( $"+ boost::lexical_cast<std::string>(dec_credit_query_params.size())
                +"::numeric * $"+ boost::lexical_cast<std::string>(dec_credit_query_params.size()-1)
                +"::numeric ) ";

        std::string fred_credit_str ( str(boost::format("%1$.2f") % registrar_novat_credit_vect.at(i).credit_from_query));
        std::string test_credit_str(std::string(conn.exec_params(
                std::string("select (")+dec_credit_query+")::numeric(10,2)"//round to 2 places
                , dec_credit_query_params)[0][0]));

        BOOST_CHECK(fred_credit_str.compare(test_credit_str)==0);

        if(fred_credit_str.compare(test_credit_str) != 0 )
        {
            std::cout << " year: " << registrar_novat_credit_vect.at(i).year
                << " price: " << registrar_novat_credit_vect.at(i).price
                << " credit: " <<  fred_credit_str
                << " test_credit: " << test_credit_str
                << " vat koef: " << registrar_novat_credit_vect.at(i).koef
                << " vat : " << registrar_novat_credit_vect.at(i).vat
                << std::endl;
        }//if not equal
    }

}//BOOST_AUTO_TEST_CASE( createDepositInvoice_novat )

// all CHECKs were changed to REQUIRE
void test_ChargeDomainOperation(Fred::Invoicing::Manager *invMan, Database::Date exdate, unsigned reg_units,
        unsigned operation, Database::ID zone_id, Database::ID registrar_id)
{
    boost::format test_desc
        = boost::format("test domain operation charging - exdate: %1%, reg_units: %2%, operation: %3% , zone_id: %4%, registrar_id: %5%")
        % exdate % reg_units % operation % zone_id % registrar_id;

    Database::Connection conn = Database::Manager::acquire();

    std::string time_string(TimeStamp::microsec());
    std::string object_roid = std::string("ROID-TEST_INVOICE")  + time_string;

    cent_amount credit_before, credit_after;

    // CHECK CREDIT before
    Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                       , Database::query_param_list(zone_id)(registrar_id));
    // TODO add check
    if(credit_res.size() ==  1 && credit_res[0].size() == 1) credit_before = get_price((std::string)credit_res[0][0]);


    // some fake object in object registry
    // TODO create unique hanle for the object
    conn.exec_params("INSERT INTO object_registry (roid, name, crid ) VALUES ($1, 'object', $2::integer)",
             Database::query_param_list  (object_roid)
                                         (registrar_id) );

    Database::Result res_or = conn.exec_params("SELECT id FROM object_registry WHERE roid = $1 AND crid = $2::integer ",
            Database::query_param_list ( object_roid)
                                       ( registrar_id) );

    BOOST_REQUIRE_MESSAGE(res_or.size() > 0 , "Fake object_registry object wasn't found, cannot perform test");

    Database::ID object_id = res_or[0][0];


    if (operation == INVOICING_DomainCreate ) {
        BOOST_REQUIRE_MESSAGE (
                invMan->chargeDomainCreate(zone_id, registrar_id, object_id, exdate, reg_units ), test_desc);
    } else if (operation == INVOICING_DomainRenew) {
        BOOST_REQUIRE_MESSAGE (
                invMan->chargeDomainRenew(zone_id, registrar_id, object_id, exdate, reg_units ), test_desc);
    } else {
        BOOST_FAIL("Not implemented");
    }

    // REQUIRE CREDIT after
    Database::Result credit_res2 = conn.exec_params(zone_registrar_credit_query
                           , Database::query_param_list(zone_id)(registrar_id));
                   if(credit_res2.size() ==  1 && credit_res2[0].size() == 1) credit_after = get_price((std::string)credit_res2[0][0]);

    // now check the credit change
    Database::Result price_res = conn.exec_params("SELECT price , period FROM price_list where zone=$1::integer and operation=$2::integer "
                                "and valid_from<now() and ((valid_to is null) or (valid_to>now()))  order by valid_from desc limit 1 ",
                                Database::query_param_list  (zone_id)
                                                            (operation));

    BOOST_REQUIRE_MESSAGE(price_res.size() == 1, "Exactly one actual valid record must be present in table price_list");

    // TODO integer division, part of questions to specification
    double counted_price = get_price(price_res[0][0]) * (reg_units / (int)price_res[0][1]);

    boost::format credit_desc = boost::format(" credit before: %1%, credit_after: %2%, counted price: %3%")
        % credit_before % credit_after % counted_price;

    BOOST_REQUIRE_MESSAGE(counted_price == credit_before - credit_after, "Charged credit match: " + test_desc.str());

    Database::Result res_ior = conn.exec_params(
    "SELECT period, ExDate FROM invoice_object_registry WHERE objectid = $1::integer "
                                                    "AND registrarid = $2::integer "
                                                    "AND zone = $3::integer",
               Database::query_param_list ( object_id )
                                          ( registrar_id)
                                          ( zone_id));

    BOOST_REQUIRE_MESSAGE(res_ior.size() == 1, "Incorrect number of records in invoice_object_registry table ");

    BOOST_REQUIRE((unsigned)res_ior[0][0] == reg_units);
    BOOST_REQUIRE(res_ior[0][1] == exdate);
}

// Fred::Registrar::Registrar::AutoPtr createTestRegistrar(const std::string &handle_base)
Database::ID createTestRegistrar(const std::string &handle_base)
{

    std::string time_string(TimeStamp::microsec());
    std::string registrar_handle = std::string("REG-FRED_2INVNEED") + time_string;

    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    registrar->setHandle(registrar_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    registrar->save();

    return registrar->getId();
}

void create2Invoices(Fred::Invoicing::Manager *man, Database::Date taxdate, Database::ID zone_cz_id, Database::ID reg_id, cent_amount amount)
{
   Database::ID invoiceid = man->createDepositInvoice(taxdate //taxdate
                   , zone_cz_id//zone
                   , reg_id//registrar
                   , amount);//price
   BOOST_CHECK_EQUAL(invoiceid != 0,true);
   // add credit for new registrar
   Database::ID invoiceid2 = man->createDepositInvoice(taxdate //taxdate
                   , zone_cz_id//zone
                   , reg_id//registrar
                   , amount);//price
   BOOST_CHECK_EQUAL(invoiceid2 != 0,true);
}


BOOST_AUTO_TEST_CASE( chargeDomainCreateNoCredit )
{

    std::string time_string(TimeStamp::microsec());
    std::string registrar_handle = std::string("REG-FRED_INV") + time_string;
    std::string object_roid = std::string("ROID-TEST_INVOICE")  + time_string;

    //db
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    registrar->setHandle(registrar_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    registrar->save();

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    // some fake object in object registry
    // TODO create unique handle for the object
    conn.exec_params("INSERT INTO object_registry (roid, name, crid ) VALUES ($1, 'object', $2::integer)",
             Database::query_param_list  (object_roid)
                                         (registrar->getId()) );

    Database::Result res_or = conn.exec_params("SELECT id FROM object_registry WHERE roid = $1 AND crid = $2::integer ",
            Database::query_param_list ( object_roid )
                                       ( registrar->getId()) );

    BOOST_REQUIRE_MESSAGE(res_or.size() > 0 , "Fake object_registry object wasn't found, cannot perform test");

    Database::ID object_id = res_or[0][0];

    // TODO this should check price_list - if operation price is really 0
    // TODO change date
    BOOST_CHECK(!invMan->chargeDomainCreate(zone_cz_id, registrar->getId(), object_id, Database::Date(2012, 1, 1), 12 ));

}

// not thread safe - database data can change in the meantime
BOOST_AUTO_TEST_CASE( chargeDomainCreate )
{

    //db
    Database::Connection conn = Database::Manager::acquire();

    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

/*
    std::string registrar_handle =          //vat
            common_init( "REG-FRED_INV", true );
    */

    std::string time_string(TimeStamp::microsec());
    std::string registrar_handle = std::string("REG-FRED_INV") + time_string;


    // create registrar
    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    registrar->setHandle(registrar_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    registrar->save();


    cent_amount amount = 20000 * 100;
    unsigned act_year = boost::gregorian::day_clock::universal_day().year();
    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());



    // add credit for new registrar
    Database::Date taxdate (act_year,1,1);
    Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_cz_id//zone
                    , registrar->getId()//registrar
                    , amount);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    // add credit for new registrar
    Database::ID invoiceid2 = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_enum_id//zone
                    , registrar->getId()//registrar
                    , amount);//price
    BOOST_CHECK_EQUAL(invoiceid2 != 0,true);


    int reg_units = 24;


    Database::Date exdate(act_year + 1, 1, 1);
    Database::Date exdate2(act_year + 5, 4, 30);

    test_ChargeDomainOperation(invMan.get(), exdate, reg_units, INVOICING_DomainCreate, zone_cz_id, registrar->getId());
    test_ChargeDomainOperation(invMan.get(), exdate, reg_units, INVOICING_DomainCreate, zone_enum_id, registrar->getId());
    test_ChargeDomainOperation(invMan.get(), exdate, 19, INVOICING_DomainCreate, zone_cz_id, registrar->getId());
    test_ChargeDomainOperation(invMan.get(), exdate, 19, INVOICING_DomainCreate, zone_enum_id, registrar->getId());

    test_ChargeDomainOperation(invMan.get(), exdate2, reg_units, INVOICING_DomainCreate, zone_cz_id, registrar->getId());
    test_ChargeDomainOperation(invMan.get(), exdate2, reg_units, INVOICING_DomainCreate, zone_enum_id, registrar->getId());
    test_ChargeDomainOperation(invMan.get(), exdate2, 19, INVOICING_DomainCreate, zone_cz_id, registrar->getId());
    test_ChargeDomainOperation(invMan.get(), exdate2, 19, INVOICING_DomainCreate, zone_enum_id, registrar->getId());

    test_ChargeDomainOperation(invMan.get(), exdate, reg_units, INVOICING_DomainRenew, zone_cz_id, registrar->getId());
   test_ChargeDomainOperation(invMan.get(), exdate, reg_units, INVOICING_DomainRenew, zone_enum_id, registrar->getId());
   test_ChargeDomainOperation(invMan.get(), exdate, 19, INVOICING_DomainRenew, zone_cz_id, registrar->getId());
   test_ChargeDomainOperation(invMan.get(), exdate, 19, INVOICING_DomainRenew, zone_enum_id, registrar->getId());

   test_ChargeDomainOperation(invMan.get(), exdate2, reg_units, INVOICING_DomainRenew, zone_cz_id, registrar->getId());
   test_ChargeDomainOperation(invMan.get(), exdate2, reg_units, INVOICING_DomainRenew, zone_enum_id, registrar->getId());
   test_ChargeDomainOperation(invMan.get(), exdate2, 19, INVOICING_DomainRenew, zone_cz_id, registrar->getId());
   test_ChargeDomainOperation(invMan.get(), exdate2, 19, INVOICING_DomainRenew, zone_enum_id, registrar->getId());

}

BOOST_AUTO_TEST_CASE(chargeDomainCreate2Invoices )
{

    //db
    Database::Connection conn = Database::Manager::acquire();
    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

// registrar
    Database::ID regid = createTestRegistrar("REG-FRED_2INVNEED");


    // get operation price so we can create invoices which will split the operation
    // TODO more detailed
    Database::Result res_price =
            conn.exec_params("SELECT price FROM price_list WHERE operation = $1::integer AND zone = $2::integer AND valid_from < now() ORDER BY valid_from DESC LIMIT 1",
            Database::query_param_list ( (unsigned)INVOICING_DomainRenew )
                                        ( zone_cz_id ));

    cent_amount renew_price = get_price (res_price[0][0]);

    // price for invoices so that 2 are not sufficient
    // cent_amount amount = op_price / 3;

    // price for invoices so that 2 are needed.
    // TODO hardcoded - change
    cent_amount amount = ( renew_price * 1.20) / 2;
    unsigned act_year = boost::gregorian::day_clock::universal_day().year();


    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());
    // add credit - 2 invoices for the same zone:
    Database::Date taxdate (act_year,1,1);

    create2Invoices(invMan.get(), taxdate, zone_cz_id, regid, amount);

    Database::Date exdate2(act_year + 5, 4, 30);
    test_ChargeDomainOperation(invMan.get(), exdate2, 12, INVOICING_DomainRenew, zone_cz_id, regid);

}

BOOST_AUTO_TEST_CASE( archiveInvoices_no_init )
{

    std::auto_ptr<Fred::Invoicing::Manager> invMan(
        Fred::Invoicing::Manager::create());

    BOOST_CHECK_EXCEPTION(
    invMan->archiveInvoices(false)
    , std::exception, check_std_exception_archiveInvoices);
}

BOOST_AUTO_TEST_CASE( archiveInvoices )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    //corba config
       FakedArgs fa = CfgArgs::instance()->fa;

    //conf pointers
    HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleRegistryArgs>();
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);


    std::string corbaNS =ns_args_ptr->nameservice_host
            + ":"
            + boost::lexical_cast<std::string>(ns_args_ptr->nameservice_port);

    std::auto_ptr<Fred::Document::Manager> docMan(
              Fred::Document::Manager::create(
                  registry_args_ptr->docgen_path
                  , registry_args_ptr->docgen_template_path
                  , registry_args_ptr->fileclient_path
                  , corbaNS)
              );

    //manager init
    MailerManager mailMan(CorbaContainer::get_instance()->getNS());
    std::auto_ptr<Fred::Invoicing::Manager> invMan(
        Fred::Invoicing::Manager::create(
        docMan.get(),&mailMan));

    FileManagerClient fm_client(
             CorbaContainer::get_instance()->getNS());

    //call archive invoices and get processed invoice ids
    InvoiceIdVect inv_id_vect = invMan->archiveInvoices(false);

    //read processed invoices query
    std::string inv_query(
        "select zone, crdate, taxdate, prefix , registrarid " // 0 - 4
        ", credit, price, vat, total, totalvat, prefix_type, file , filexml " // 5 - 12
        " from invoice where ");

  /* table invoice
  id serial NOT NULL, -- unique automatically generated identifier
  "zone" integer, -- reference to zone
  crdate timestamp without time zone NOT NULL DEFAULT now(), -- date and time of invoice creation
  taxdate date NOT NULL, -- date of taxable fulfilment (when payment cames by advance FA)
  prefix bigint NOT NULL, -- 9 placed number of invoice from invoice_prefix.prefix counted via TaxDate
  registrarid integer NOT NULL, -- link to registrar
  credit numeric(10,2) DEFAULT 0.0, -- credit from which is taken till zero, if it is NULL it is normal invoice
  price numeric(10,2) NOT NULL DEFAULT 0.0, -- invoice high with tax
  vat integer NOT NULL DEFAULT 19, -- VAT hight from account
  total numeric(10,2) NOT NULL DEFAULT 0.0, -- amount without tax
  totalvat numeric(10,2) NOT NULL DEFAULT 0.0, -- tax paid
  prefix_type integer NOT NULL, -- invoice type (from which year is and which type is according to prefix)
  file integer, -- link to generated PDF file, it can be NULL till file is generated
  filexml integer, -- link to generated XML file, it can be NULL till file is generated
 *
 * */

    bool inv_query_first_id = true;

    Database::QueryParams inv_query_params;

    //TODO: parse xml from filemanager , check with data in db

    //select invoices by export invoice id"
    for (InvoiceIdVect::iterator i = inv_id_vect.begin(); i != inv_id_vect.end(); ++i )
    {
        //  std::cout << *i << " " //id

        if (inv_query_first_id)
        {//first id
            inv_query_first_id = false;
            //add first id
            inv_query_params.push_back(*i);
            inv_query += " id = $" + boost::lexical_cast<std::string>(inv_query_params.size()) +"::bigint ";
        }
        else
        {//next id
            inv_query_params.push_back(*i);
            inv_query += "or id = $" + boost::lexical_cast<std::string>(inv_query_params.size()) +"::bigint ";
        }
    }//for id
    //std::cout << std::endl;

    Database::Result invoice_res= conn.exec_params(inv_query, inv_query_params);

    for (std::size_t i = 0 ; i < invoice_res.size(); ++i)//check invoice data
    {
        unsigned long long  file_id = invoice_res[i][12];
        std::vector<char> out_buffer;
        fm_client.download(file_id, out_buffer);

        std::string xml(out_buffer.begin(), out_buffer.end());

        try
        {
            Fred::Banking::XMLparser parser;
            if (!parser.parse(xml)) throw std::runtime_error("parser error");

            Fred::Banking::XMLnode root = parser.getRootNode();
            if (root.getName().compare("invoice") != 0) throw std::runtime_error("root xml element name is not \"invoice\"");

            Fred::Banking::XMLnode delivery = root.getChild("delivery");
            if (delivery.getName().compare("delivery") != 0) throw std::runtime_error("xml element name is not \"delivery\"");

            Fred::Banking::XMLnode vat_rates = delivery.getChild("vat_rates");
            if (vat_rates.getName().compare("vat_rates") != 0) throw std::runtime_error("xml element name is not \"vat_rates\"");

            if(vat_rates.hasChild("entry"))
            {
                Fred::Banking::XMLnode entry = vat_rates.getChild("entry");
                if (entry.getName().compare("entry") != 0) throw std::runtime_error("xml element name is not \"entry\"");

                if(
                        (entry.getChild("vatperc").getValue().compare(std::string(invoice_res[i][7])//invoice vat
                                                    )!=0)
                        || (entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][8])//invoice total
                        )!=0)
                        || (entry.getChild("vat").getValue().compare(std::string(invoice_res[i][9])//invoice totalvat
                        )!=0)
                        || (entry.getChild("total").getValue().compare(std::string(invoice_res[i][6])//invoice price
                        )!=0)
                )
                std::cout << "archiveInvoices debug entry "
                    << "\n"
                    << " xml entry vatperc: " << entry.getChild("vatperc").getValue()
                    << " xml entry basetax: " << entry.getChild("basetax").getValue()
                    << " xml entry vat: " << entry.getChild("vat").getValue()
                    << " xml entry total: " << entry.getChild("total").getValue()
                    << "\n"
                    << " db crdate: " << std::string( invoice_res[i][1])
                    << " db taxdate: " << std::string( invoice_res[i][2])
                    << " db prefix: " << std::string( invoice_res[i][3])
                    << " db credit: " << std::string(invoice_res[i][5])
                    << " db price: " << std::string(invoice_res[i][6])
                    << " db vat: " << std::string(invoice_res[i][7])
                    << " db total: " << std::string( invoice_res[i][8])
                    << " db totalvat: " << std::string( invoice_res[i][9])
                    << "\n"
                    << " xml file id: " << std::string(invoice_res[i][12])
                    << " file_id: " << file_id
                    << " out_buffer.size(): " << out_buffer.size()
                    << " xml.size(): " << xml.size()
                    << " \nxml: " << xml
                    << std::endl;

                BOOST_CHECK(entry.getChild("vatperc").getValue().compare(std::string(invoice_res[i][7])//invoice vat
                            )==0);
                BOOST_CHECK(entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][8])//invoice total
                            )==0);
                BOOST_CHECK(entry.getChild("vat").getValue().compare(std::string(invoice_res[i][9])//invoice totalvat
                            )==0);
                BOOST_CHECK(entry.getChild("total").getValue().compare(std::string(invoice_res[i][6])//invoice price
                            )==0);
            }

            Fred::Banking::XMLnode sumarize = delivery.getChild("sumarize");
            if (sumarize.getName().compare("sumarize") != 0) throw std::runtime_error("xml element name is not \"sumarize\"");



            if (sumarize.getChild("total").getValue().compare(std::string(invoice_res[i][6])//invoice price
            )!=0)
            {
                std::cout << "archiveInvoices debug total "
                    << "\n"
                    << " db crdate: " << std::string( invoice_res[i][1])
                    << " db taxdate: " << std::string( invoice_res[i][2])
                    << " db prefix: " << std::string( invoice_res[i][3])
                    << " db credit: " << std::string(invoice_res[i][5])
                    << " db price: " << std::string(invoice_res[i][6])
                    << " db vat: " << std::string(invoice_res[i][7])
                    << " db total: " << std::string( invoice_res[i][8])
                    << " db totalvat: " << std::string( invoice_res[i][9])

                    << " xml sumarize total: " << sumarize.getChild("total").getValue()

                    << " xml file id: " << std::string(invoice_res[i][12])
                    << " file_id: " << file_id
                    << " out_buffer.size(): " << out_buffer.size()
                    << " xml.size(): " << xml.size()
                    << " \nxml: " << xml
                    << std::endl;
            }

            BOOST_CHECK(sumarize.getChild("total").getValue().compare(std::string(invoice_res[i][6])//invoice price
                            )==0);



        }
        catch(const std::exception& ex)
        {
            std::cout << "archiveInvoices debug exception: " << ex.what()
                << "\nCredit: " << std::string(invoice_res[i][5])
                << " xml file id: " << std::string(invoice_res[i][12])
                << " file_id: " << file_id
                << " out_buffer.size(): " << out_buffer.size()
                << " xml.size(): " << xml.size()
                << " \nxml: " << xml
                << std::endl;
        }
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestInv

