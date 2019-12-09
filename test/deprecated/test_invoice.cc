/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#include "test/deprecated/test_common_registry.hh"

#include "src/bin/corba/file_manager_client.hh"
#include "src/bin/corba/Admin.hh"

#include "src/util/setup_server_decl.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/types/money.hh"
#include "src/util/time_clock.hh"

#include "src/deprecated/libfred/credit.hh"
#include "src/deprecated/libfred/banking/bank_common.hh"
#include "src/deprecated/libfred/exceptions.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

#include "test/setup/test_common_threaded.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "test/deprecated/test_invoice_common.hh"
#include "src/util/cfg/config_handler_decl.hh"

#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

BOOST_AUTO_TEST_SUITE(TestInvoice)

using namespace ::LibFred::Invoicing;

struct registrar_credit_item
{
  int year;
  std::string credit_from_query;
  int vat;
  std::string koef;
  std::string price;
  Database::Date taxdate;
};

void create2Invoices(::LibFred::Invoicing::Manager *man, Database::Date taxdate, Database::ID zone_id, Database::ID reg_id, Money amount)
{
    Money out_credit;
   Database::ID invoiceid = man->createDepositInvoice(taxdate //taxdate
                   , zone_id//zone
                   , reg_id//registrar
                   , amount
                   , boost::posix_time::ptime(taxdate)
                   , out_credit);//price
   BOOST_CHECK_EQUAL(invoiceid != 0,true);
   ::LibFred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid);

   Database::ID invoiceid2 = man->createDepositInvoice(taxdate //taxdate
                   , zone_id//zone
                   , reg_id//registrar
                   , amount
                   , boost::posix_time::ptime(taxdate)
                   , out_credit);//price
   BOOST_CHECK_EQUAL(invoiceid2 != 0,true);
   ::LibFred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid2);
}

unsigned getAccountNumber(unsigned zone_id, const std::string &bank_code, const std::string &account_number)
{
    Database::Connection conn = Database::Manager::acquire();

    Result res = conn.exec_params("SELECT id FROM bank_account WHERE zone=$1::integer AND bank_code=$2::text AND account_number=$3::text ",
            Database::query_param_list(zone_id)
                                  (bank_code)
                                  (account_number)
    );

    return res[0][0];
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
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    BOOST_CHECK((invMan->getCreditByZone(registrar_handle,zone_cz_id).compare("0.00") == 0));//noregistrar
    BOOST_CHECK((invMan->getCreditByZone(registrar_handle,0).compare("0.00") == 0));//no registrar no zone
}

BOOST_AUTO_TEST_CASE( insertInvoicePrefix_nozone )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

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
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

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
        "select * from invoice_prefix where zone_id=$1::bigint and typ=0 and "
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
        "select * from invoice_prefix where zone_id=$1::bigint and typ=1 and "
        " year=$2::integer and prefix=$3::bigint"
        , Database::query_param_list(zone_cz_id)(year)(year*10000 + 1000)).size() == 1));

}//insertInvoicePrefix

BOOST_AUTO_TEST_CASE( createDepositInvoice_nozone )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long registrar_inv_id = createTestRegistrar();

    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    {
        int year = boost::gregorian::day_clock::universal_day().year();

        Database::Date taxdate (year,1,1);
        Money out_credit;
        BOOST_CHECK_EXCEPTION(
        invMan->createDepositInvoice(taxdate//taxdate
                , 0//no zone
                , registrar_inv_id//registrar
                , Money("200.00")
                , boost::posix_time::ptime(taxdate)
                , out_credit)//price
    , std::exception, check_std_exception_invoice_prefix);

    }
}//BOOST_AUTO_TEST_CASE( createDepositInvoice_nozone )

BOOST_AUTO_TEST_CASE( createDepositInvoice_novat_noprefix )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    conn.exec("delete from invoice_prefix where year >= 1500 and year < 1505");

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    unsigned long long registrar_novat_inv_id = createTestRegistrar();

    conn.exec_params("UPDATE registrar SET vat=false WHERE id = $1::bigint"
                , Database::query_param_list(registrar_novat_inv_id));

    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    for (int year = 1500; year < 1505 ; ++year)
    {
        {
            Database::Date taxdate (year,1,1);
            Money out_credit;
            BOOST_CHECK_EXCEPTION(
            invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_novat_inv_id//registrar
                    , Money("200.00")
                    , boost::posix_time::ptime(taxdate), out_credit)//price
                    , std::exception
                    , check_std_exception_invoice_prefix);

        }
    }//for createDepositInvoice
}//BOOST_AUTO_TEST_CASE( createDepositInvoice_novat_noprefix )

