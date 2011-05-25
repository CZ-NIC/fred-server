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


//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestInvoice)


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

BOOST_AUTO_TEST_CASE( archiveInvoices_no_init )
{
    // setting up logger
    setup_logging(CfgArgs::instance());

    std::auto_ptr<Fred::Invoicing::Manager> invMan(
        Fred::Invoicing::Manager::create());

    BOOST_CHECK_EXCEPTION(
    invMan->archiveInvoices(false)
    , std::exception, check_std_exception_archiveInvoices);
}

BOOST_AUTO_TEST_CASE( archiveInvoices )
{
    // setting up logger
    setup_logging(CfgArgs::instance());

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

    MailerManager mailMan(CorbaContainer::get_instance()->getNS());
    std::auto_ptr<Fred::Invoicing::Manager> invMan(
        Fred::Invoicing::Manager::create(
        docMan.get(),&mailMan));

    invMan->archiveInvoices(false);
}

BOOST_AUTO_TEST_CASE( getCreditByZone_noregistrar_nozone)
{

    // setting up logger
    setup_logging(CfgArgs::instance());
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
    // setting up logger
    setup_logging(CfgArgs::instance());
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
    // setting up logger
    setup_logging(CfgArgs::instance());
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
    // setting up logger
    setup_logging(CfgArgs::instance());
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
    // setting up logger
    setup_logging(CfgArgs::instance());
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
    // setting up logger
    setup_logging(CfgArgs::instance());
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

void test_ChargeDomainOperation(Fred::Invoicing::Manager *invMan, Database::Date exdate, unsigned reg_units,
        unsigned operation, Database::ID zone_id, Database::ID registrar_id)
{
    BOOST_TEST_MESSAGE( boost::format("test domain operation charging - exdate: %1%, reg_units: %2%, operation: %3% , zone_id: %4%, registrar_id: %5%")
                % exdate % reg_units % operation % zone_id % registrar_id );

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
        BOOST_CHECK(invMan->chargeDomainCreate(zone_id, registrar_id, object_id, exdate, reg_units ));
    } else if (operation == INVOICING_DomainRenew) {
        BOOST_CHECK(invMan->chargeDomainRenew(zone_id, registrar_id, object_id, exdate, reg_units ));
    } else {
        BOOST_FAIL("Not implemented");
    }

    // CHECK CREDIT after
    Database::Result credit_res2 = conn.exec_params(zone_registrar_credit_query
                           , Database::query_param_list(zone_id)(registrar_id));
                   if(credit_res2.size() ==  1 && credit_res2[0].size() == 1) credit_after = get_price((std::string)credit_res2[0][0]);

    // now check the credit change
    Database::Result price_res = conn.exec_params("SELECT price , period FROM price_list where zone=$1::integer and operation=$2::integer "
                                "and valid_from<now() and ((valid_to is null) or (valid_to>now()))  order by valid_from desc limit 1 ",
                                Database::query_param_list  (zone_id)
                                                            (operation));

    BOOST_CHECK_MESSAGE(price_res.size() == 1, "Exactly one actual valid record must be present in table price_list");

    // TODO integer division, part of questions to specification
    double counted_price = get_price(price_res[0][0]) * (reg_units / (int)price_res[0][1]);

    std::cout << "credit before: " << credit_before << ", credit_after: " << credit_after <<
            ", counted price: " << counted_price << std::endl;
    BOOST_CHECK_MESSAGE(counted_price == credit_before - credit_after, "Charged credit does not match");


    Database::Result res_ior = conn.exec_params(
    "SELECT period, ExDate FROM invoice_object_registry WHERE objectid = $1::integer "
                                                    "AND registrarid = $2::integer "
                                                    "AND zone = $3::integer",
               Database::query_param_list ( object_id )
                                          ( registrar_id)
                                          ( zone_id));

    BOOST_CHECK_MESSAGE(res_ior.size() == 1, "Incorrect number of records in invoice_object_registry table ");

    BOOST_CHECK((unsigned)res_ior[0][0] == reg_units);
    BOOST_CHECK(res_ior[0][1] == exdate);
}

BOOST_AUTO_TEST_CASE( chargeDomainCreateNoCredit )
{
    // setting up logger
    setup_logging(CfgArgs::instance());

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
    // setting up logger
    setup_logging(CfgArgs::instance());

    std::string time_string(TimeStamp::microsec());
    std::string registrar_handle = std::string("REG-FRED_INV") + time_string;

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    //db
    Database::Connection conn = Database::Manager::acquire();

    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    Fred::Registrar::Manager::AutoPtr regMan
             = Fred::Registrar::Manager::create(DBSharedPtr());
    Fred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();

    registrar->setHandle(registrar_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    registrar->save();

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    // add credit for new registrar
    Database::Date taxdate (act_year,1,1);
    Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_cz_id//zone
                    , registrar->getId()//registrar
                    , 2000000);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    // add credit for new registrar
    Database::ID invoiceid2 = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_enum_id//zone
                    , registrar->getId()//registrar
                    , 1000000);//price
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



BOOST_AUTO_TEST_SUITE_END();//TestInv