BOOST_AUTO_TEST_CASE( createDepositInvoice )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    init_corba_container();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    unsigned long long registrar_inv_id = registrar->getId();
    std::string registrar_handle = registrar->getHandle();

    LOGGER.debug ( boost::format("createDepositInvoice test registrar_id %1%")
               % registrar_inv_id);


    std::vector<registrar_credit_item> registrar_credit_vect;

    {//get registrar credit
        registrar_credit_item ci={1400,std::string("0.00"),0,std::string("0.00"), std::string("0.00"), Database::Date(1400,1,1)};

        Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_inv_id));
        if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

        Database::Date taxdate (1400,1,1);
        Database::Result vat_details = conn.exec_params(
            "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
            , Database::query_param_list(taxdate.get()));
        if(vat_details.size() == 1 && vat_details[0].size() == 2)
        {
            ci.vat = vat_details[0][0];
            ci.koef = std::string(vat_details[0][1]);
            ci.taxdate = taxdate;
        }

        registrar_credit_vect.push_back(ci);//save credit
    }

    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    try_insert_invoice_prefix();

    unsigned long long invoiceid = 0;

    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        {
            Database::Date taxdate (year,1,1);
            Money out_credit;
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_inv_id//registrar
                    , Money("200.00")
                    , boost::posix_time::ptime(taxdate)
            , out_credit);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);
            ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

            //get registrar credit
            registrar_credit_item ci={year,std::string("0.00"),0,std::string("0.00"), std::string("200.00"), Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

            Database::Result vat_details = conn.exec_params(
                "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(taxdate.get()));
            if(vat_details.size() == 1 && vat_details[0].size() == 2)
            {
                ci.vat = vat_details[0][0];
                ci.koef = std::string(vat_details[0][1]);
                ci.taxdate= taxdate;
            }

            registrar_credit_vect.push_back(ci);//save credit
        }

        {
            Database::Date taxdate (year,12,31);
            Money out_credit;
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_inv_id//registrar
                    , Money("21474836.47")
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);
            ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

            //get registrar credit
            registrar_credit_item ci={year,std::string("0.00"),0,std::string("0.00"), std::string("21474836.47"), Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

            Database::Result vat_details = conn.exec_params(
                "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(taxdate.get()));
            if(vat_details.size() == 1 && vat_details[0].size() == 2)
            {
                ci.vat = vat_details[0][0];
                ci.koef = std::string(vat_details[0][1]);
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
                +"::numeric )::numeric(10,2) ";//round vat to 2 places

        //std::string fred_credit_str ( str(boost::format("%1$.2f") % registrar_credit_vect.at(i).credit_from_query));
        std::string fred_credit_str (registrar_credit_vect.at(i).credit_from_query);

        test_credit_str = std::string(conn.exec_params(
                dec_credit_query
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


            std::cout << "\ndec_credit_query: " << dec_credit_query <<  std::endl;
            std::cout << "\ndec_credit_query_params: ";

            std::size_t i_num=0;
            for (Database::QueryParams::iterator it = dec_credit_query_params.begin(); it != dec_credit_query_params.end(); ++it )
            {
                ++i_num;
                std::cout << " $" << i_num << ": " << it->print_buffer();
            }
            std::cout << std::endl;

        }//if not equal
    }

    std::string test_get_credit_by_zone = invMan->getCreditByZone(registrar_handle,zone_cz_id);

    if(test_get_credit_by_zone.compare(test_credit_str) != 0)
    {
        /*
        std::cout << "test_get_credit_by_zone: " << test_get_credit_by_zone
	    << " test_credit_str: " << test_credit_str << std::endl;
	    */
    }
    BOOST_CHECK((test_get_credit_by_zone.compare(test_credit_str) == 0));

    //try to resolve
    ccReg::Admin_var admin_ref;
    admin_ref = ccReg::Admin::_narrow(
            CorbaContainer::get_instance()->nsresolve("Admin"));

    CORBA::String_var registrar_handle_cpy = CORBA::string_dup(registrar_handle.c_str());
    CORBA::String_var corba_credit = admin_ref->getCreditByZone(registrar_handle_cpy,zone_cz_id);

    if(std::string(corba_credit.in()).compare(test_credit_str) != 0)
    {
        /*
        std::cout << "corba_credit: " << std::string(corba_credit.in())
        << " test_credit_str: " << test_credit_str << std::endl;
        */
    }
    BOOST_CHECK((std::string(corba_credit.in()).compare(test_credit_str) == 0));
}//BOOST_AUTO_TEST_CASE( createDepositInvoice )

BOOST_AUTO_TEST_CASE( createDepositInvoice_credit_note )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    init_corba_container();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::string registrar_handle = registrar->getHandle();
    unsigned long long registrar_inv_id = registrar->getId();

    LOGGER.debug ( boost::format("createDepositInvoice test registrar_handle %1% registrar_inv_id %2%")
               % registrar_handle % registrar_inv_id);


    std::vector<registrar_credit_item> registrar_credit_vect;

    {//get registrar credit
        registrar_credit_item ci={1400,std::string("0.00"),0,std::string("0.00"), std::string("0.00"), Database::Date(1400,1,1)};

        Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_inv_id));
        if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

        Database::Date taxdate (1400,1,1);
        Database::Result vat_details = conn.exec_params(
            "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
            , Database::query_param_list(taxdate.get()));
        if(vat_details.size() == 1 && vat_details[0].size() == 2)
        {
            ci.vat = vat_details[0][0];
            ci.koef = std::string(vat_details[0][1]);
            ci.taxdate = taxdate;
        }

        registrar_credit_vect.push_back(ci);//save credit
    }

    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    try_insert_invoice_prefix();

    unsigned long long invoiceid = 0;

    unsigned long long credit_note_id = 0;

    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        {
            Database::Date taxdate (year,1,1);
            Money out_credit;
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_inv_id//registrar
                    , Money("200.00")
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);
            ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

            //get registrar credit
            registrar_credit_item ci={year,std::string("0.00"),0,std::string("0.00"), std::string("200.00"), Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

            Database::Result vat_details = conn.exec_params(
                "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(taxdate.get()));
            if(vat_details.size() == 1 && vat_details[0].size() == 2)
            {
                ci.vat = vat_details[0][0];
                ci.koef = std::string(vat_details[0][1]);
                ci.taxdate= taxdate;
            }

            registrar_credit_vect.push_back(ci);//save credit
        }

        {
	    //credit note
            Database::Date taxdate (year,1,1);
            Money out_credit;
            credit_note_id = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_inv_id//registrar
                    , Money("-200.00")
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
            BOOST_CHECK_EQUAL(credit_note_id != 0,true);
            ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, credit_note_id);

            //get registrar credit
            registrar_credit_item ci={year,std::string("0.00"),0,std::string("0.00"), std::string("-200.00"), Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

            Database::Result vat_details = conn.exec_params(
                "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(taxdate.get()));
            if(vat_details.size() == 1 && vat_details[0].size() == 2)
            {
                ci.vat = vat_details[0][0];
                ci.koef = std::string(vat_details[0][1]);
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
                +"::numeric )::numeric(10,2) ";//round vat to 2 places

        //std::string fred_credit_str ( str(boost::format("%1$.2f") % registrar_credit_vect.at(i).credit_from_query));
        std::string fred_credit_str (registrar_credit_vect.at(i).credit_from_query);

        test_credit_str = std::string(conn.exec_params(
                dec_credit_query
                , dec_credit_query_params)[0][0]);

        BOOST_CHECK(fred_credit_str.compare(test_credit_str)==0);

        if(fred_credit_str.compare(test_credit_str) != 0 )
        {
            /*
            std::cout << "taxdate: " << registrar_credit_vect.at(i).taxdate
                << " year: " << registrar_credit_vect.at(i).year
                << " price: " << registrar_credit_vect.at(i).price
                << " credit: " <<  fred_credit_str
                << " test_credit: " << test_credit_str
                << " vat koef: " << registrar_credit_vect.at(i).koef
                << " vat : " << registrar_credit_vect.at(i).vat
                << std::endl;


            std::cout << "\ndec_credit_query: " << dec_credit_query <<  std::endl;
            std::cout << "\ndec_credit_query_params: ";
            std::size_t i_num=0;
            for (Database::QueryParams::iterator it = dec_credit_query_params.begin(); it != dec_credit_query_params.end(); ++it )
            {
                ++i_num;
                std::cout << " $" << i_num << ": " << it->print_buffer();
            }
            std::cout << std::endl;
            */
        }//if not equal
    }

    std::string test_get_credit_by_zone = invMan->getCreditByZone(registrar_handle,zone_cz_id);

    if(test_get_credit_by_zone.compare(test_credit_str) != 0)
    {
        /*
        std::cout << "test_get_credit_by_zone: " << test_get_credit_by_zone
	    << " test_credit_str: " << test_credit_str << std::endl;
	    */
    }
    BOOST_CHECK((test_get_credit_by_zone.compare(test_credit_str) == 0));

    //try to resolve
    ccReg::Admin_var admin_ref;
    admin_ref = ccReg::Admin::_narrow(
            CorbaContainer::get_instance()->nsresolve("Admin"));

    CORBA::String_var registrar_handle_cpy = CORBA::string_dup(registrar_handle.c_str());
    CORBA::String_var corba_credit = admin_ref->getCreditByZone(registrar_handle_cpy,zone_cz_id);

    if(std::string(corba_credit.in()).compare(test_credit_str) != 0)
    {
        /*
        std::cout << "corba_credit: " << std::string(corba_credit.in())
        << " test_credit_str: " << test_credit_str << std::endl;
        */
    }
    BOOST_CHECK((std::string(corba_credit.in()).compare(test_credit_str) == 0));
}//BOOST_AUTO_TEST_CASE( createDepositInvoice_credit_note )

BOOST_AUTO_TEST_CASE( createDepositInvoice_novat )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    unsigned long long registrar_novat_inv_id = createTestRegistrar();

    conn.exec_params("UPDATE registrar SET vat=false WHERE id = $1::bigint"
            , Database::query_param_list(registrar_novat_inv_id));

    std::vector<registrar_credit_item> registrar_novat_credit_vect;

    {//get registrar novat credit
        registrar_credit_item ci={1400,"0.00",0,"0.00", "0.00", Database::Date(1400,1,1)};

        Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_novat_inv_id));
        if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

        registrar_novat_credit_vect.push_back(ci);//save credit
    }

    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    try_insert_invoice_prefix();

    unsigned long long invoiceid = 0;

    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        {
            Database::Date taxdate (year,1,1);
            Money out_credit;
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_novat_inv_id//registrar
                    , Money("200.00")
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);
            ::LibFred::Credit::add_credit_to_invoice( registrar_novat_inv_id,  zone_cz_id, out_credit, invoiceid);

            //get registrar credit
            registrar_credit_item ci={year,"0.00",0,"0.00", "200.00", Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_novat_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

            registrar_novat_credit_vect.push_back(ci);//save credit
        }

        {
            Database::Date taxdate (year,12,31);
            Money out_credit;
            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                    , zone_cz_id//zone
                    , registrar_novat_inv_id//registrar
                    , Money("200.00")
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
            BOOST_CHECK_EQUAL(invoiceid != 0,true);
            ::LibFred::Credit::add_credit_to_invoice( registrar_novat_inv_id,  zone_cz_id, out_credit, invoiceid);

            //get registrar credit
            registrar_credit_item ci={year,"0.00",0,"0.00", "200.00", Database::Date(1400,1,1)};

            Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                    , Database::query_param_list(zone_cz_id)(registrar_novat_inv_id));
            if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

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
                +"::numeric )::numeric(10,2) ";//round vat to 2 places

        //std::string fred_credit_str ( str(boost::format("%1$.2f") % registrar_novat_credit_vect.at(i).credit_from_query));
        std::string fred_credit_str (registrar_novat_credit_vect.at(i).credit_from_query);

        std::string test_credit_str(std::string(conn.exec_params(
                dec_credit_query
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


            std::cout << "\ndec_credit_query: " << dec_credit_query <<  std::endl;
            std::cout << "\ndec_credit_query_params: ";
            std::size_t i_num=0;
            for (Database::QueryParams::iterator it = dec_credit_query_params.begin(); it != dec_credit_query_params.end(); ++it )
            {
                ++i_num;
                std::cout << " $" << i_num << ": " << it->print_buffer();
            }
            std::cout << std::endl;


        }//if not equal
    }

}//BOOST_AUTO_TEST_CASE( createDepositInvoice_novat )

// some fields are only used by Create Domain test
struct ChargeTestParams {
    Database::ID zone_id;
    Database::ID regid;

    ccReg_EPP_i *epp_backend;

    unsigned clientId;

    ChargeTestParams() : zone_id(0), regid(0), epp_backend(NULL), clientId(0)
    { }
};

struct ResultTestCharge : ChargeTestParams {
    unsigned number;

    Money credit_before;
    Money credit_after;
    Money counted_price;
    unsigned operation;
    std::string object_handle;
    unsigned units;
    Database::Date exdate;
    std::string test_desc;

    ResultTestCharge() : ChargeTestParams(), number(0), credit_before("0"), credit_after("0"), counted_price("0"),
            operation(0), object_handle(), units(0), exdate(), test_desc()
        { }
};

// thread-safe worker
ResultTestCharge testCreateDomainDirectWorker(ccReg_EPP_i *epp_backend, Database::Date exdate_, unsigned reg_units,
        unsigned operation, Database::ID registrar_id,
        unsigned number, unsigned clientId, int zone_id)
{
    ResultTestCharge ret;

    ret.exdate = exdate_;
    ret.units = reg_units;
    ret.zone_id = zone_id;
    ret.regid = registrar_id;
    ret.operation = operation;
    ret.number = 0;

    ret.test_desc =
        (boost::format("test create domain operation charging - exdate: %1%, reg_units: %2%, operation: %3% , zone_id: %4%, registrar_id: %5%")
        % ret.exdate % ret.units % operation % zone_id % registrar_id).str();

    std::string time_string(TimeStamp::microsec());


    // do the operation
    if ( operation == INVOICING_DomainCreate ) {
        ccReg::Response_var r;

        ccReg::Period_str period;
        period.count = reg_units;
        period.unit = ccReg::unit_year;
        ccReg::EppParams epp_params;
        epp_params.requestID = clientId + number;
        epp_params.loginID = clientId;
        epp_params.clTRID = "";
        epp_params.XML = "";
        CORBA::String_var crdate;
        CORBA::String_var exdate(CORBA::string_dup(exdate_.to_string().c_str()));

        std::string test_domain_fqdn((boost::format("tdomain%1%-%2%.cz") % time_string % number).str());

        ret.object_handle = test_domain_fqdn;

        try {
            r = epp_backend->DomainCreate(
                (test_domain_fqdn).c_str(), // fqdn
                "KONTAKT",                // contact
                "",                       // nsset
                "",                       // keyset
                "",                       // authinfo
                period,                   // reg. period
                ccReg::AdminContact(),    // admin contact list
                crdate,                   // create datetime (output)
                exdate,                   // expiration date (output)
                epp_params,               // common call params
                ccReg::ExtensionList());

        } catch (ccReg::EPP::EppError &ex) {
            handle_epp_exception(ex);
        }

        if(r->code != 1000) {
            std::cerr << "ERROR: Return code: " << r->code << std::endl;
            throw CreateDomainFailed(std::string("Error received. Code: ") + boost::lexical_cast<std::string>(r->code));
        }

    } else if (operation == INVOICING_DomainRenew) {
        throw std::runtime_error("Not implemented");
    } else {
        throw std::runtime_error("Not implemented");
    }

    return ret;
}

// check if credit was changed correctly and if correct data was inserted into database after charging for create or renew
void testChargeEval(const ResultTestCharge &res)
{
    Database::Connection conn = Database::Manager::acquire();

    // now check the credit change
    boost::format credit_desc = boost::format(" credit before: %1%, credit_after: %2%, counted price: %3%")
        % res.credit_before % res.credit_after % res.counted_price;

    BOOST_REQUIRE_MESSAGE(res.counted_price == res.credit_before - res.credit_after, "Charged credit match: " + credit_desc.str());

    Database::Result res_or = conn.exec_params("SELECT id FROM object_registry WHERE "
                    "name = $1 AND crid = $2::integer AND erdate is null",
            Database::query_param_list ( res.object_handle )
                                       ( res.regid)     );
    BOOST_REQUIRE_MESSAGE(res_or.size() == 1, "Expecting uniquer record in object_registry... ");
    unsigned object_id = res_or[0][0];


    Database::Result res_ior = conn.exec_params(
    "SELECT quantity, date_to, operation_id, ((crdate AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague')::date FROM invoice_operation WHERE object_id = $1::integer "
                                                    "AND registrar_id = $2::integer "
                                                    "AND zone_id = $3::integer",
               Database::query_param_list ( object_id )
                                          ( res.regid)
                                          ( res.zone_id));

    if(res.operation == INVOICING_DomainCreate) {
        boost::format msg ("Incorrect number of records in invoice_operation table: %1%");
        msg % res.test_desc;
        BOOST_REQUIRE_MESSAGE(res_ior.size() == 2, msg.str());

        for (int i=0; i<2; i++) {
            int  operation        = (int) res_ior[i][2];

            // units number is only saved when renewing
            // this is simplified - it depends also on data in price list - it's true only of base_period == 0
            // which is now the case for 'create' operation
            if (operation == INVOICING_DomainRenew) {
                BOOST_REQUIRE((unsigned)res_ior[i][0] == res.units);
                BOOST_REQUIRE(date(from_string(res_ior[i][1])) == date(from_string(res_ior[i][3])) + years(res.units));
            } else if (operation == INVOICING_DomainCreate) {
                BOOST_REQUIRE(res_ior[i][1].isnull());
            }
        }
    } else {
        throw std::runtime_error("Operation not implemented (in testChargeEval) ");
    }
}

void testCreateDomainWrap(ccReg_EPP_i *epp_backend, Database::Date exdate, unsigned reg_units, unsigned operation,
        Database::ID registrar_id, unsigned number, unsigned clientId, Database::ID zone_id)
{
    Money c1 = get_credit (registrar_id, zone_id);

    ResultTestCharge res = testCreateDomainDirectWorker(epp_backend, exdate, reg_units, operation, registrar_id, number, clientId, zone_id);

    Money c2 = get_credit (registrar_id, zone_id);

    res.credit_before = c1;
    res.credit_after  = c2;

    // TODO HARDCODED spec - create is for free, renew is paid and they are called together
    res.counted_price = getOperationPrice(INVOICING_DomainRenew, zone_id, reg_units);

    testChargeEval(res);
}

BOOST_AUTO_TEST_CASE( chargeDomainNoCredit )
{
    Database::ID zone_cz_id = get_zone_cz_id();

    ::LibFred::Registrar::Registrar::AutoPtr reg = createTestRegistrarClass();

    std::unique_ptr<ccReg_EPP_i> b = create_epp_backend_object();
    unsigned clid = epp_backend_login(b.get(), reg->getHandle());

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    Money before = get_credit(reg->getId(), zone_cz_id);
    BOOST_CHECK_EXCEPTION(
            testCreateDomainDirectWorker(b.get(), Database::Date(act_year+1, 1,1), 1, INVOICING_DomainCreate, reg->getId(), 1, clid, zone_cz_id),
            std::runtime_error,
            check_std_exception_billing_fail
    );
    Money after = get_credit(reg->getId(), zone_cz_id);
    BOOST_REQUIRE_MESSAGE(before == after, "Credit before and after unsuccessful operation has to match");
}

// try to charge create domain with insufficient credit
void testChargeInsuffCredit(::LibFred::Invoicing::Manager *invMan, unsigned reg_units, unsigned op,
        Database::ID zone_id)
{
    ::LibFred::Registrar::Registrar::AutoPtr reg = createTestRegistrarClass();

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();
    Money amount = ( getOperationPrice(op, zone_id, reg_units) * Decimal("0.9")).integral_division( Decimal("2"));

    // add credit for new registrar
    Database::Date taxdate (act_year,1,1);
    Money out_credit;
    Database::ID invoiceid = invMan->createDepositInvoice(
                    Database::Date (act_year,1,1)//taxdate
                    , zone_id//zone
                    , reg->getId()//registrar
                    , amount
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
    ::LibFred::Credit::add_credit_to_invoice( reg->getId(),  zone_id, out_credit, invoiceid);

    BOOST_CHECK_EQUAL(invoiceid != 0,true);


    std::unique_ptr<ccReg_EPP_i> myccReg_EPP_i = create_epp_backend_object();
    CORBA::Long clientId = epp_backend_login(myccReg_EPP_i.get(), reg->getHandle());

    Money before = get_credit(reg->getId(), zone_id);
    Database::Date exdate(act_year + 1, 1, 1);
    BOOST_CHECK_EXCEPTION(
            testCreateDomainDirectWorker(myccReg_EPP_i.get(), exdate, reg_units, INVOICING_DomainCreate, reg->getId(), 1, clientId, zone_id),
            std::runtime_error,
            check_std_exception_billing_fail
    );
    Database::Date exdate2(act_year + 5, 4, 30);

    BOOST_CHECK_EXCEPTION(
            testCreateDomainDirectWorker(myccReg_EPP_i.get(), exdate, reg_units, INVOICING_DomainCreate, reg->getId(), 1, clientId, zone_id),
            std::runtime_error,
            check_std_exception_billing_fail
    );
    Money after = get_credit(reg->getId(), zone_id);
    BOOST_REQUIRE_MESSAGE(before == after, "Credit before and after unsuccessful operation has to match");
}

// not thread safe - database data can change in the meantime
BOOST_AUTO_TEST_CASE( chargeDomainInsuffCredit )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainCreate, zone_cz_id);
    testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainCreate, zone_enum_id);
    testChargeInsuffCredit(invMan.get(),  7, INVOICING_DomainCreate, zone_cz_id);
    testChargeInsuffCredit(invMan.get(),  7, INVOICING_DomainCreate, zone_enum_id);

    testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainCreate, zone_cz_id);
    testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainCreate, zone_enum_id);
    testChargeInsuffCredit(invMan.get(), 7, INVOICING_DomainCreate, zone_cz_id);
    testChargeInsuffCredit(invMan.get(), 7, INVOICING_DomainCreate, zone_enum_id);

    //testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainRenew, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainRenew, zone_enum_id);
    //testChargeInsuffCredit(invMan.get(),  19, INVOICING_DomainRenew, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(),  19, INVOICING_DomainRenew, zone_enum_id);

    //testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainRenew, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainRenew, zone_enum_id);
    //testChargeInsuffCredit(invMan.get(), 19, INVOICING_DomainRenew, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(), 19, INVOICING_DomainRenew, zone_enum_id);
}

// not thread safe - database data can change in the meantime
BOOST_AUTO_TEST_CASE( chargeDomain )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();

    // add credit
    Money amount = std::string("20000.00");
    unsigned act_year = boost::gregorian::day_clock::universal_day().year();
    //manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    Database::Date taxdate (act_year,1,1);
    Money out_credit;
    Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_cz_id//zone
                    , registrar->getId()//registrar
                    , amount
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
    ::LibFred::Credit::add_credit_to_invoice( registrar->getId(),  zone_cz_id, out_credit, invoiceid);
    BOOST_CHECK_EQUAL(invoiceid != 0,true);

    Database::ID invoiceid2 = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_enum_id//zone
                    , registrar->getId()//registrar
                    , amount
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
    ::LibFred::Credit::add_credit_to_invoice( registrar->getId(),  zone_enum_id, out_credit, invoiceid2);
    BOOST_CHECK_EQUAL(invoiceid2 != 0,true);

    // run tests
    Database::Date exdate(act_year + 1, 1, 1);
    Database::Date exdate2(act_year + 5, 4, 30);

    std::unique_ptr<ccReg_EPP_i> myccReg_EPP_i = create_epp_backend_object();
    CORBA::Long clientId = epp_backend_login(myccReg_EPP_i.get(), registrar->getHandle());

    testCreateDomainWrap(myccReg_EPP_i.get(), exdate, 2, INVOICING_DomainCreate, registrar->getId(), 1, clientId, zone_cz_id);
    testCreateDomainWrap(myccReg_EPP_i.get(), exdate, 3, INVOICING_DomainCreate, registrar->getId(), 1, clientId, zone_cz_id);

    // TODO support enum
    // res = testCreateDomainDirectWorker(myccReg_EPP_i.get(), exdate, 2, INVOICING_DomainCreate, registrar->getId(), 1, clientId, zone_enum_id);
    // testChargeEval(res);
//    ResultTestCharge res = testChargeWorker(invMan.get(), exdate2, 2, INVOICING_DomainCreate, zone_enum_id, regid);
//    ResultTestCharge res = testChargeWorker(invMan.get(), exdate, 2, INVOICING_DomainCreate, zone_enum_id, regid);


//    ResultTestCharge res = testChargeWorker(invMan.get(), exdate, 3, INVOICING_DomainCreate, zone_enum_id, regid);

    // TODO support enum
    // res = testCreateDomainDirectWorker(myccReg_EPP_i.get(), exdate, 3, INVOICING_DomainCreate, registrar->getId(), 1, clientId, zone_enum_id);
    // testChargeEval(res);
//    ResultTestCharge res = testChargeWorker(invMan.get(), exdate2, 3, INVOICING_DomainCreate, zone_enum_id, regid);

//     ResultTestCharge res = testChargeWorker(invMan.get(), exdate, 2, INVOICING_DomainRenew, zone_cz_id, regid);
//   ResultTestCharge res = testChargeWorker(invMan.get(), exdate, 2, INVOICING_DomainRenew, zone_enum_id, regid);
//    ResultTestCharge res = testChargeWorker(invMan.get(), exdate2, 2, INVOICING_DomainRenew, zone_cz_id, regid);
//   ResultTestCharge res = testChargeWorker(invMan.get(), exdate2, 2, INVOICING_DomainRenew, zone_enum_id, regid);

//   ResultTestCharge res = testChargeWorker(invMan.get(), exdate, 3, INVOICING_DomainRenew, zone_cz_id, regid);
//   ResultTestCharge res = testChargeWorker(invMan.get(), exdate, 3, INVOICING_DomainRenew, zone_enum_id, regid);

//   ResultTestCharge res = testChargeWorker(invMan.get(), exdate2, 3, INVOICING_DomainRenew, zone_cz_id, regid);
//   ResultTestCharge res = testChargeWorker(invMan.get(), exdate2, 3, INVOICING_DomainRenew, zone_enum_id, regid);

}

void testCharge2InvoicesWorker(Database::ID zone_id, unsigned op, unsigned period,
        Database::Date taxdate, Database::Date exdate, Money amount, bool should_succ)
{
    // registrar
    ::LibFred::Registrar::Registrar::AutoPtr reg = createTestRegistrarClass();

    // invoice manager
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());
    // add credit - 2 invoices for the same zone:
    if(amount > Money("0")) {
        create2Invoices(invMan.get(), taxdate, zone_id, reg->getId(), amount);
    }

    std::unique_ptr<ccReg_EPP_i> myccReg_EPP_i = create_epp_backend_object();
    CORBA::Long clientId = epp_backend_login(myccReg_EPP_i.get(), reg->getHandle());
    if(should_succ) {
        testCreateDomainWrap(myccReg_EPP_i.get(), exdate, period, INVOICING_DomainCreate, reg->getId(), 1, clientId, zone_id);
    } else {
        Money before = get_credit(reg->getId(), zone_id);
        BOOST_CHECK_EXCEPTION(
                testCreateDomainDirectWorker(myccReg_EPP_i.get(), exdate, period, INVOICING_DomainCreate, reg->getId(), 1, clientId, zone_id),
                std::runtime_error,
                check_std_exception_billing_fail
        );
        Money after = get_credit(reg->getId(), zone_id);
        BOOST_REQUIRE_MESSAGE(before == after, "Credit before and after unsuccessful operation has to match");
    }

    //db
    Database::Connection conn = Database::Manager::acquire();
    // zone IDs
    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    const auto crdate = boost::posix_time::second_clock::universal_time();
    const auto price_timestamp = crdate;
    invMan->charge_operation_auto_price(
              "GeneralEppOperation"
              , zone_cz_id
              , reg->getId()
              , 0 //object_id
              , crdate
              , exdate.get() - boost::gregorian::months(1)//date_from //local date
              , exdate.get()// date_to //local date
              , price_timestamp
              , Decimal ("900000"));
}

BOOST_AUTO_TEST_CASE( chargeDomain2Invoices )
{
    // zone IDs
    Database::ID zone_cz_id = get_zone_cz_id();
    // Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    unsigned operation = (unsigned) INVOICING_DomainRenew;
    unsigned period = 7;

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    Money op_price_cz = getOperationPrice(operation, zone_cz_id, period);
       // price for invoices so that 2 are not sufficient
       // cent_amount amount = op_price / 3;

       // price for invoices so that 2 are needed.
       // TODO hardcoded VAT - change
    Money amount = ( op_price_cz * Decimal("1.22")) / Decimal("2");

    testCharge2InvoicesWorker(zone_cz_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount, true);


    /* TODO enum not supported in the test yet
    Money op_price_enum = getOperationPrice(operation, zone_enum_id, period);
       // price for invoices so that 2 are not sufficient
       // cent_amount amount = op_price / 3;

       // price for invoices so that 2 are needed.
       // TODO hardcoded VAT - change
    Money amount2 = ( op_price_enum * Decimal("1.22")) / Decimal("2");
    std::cout << "Second case credit: " << amount2 << std::endl;

    testCharge2InvoicesWorker(zone_enum_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount2, true);
            */
}

BOOST_AUTO_TEST_CASE( chargeDomain2InvoicesNotSuff )
{
    // zone IDs
    Database::ID zone_cz_id = get_zone_cz_id();
    // TODO unused

    unsigned operation = (unsigned) INVOICING_DomainRenew;
    unsigned period = 3;

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    Decimal op_price = getOperationPrice(operation, zone_cz_id, period);
    Decimal amount = ( op_price * Decimal("0.9")) / Decimal("2");

    testCharge2InvoicesWorker(zone_cz_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount, false);

// The same for ENUM
 /*
    Decimal op_price_enum = getOperationPrice(operation, zone_enum_id, period);
    Decimal amount2 = ( op_price_enum * Decimal("0.9")) / Decimal("2");

    testCharge2InvoicesWorker(zone_enum_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount2, false);
    */
}

BOOST_AUTO_TEST_CASE( chargeDomain2InvoicesNoCred )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    // TODO unused
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    unsigned operation = (unsigned) INVOICING_DomainRenew;
    unsigned period = 3;

    Money price_cz = getOperationPrice(operation, zone_cz_id, 1);
    if ( price_cz == Money("0"))
    {
        BOOST_ERROR("Cannot perform the test - operation not charged");
    }

    Money price_enum = getOperationPrice(operation, zone_enum_id, 1);
    if ( price_enum == Money("0")) {
        BOOST_ERROR("Cannot perform the test - operation not charged");
    }

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    testCharge2InvoicesWorker(zone_cz_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), Money("0"), false);

    testCharge2InvoicesWorker(zone_enum_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), Money("0"), false);
}

class TestCreateDomainDirectThreadedWorker : public ThreadedTestWorker<ResultTestCharge, ChargeTestParams>
{
public:
    typedef ThreadedTestWorker<ResultTestCharge, ChargeTestParams>::ThreadedTestResultQueue queue_type;

    TestCreateDomainDirectThreadedWorker(
            unsigned number,
            boost::barrier* sb,
            std::size_t thread_group_divisor,
            queue_type* result_queue, ChargeTestParams params)
        : ThreadedTestWorker<ResultTestCharge, ChargeTestParams>(number, sb, thread_group_divisor, result_queue, params)
    { }
    // this shouldn't throw
    ResultTestCharge run(const ChargeTestParams &p)
    {
       unsigned act_year = boost::gregorian::day_clock::universal_day().year();

       Database::Date exdate(act_year + 1, 1, 1);

       ResultTestCharge ret = testCreateDomainDirectWorker(p.epp_backend, exdate,
               1, INVOICING_DomainCreate, p.regid, number_, p.clientId, p.zone_id);

       // copy all the parametres to Result
       ret.number = number_;

       return ret;
    }
};

BOOST_AUTO_TEST_CASE(createDomainDirectThreaded)
{
     std::unique_ptr<ccReg_EPP_i> epp = create_epp_backend_object();

     std::string time_string(TimeStamp::microsec());
     std::string noregistrar_handle(std::string("REG-NOTEXISTS")+time_string);
    // registrar
     ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
     std::string registrar_handle  = registrar->getHandle();
     unsigned long long registrar_inv_id = registrar->getId();

     // ------------------ login
     /// log in while Database::Connection is not acquired yet

     CORBA::Long clientId = epp_backend_login(epp.get(), registrar_handle);

     // --------------------- add credit, acquire connection
     Money amount ("90000.00");
     unsigned act_year = boost::gregorian::day_clock::universal_day().year();

     Database::Connection conn = Database::Manager::acquire();
     Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

     // Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];
     std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

     Database::Date taxdate (act_year,1,1);
     Money out_credit;
     Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                     , zone_cz_id//zone
                     , registrar_inv_id//registrar
                     , amount
                     , boost::posix_time::ptime(taxdate), out_credit);//price
     BOOST_CHECK_EQUAL(invoiceid != 0,true);
     ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

    // CHECK CREDIT before
    Money credit_before, credit_after;

    Database::Result credit_res1 = conn.exec_params(zone_registrar_credit_query
                           , Database::query_param_list(zone_cz_id)(registrar_inv_id));
    if(credit_res1.size() ==  1 && credit_res1[0].size() == 1) {
        credit_before.set_string(std::string(credit_res1[0][0]));
    } else {
        BOOST_FAIL("Couldn't count registrar credit ");
    }

//  ----------- test itself
    ChargeTestParams params;
    params.zone_id = zone_cz_id ;
    params.regid = registrar_inv_id;
    params.epp_backend = epp.get();
    params.clientId = clientId;

    Decimal thread_count = boost::lexical_cast<std::string>(
            threadedTest< TestCreateDomainDirectThreadedWorker> (params, &testChargeEval));

    Database::Connection conn2 = Database::Manager::acquire();

    // Database::Connection conn_new = Database::Manager::acquire();
    // ----------------------- CHECK CREDIT after
    Database::Result credit_res2 = conn2.exec_params(zone_registrar_credit_query
                           , Database::query_param_list(zone_cz_id)(registrar_inv_id));
    if(credit_res2.size() ==  1 && !credit_res2[0][0].isnull()) {
        credit_after.set_string(std::string(credit_res2[0][0]));
    } else {
        BOOST_FAIL("Couldn't count registrar credit ");
    }

    // if(operation == INVOICING_DomainCreate)
    Money counted_price =
            getOperationPrice(INVOICING_DomainRenew, zone_cz_id, 1)
            + getOperationPrice(INVOICING_DomainCreate, zone_cz_id, 1);

    counted_price *= thread_count;

    BOOST_REQUIRE_MESSAGE(credit_before - credit_after == counted_price, boost::format(
        "Credit before (%1%) and  after (%2%) + price (%3%) threaded test don't match. ")
            % credit_before
            % credit_after
            % counted_price );

}

void createCzDomain(Database::ID regid, const std::string &reg_handle)
{
    Database::ID zone_cz_id = get_zone_cz_id();

    std::unique_ptr<ccReg_EPP_i> myccReg_EPP_i = create_epp_backend_object();

   // ------------------ login

   CORBA::Long clientId = epp_backend_login(myccReg_EPP_i.get(), reg_handle);

   testCreateDomainDirectWorker(myccReg_EPP_i.get(), Database::Date(), 1,
          INVOICING_DomainCreate, regid, 1, clientId, zone_cz_id);
}

BOOST_AUTO_TEST_CASE(testCreateDomainEPPNoCORBA)
{
    // registrar
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    unsigned long long registrar_inv_id = registrar->getId();
    std::string registrar_handle = registrar->getHandle();

     // ### add credit
     Money amount ("90000.00");
     unsigned act_year = boost::gregorian::day_clock::universal_day().year();

     Database::Connection conn = Database::Manager::acquire();
     Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

     // Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];
     std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

     Database::Date taxdate (act_year,1,1);
     Money out_credit;
     Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                     , zone_cz_id//zone
                     , registrar_inv_id//registrar
                     , amount
                     , boost::posix_time::ptime(taxdate), out_credit);//price
     BOOST_CHECK_EQUAL(invoiceid != 0,true);
     ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

     createCzDomain(registrar->getId(), registrar_handle);
}

BOOST_AUTO_TEST_CASE(make_debt)
{
    const Decimal postpaid_op_price ("100000");
    // assuming that price of request is 0.1
    const Decimal unit_price ("0.1");
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();

    Database::ID reg_id = registrar->getId();
    Database::ID zone_id = get_zone_cz_id();

    date today = day_clock::local_day();
    Database::Date taxdate(today);

    Money price = std::string("5000.00");//money
    Money out_credit;

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    unsigned long long  invoiceid = invMan->createDepositInvoice(taxdate//taxdate
        , zone_id//zone
        , reg_id//registrar
        , price
        , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    ::LibFred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid);

    Decimal credit_before = get_credit(reg_id, zone_id);

    const auto crdate = boost::posix_time::second_clock::universal_time();
    const auto price_timestamp = crdate;
    BOOST_CHECK(
            invMan->charge_operation_auto_price(
         "GeneralEppOperation"
         , zone_id
         , reg_id
         , 0 //object_id
         , crdate
         , today - boost::gregorian::months(1)//date_from //local date
         , today // date_to //local date
         , price_timestamp
         , postpaid_op_price )
    );

    Decimal credit_between = get_credit(reg_id, zone_id);
    BOOST_CHECK(credit_before - credit_between == unit_price * postpaid_op_price);

    BOOST_CHECK_EXCEPTION(
            createCzDomain(reg_id, registrar->getHandle()),
            std::runtime_error,
            check_std_exception_billing_fail
    );

    Decimal credit_after = get_credit(reg_id, zone_id);
    BOOST_CHECK(credit_after == credit_between);
}

BOOST_AUTO_TEST_CASE(lower_debt)
{
    const Decimal postpaid_operation ("100000");
    const Decimal unit_price ("0.1");
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    Database::ID zone_id = get_zone_cz_id();

    Database::ID reg_id = registrar->getId();

    date today = day_clock::local_day();
    Database::Date taxdate(today);

    Money price = std::string("5000.00");//money
    Money out_credit;

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(::LibFred::Invoicing::Manager::create());

    unsigned long long  invoiceid = invMan->createDepositInvoice(taxdate//taxdate
        , zone_id//zone
        , reg_id//registrar
        , price
        , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    ::LibFred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid);

    const auto crdate = boost::posix_time::second_clock::universal_time();
    const auto price_timestamp = crdate;
// assuming that price of request is 0.1
    BOOST_CHECK(
            invMan->charge_operation_auto_price(
         "GeneralEppOperation"
         , zone_id
         , reg_id
         , 0 //object_id
         , crdate
         , today - boost::gregorian::months(1)//date_from //local date
         , today // date_to //local date
         , price_timestamp
         , postpaid_operation )
    );

    // recharge 1000
    const Decimal recharge("1500");
    unsigned long long  invoiceid2 = invMan->createDepositInvoice(taxdate//taxdate
        , zone_id//zone
        , reg_id//registrar
        , recharge
        , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid2 != 0,true);
    ::LibFred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid2);

    BOOST_CHECK_EXCEPTION(
            createCzDomain(reg_id, registrar->getHandle()),
            std::runtime_error,
            check_std_exception_billing_fail
    );

    int vat;
    std::string koef;
    get_vat(vat, koef);
    Decimal vat_ratio = Decimal("1") - Decimal(koef);

    Decimal estimated_credit = Decimal(
                       Decimal(vat_ratio * price).round_half_up(2)
                     + Decimal(vat_ratio * recharge).round_half_up(2)
                     - unit_price * postpaid_operation
        ).round_half_up(2);

    vat_ratio * (price + recharge) - unit_price * postpaid_operation;
    Decimal credit_after = get_credit(reg_id, zone_id);

    BOOST_CHECK(credit_after == estimated_credit);
}

/*
BOOST_AUTO_TESTCASE()
{

    Database::Date taxdate;

        taxdate = Database::Date(year,1,1);
        Money price = std::string("50000.00");//money
        Money out_credit;

Money prices[] = { std::string("500"), std::string("300"), std::string("1000") };
const size_t count = 4;

for(unsigned i=0; i<count; i++)
    unsigned long long  invoiceid = invMan->createDepositInvoice(taxdate//taxdate
        , zone_cz_id//zone
        , registrar_inv_id//registrar
        , prices[i]
        , boost::posix_time::ptime(taxdate), out_credit);//price
BOOST_CHECK_EQUAL(invoiceid != 0,true);
LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);
}
*/

bool default_charge_request_fee(::LibFred::Invoicing::Manager *manager, Database::ID reg_id)
{
    boost::gregorian::date local_today = boost::gregorian::day_clock::local_day();
    date poll_msg_period_to = date(local_today.year(), local_today.month(), 1);

    return manager->chargeRequestFee(reg_id, poll_msg_period_to);
}


BOOST_AUTO_TEST_CASE(test_charge_request)
{
    Decimal price ("10000");
    Database::ID zone_cz_id = get_zone_cz_id();

    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create());

    insert_poll_request_fee(registrar->getId(), price);

    Decimal credit_before = get_credit(registrar->getId(), zone_cz_id );
    BOOST_CHECK(default_charge_request_fee(invMan.get(), registrar->getId()));
    Decimal credit_after = get_credit(registrar->getId(), zone_cz_id );

    BOOST_CHECK(credit_before - credit_after == price);
}

BOOST_AUTO_TEST_CASE(test_charge_request_double)
{
    Decimal price ("10000");

    Database::ID zone_cz_id = get_zone_cz_id();
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
       ::LibFred::Invoicing::Manager::create());

    insert_poll_request_fee(registrar->getId(), price);

    Decimal credit_before = get_credit(registrar->getId(), zone_cz_id);
    BOOST_CHECK(default_charge_request_fee(invMan.get(), registrar->getId()));
    Decimal credit_between = get_credit(registrar->getId(), zone_cz_id);

    BOOST_CHECK(credit_before - credit_between == price);
    // double charging still return true
    BOOST_CHECK(default_charge_request_fee(invMan.get(), registrar->getId()));
    Decimal credit_after = get_credit(registrar->getId(), zone_cz_id);

    BOOST_CHECK(credit_between == credit_after);
}

BOOST_AUTO_TEST_CASE(test_charge_request_missing_poll)
{
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    Database::ID reg_id = registrar->getId();

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
       ::LibFred::Invoicing::Manager::create());

    date local_today = day_clock::local_day();
    for (unsigned month = 1; month < local_today.month(); ++month) {
        date date_to = date(local_today.year(), month, 1);
        insert_poll_request_fee(reg_id, Decimal("10000"), date_to-months(1), date_to );
    }

    date first_day_this_month = date(local_today.year(), local_today.month(), 1);
    insert_poll_request_fee(reg_id,
            Decimal("10000"),
            first_day_this_month,
            first_day_this_month + days(1)
            );

    insert_poll_request_fee(reg_id,
            Decimal("10000"),
            first_day_this_month - months(1),
            first_day_this_month - days(1)
            );

    Database::ID zone_cz_id = get_zone_cz_id();
    // HARDCODED - requests are charged to zone_cz
    Money before = get_credit(reg_id, zone_cz_id);
    BOOST_CHECK_EXCEPTION(default_charge_request_fee(invMan.get(), reg_id), std::exception, check_dummy);

    Money after = get_credit(reg_id, zone_cz_id);
    BOOST_REQUIRE_MESSAGE(before == after, "Credit before and after unsuccessful operation has to match");
}

BOOST_AUTO_TEST_SUITE_END()//TestInvoice
