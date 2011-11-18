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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include "test-common-registry.h"

#include "setup_server_decl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"


#include "types/money.h"
//#include "fredlib/registrar.h"
#include "fredlib/invoicing/invoice.h"

#include "poll.h"
#include "time_clock.h"
#include "credit.h"
#include "file_manager_client.h"
#include "fredlib/banking/bank_common.h"
#include "exceptions.h"

#include "corba/Admin.hh"

#include "test-common-threaded.h"


//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "test-invoice-common.h"

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>






BOOST_AUTO_TEST_SUITE(TestInvoice)



using namespace Fred::Invoicing;

const std::string server_name = "test-invoice";



struct registrar_credit_item
{
  int year;
  std::string credit_from_query;
  int vat;
  std::string koef;
  std::string price;
  Database::Date taxdate;
};

void create2Invoices(Fred::Invoicing::Manager *man, Database::Date taxdate, Database::ID zone_cz_id, Database::ID reg_id, Money amount)
{
    Money out_credit;
   Database::ID invoiceid = man->createDepositInvoice(taxdate //taxdate
                   , zone_cz_id//zone
                   , reg_id//registrar
                   , amount
                   , boost::posix_time::ptime(taxdate)
                   , out_credit);//price
   BOOST_CHECK_EQUAL(invoiceid != 0,true);
   Fred::Credit::add_credit_to_invoice( reg_id,  zone_cz_id, out_credit, invoiceid);

   // add credit for new registrar
   Database::ID invoiceid2 = man->createDepositInvoice(taxdate //taxdate
                   , zone_cz_id//zone
                   , reg_id//registrar
                   , amount
                   , boost::posix_time::ptime(taxdate)
                   , out_credit);//price
   BOOST_CHECK_EQUAL(invoiceid2 != 0,true);
   Fred::Credit::add_credit_to_invoice( reg_id,  zone_cz_id, out_credit, invoiceid2);
}

/*
//  old handler
// this one rethrows - does not work well with boost framework
void handle_epp_exception(ccReg::EPP::EppError &ex) 
{
    std::string error_msg = str(boost::format("code: %1%  message: %2%  svtrid: %3%")
					% _epp_error.errCode
					% _epp_error.errMsg
					% _epp_error.svTRID);

    LOGGER(PACKAGE).error(error_msg);
    std::cerr << error_msg << std::endl;
    throw;    
}
*/

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

    BOOST_CHECK((invMan->getCreditByZone(registrar_handle,zone_cz_id).compare("0.00") == 0));//noregistrar
    BOOST_CHECK((invMan->getCreditByZone(registrar_handle,0).compare("0.00") == 0));//no registrar no zone
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
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

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
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

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

    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    unsigned long long registrar_inv_id = registrar->getId();
    std::string registrar_handle = registrar->getHandle();

    LOGGER(PACKAGE).debug ( boost::format("createDepositInvoice test registrar_id %1%")
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
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

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
            Fred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

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
            Fred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

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

    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::string registrar_handle = registrar->getHandle();
    unsigned long long registrar_inv_id = registrar->getId();

    LOGGER(PACKAGE).debug ( boost::format("createDepositInvoice test registrar_handle %1% registrar_inv_id %2%")
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
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

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
            Fred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

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
            Fred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, credit_note_id);

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
        registrar_credit_item ci={1400,"0.00",0,"0.00", "0.00"};

        Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                , Database::query_param_list(zone_cz_id)(registrar_novat_inv_id));
        if(credit_res.size() ==  1 && credit_res[0].size() == 1) ci.credit_from_query = std::string(credit_res[0][0]);

        registrar_novat_credit_vect.push_back(ci);//save credit
    }

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

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
            Fred::Credit::add_credit_to_invoice( registrar_novat_inv_id,  zone_cz_id, out_credit, invoiceid);

            //get registrar credit
            registrar_credit_item ci={year,"0.00",0,"0.00", "200.00"};

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
            Fred::Credit::add_credit_to_invoice( registrar_novat_inv_id,  zone_cz_id, out_credit, invoiceid);

            //get registrar credit
            registrar_credit_item ci={year,"0.00",0,"0.00", "200.00"};

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

    bool success;
    Money credit_before;
    Money credit_after;
    Money counted_price;
    Database::ID object_id;
    std::string object_handle;
    unsigned units;
    Database::Date exdate;
    std::string test_desc;

    ResultTestCharge() : ChargeTestParams(), success(false), credit_before("0"), credit_after("0"),
            object_id(0), object_handle(), units(0), exdate(), test_desc()
        { }
};

/*
 * TODO to be updated - inconsistent INSERT
// thread-safe worker
// all CHECKs were changed to REQUIRE
ResultTestCharge testChargeWorker(Fred::Invoicing::Manager *invMan, Database::Date exdate, unsigned reg_units,
        unsigned operation, Database::ID zone_id, Database::ID registrar_id)
{
    ResultTestCharge ret;

    ret.exdate = exdate;
    ret.units = reg_units;
    ret.zone_id = zone_id;
    ret.regid = registrar_id;

    ret.test_desc =
        (boost::format("test domain operation charging - exdate: %1%, reg_units: %2%, operation: %3% , zone_id: %4%, registrar_id: %5%")
        % ret.exdate % ret.units % operation % zone_id % registrar_id).str();

    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);

    std::string time_string(TimeStamp::microsec());
    std::string object_roid = std::string("ROID-TEST_INVOICE")  + time_string;

    // CHECK CREDIT before
    Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                       , Database::query_param_list(zone_id)(registrar_id));
    // TODO add check
    if(credit_res.size() ==  1 && credit_res[0].size() == 1) ret.credit_before = std::string(credit_res[0][0]);


    // some fake object in object registry
    // TODO create unique hanle for the object
    conn.exec_params("INSERT INTO object_registry (roid, name, crid ) VALUES ($1, 'object', $2::integer)",
             Database::query_param_list  (object_roid)
                                         (registrar_id) );

    Database::Result res_or = conn.exec_params("SELECT id FROM object_registry WHERE roid = $1 AND crid = $2::integer ",
            Database::query_param_list ( object_roid)
                                       ( registrar_id) );

    BOOST_REQUIRE_MESSAGE(res_or.size() > 0 , "Fake object_registry object wasn't found, cannot perform test");

    ret.object_id = res_or[0][0];

    if (operation == INVOICING_DomainCreate ) {
        ret.success = invMan->chargeDomainCreate(zone_id, registrar_id, ret.object_id, exdate, ret.units * 12 );
    } else if (operation == INVOICING_DomainRenew) {
        ret.success = invMan->chargeDomainRenew(zone_id, registrar_id, ret.object_id, exdate, ret.units * 12 );
    } else {
        THREAD_BOOST_ERROR("Not implemented");
    }

    // CHECK CREDIT after
    Database::Result credit_res2 = conn.exec_params(zone_registrar_credit_query
                           , Database::query_param_list(zone_id)(registrar_id));
                   if(credit_res2.size() ==  1 && credit_res2[0].size() == 1) ret.credit_after = std::string(credit_res2[0][0]);

    ret.counted_price = getOperationPrice(operation, zone_id, ret.units);

    tx.commit();
    return ret;
}
*/

void testChargeEval(const ResultTestCharge &res, bool should_succeed)
{
    Database::Connection conn = Database::Manager::acquire();

    if(should_succeed) {
        BOOST_REQUIRE_MESSAGE (res.success, res.test_desc);
    } else {
        BOOST_REQUIRE_MESSAGE (!res.success, res.test_desc);
    }

    // now check the credit change
    boost::format credit_desc = boost::format(" credit before: %1%, credit_after: %2%, counted price: %3%")
        % res.credit_before % res.credit_after % res.counted_price;

    if(should_succeed) {
        BOOST_REQUIRE_MESSAGE(res.counted_price == res.credit_before - res.credit_after, "Charged credit match: " + credit_desc.str());
    } else {
        BOOST_REQUIRE_MESSAGE(res.credit_before == res.credit_after, "No credit charged - operation was not successful. " + credit_desc.str());
    }

    if(should_succeed) {
        Database::Result res_ior = conn.exec_params(
        "SELECT quantity, date_to, operation_id FROM invoice_operation WHERE object_id = $1::integer "
                                                        "AND registrar_id = $2::integer "
                                                        "AND zone_id = $3::integer",
                   Database::query_param_list ( res.object_id )
                                              ( res.regid)
                                              ( res.zone_id));

        BOOST_REQUIRE_MESSAGE(res_ior.size() == 1, "Incorrect number of records in invoice_operation table ");

	int  operation        = (int) res_ior[0][2];

	// units number is only saved when renewing 
	// this is simplified - it depends also on data in price list - it's true only of base_period == 0
	// which is now the case for 'create' operation
	if (operation == INVOICING_DomainRenew) {
		BOOST_REQUIRE((unsigned)res_ior[0][0] == res.units);
		BOOST_REQUIRE(res_ior[0][1] == res.exdate);
	} else if (operation == INVOICING_DomainCreate) {
		BOOST_REQUIRE(res_ior[0][1].isnull());
	}
    }

}


// TODO this could be more detailed
/* if successful operation is expected:
 *    - res.success must be true
 *    - created object exists in object_registry (this is disputable -
 *       in our case testing charging means testing domain creation
 *    - correct records in invoice_operation bound to object
 *
 */
void testCreateDomainEval(const ResultTestCharge &res, bool should_succeed)
{
    Database::Connection conn = Database::Manager::acquire();

    assert(res.object_id  == 0);

    if(!should_succeed) {
        BOOST_REQUIRE_MESSAGE (!res.success, res.test_desc);
    }

    if(should_succeed) {

        BOOST_REQUIRE_MESSAGE (res.success, res.test_desc);

        // TODO here -- threaded
        Database::Result res_or = conn.exec_params("SELECT id FROM object_registry WHERE "
                        "name = $1 AND crid = $2::integer AND erdate is null",
                Database::query_param_list ( res.object_handle )
                                           ( res.regid)     );

        BOOST_REQUIRE_MESSAGE(res_or.size() == 1, "Expecting uniquer record in object_registry... ");
        unsigned object_id = res_or[0][0];

        Database::Result res_ior = conn.exec_params(
        "SELECT quantity, date_to FROM invoice_operation WHERE object_id = $1::integer "
                                                        "AND registrar_id = $2::integer "
                                                        "AND zone_id = $3::integer",
                   Database::query_param_list ( object_id )
                                              ( res.regid)
                                              ( res.zone_id));

        boost::format msg = boost::format("Incorrect number of records in invoice_operation table, object_id: %1%, regid: %2%, zone_id: %3% ")
            % object_id % res.regid % res.zone_id;
        BOOST_REQUIRE_MESSAGE(res_ior.size() == 2, msg.str());
    }

}

//this is needed for threaded test template
void testChargeEvalSucc(const ResultTestCharge &res)
{
    testChargeEval(res, true);
}

void testCreateDomainEvalSucc(const ResultTestCharge &res)
{
    testCreateDomainEval(res, true);
}

/*
void testChargeSucc(Fred::Invoicing::Manager *invMan, Database::Date &exdate, unsigned reg_units,
        unsigned operation, Database::ID zone_id, Database::ID registrar_id)
{
    ResultTestCharge res = testChargeWorker(invMan, exdate, reg_units, operation, zone_id, registrar_id);
    testChargeEval(res, true);
}

void testChargeFail(Fred::Invoicing::Manager *invMan, Database::Date exdate, unsigned reg_units,
        unsigned operation, Database::ID zone_id, Database::ID registrar_id)
{
    ResultTestCharge res = testChargeWorker(invMan, exdate, reg_units, operation, zone_id, registrar_id);
    testChargeEval(res, false);
}
*/



/*
BOOST_AUTO_TEST_CASE( chargeDomainNoCredit )
{

    std::string time_string(TimeStamp::microsec());
    std::string object_roid = std::string("ROID-TEST_INVOICE")  + time_string;

    //db
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    Database::ID regid = createTestRegistrar();

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());







    // some fake object in object registry
    // TODO create unique handle for the object
    conn.exec_params("INSERT INTO object_registry (roid, name, crid ) VALUES ($1, 'object', $2::integer)",
             Database::query_param_list  (object_roid)
                                         (regid) );
    Database::Result res_or = conn.exec_params("SELECT id FROM object_registry WHERE roid = $1 AND crid = $2::integer ",
            Database::query_param_list ( object_roid )
                                       ( regid) );

    BOOST_REQUIRE_MESSAGE(res_or.size() > 0 , "Fake object_registry object wasn't found, cannot perform test");

    Database::ID object_id = res_or[0][0];

    // TODO this should check price_list - if operation price is really 0
    // TODO change date
    BOOST_CHECK(invMan->chargeDomainCreate(zone_cz_id, regid, object_id, Database::Date(2012, 1, 1), 0 ));

    tx.commit();
}

TODO: modify
// try to charge create domain with insufficient credit
void testChargeInsuffCredit(Fred::Invoicing::Manager *invMan, unsigned reg_units, unsigned op,
        Database::ID zone_id)
{
    Database::ID reg_id = createTestRegistrar();

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    Money amount = ( getOperationPrice(op, zone_id, reg_units) * Decimal("0.9")).integral_division( Decimal("2"));

    // add credit for new registrar
    Database::Date taxdate (act_year,1,1);
    Money out_credit;
    Database::ID invoiceid = invMan->createDepositInvoice(
                    Database::Date (act_year,1,1)//taxdate
                    , zone_id//zone
                    , reg_id//registrar
                    , amount
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
    Fred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid);

    BOOST_CHECK_EQUAL(invoiceid != 0,true);

    Database::Date exdate(act_year + 1, 1, 1);
    Database::Date exdate2(act_year + 5, 4, 30);
    testChargeFail(invMan, exdate, reg_units, INVOICING_DomainRenew, zone_id, reg_id);
    testChargeFail(invMan, exdate2, reg_units, INVOICING_DomainRenew, zone_id, reg_id);
}
*/

/*
// not thread safe - database data can change in the meantime
BOOST_AUTO_TEST_CASE( chargeDomainInsuffCredit )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainCreate, zone_cz_id);
    testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainCreate, zone_enum_id);
    //testChargeInsuffCredit(invMan.get(),  19, INVOICING_DomainCreate, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(),  19, INVOICING_DomainCreate, zone_enum_id);

    testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainCreate, zone_cz_id);
    testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainCreate, zone_enum_id);
    //testChargeInsuffCredit(invMan.get(), 19, INVOICING_DomainCreate, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(), 19, INVOICING_DomainCreate, zone_enum_id);

    testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainRenew, zone_cz_id);
    testChargeInsuffCredit(invMan.get(),  2, INVOICING_DomainRenew, zone_enum_id);
    //testChargeInsuffCredit(invMan.get(),  19, INVOICING_DomainRenew, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(),  19, INVOICING_DomainRenew, zone_enum_id);

    testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainRenew, zone_cz_id);
    testChargeInsuffCredit(invMan.get(), 2, INVOICING_DomainRenew, zone_enum_id);
    //testChargeInsuffCredit(invMan.get(), 19, INVOICING_DomainRenew, zone_cz_id);
    //testChargeInsuffCredit(invMan.get(), 19, INVOICING_DomainRenew, zone_enum_id);
}
*/

/*
// not thread safe - database data can change in the meantime
BOOST_AUTO_TEST_CASE( chargeDomain )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    Database::ID regid = createTestRegistrar();

    Money amount = std::string("20000.00");
    unsigned act_year = boost::gregorian::day_clock::universal_day().year();
    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    // add credit for new registrar
    Database::Date taxdate (act_year,1,1);
    Money out_credit;
    Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_cz_id//zone
                    , regid//registrar
                    , amount
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
    Fred::Credit::add_credit_to_invoice( regid,  zone_cz_id, out_credit, invoiceid);

    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    // add credit for new registrar
    Database::ID invoiceid2 = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_enum_id//zone
                    , regid//registrar
                    , amount
                    , boost::posix_time::ptime(taxdate)
                    , out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid2 != 0,true);
    Fred::Credit::add_credit_to_invoice( regid,  zone_enum_id, out_credit, invoiceid2);

    Database::Date exdate(act_year + 1, 1, 1);
    Database::Date exdate2(act_year + 5, 4, 30);

    testChargeSucc(invMan.get(), exdate, 24, INVOICING_DomainCreate, zone_cz_id, regid);
    testChargeSucc(invMan.get(), exdate, 24, INVOICING_DomainCreate, zone_enum_id, regid);
     testChargeSucc(invMan.get(), exdate2, 24, INVOICING_DomainCreate, zone_cz_id, regid);
    testChargeSucc(invMan.get(), exdate2, 24, INVOICING_DomainCreate, zone_enum_id, regid);
     testChargeSucc(invMan.get(), exdate, 24, INVOICING_DomainRenew, zone_cz_id, regid);
   testChargeSucc(invMan.get(), exdate, 24, INVOICING_DomainRenew, zone_enum_id, regid);
    testChargeSucc(invMan.get(), exdate2, 24, INVOICING_DomainRenew, zone_cz_id, regid);
   testChargeSucc(invMan.get(), exdate2, 24, INVOICING_DomainRenew, zone_enum_id, regid);


    testChargeSucc(invMan.get(), exdate, 19, INVOICING_DomainCreate, zone_cz_id, regid);
    testChargeSucc(invMan.get(), exdate, 19, INVOICING_DomainCreate, zone_enum_id, regid);


    testChargeSucc(invMan.get(), exdate2, 19, INVOICING_DomainCreate, zone_cz_id, regid);
    testChargeSucc(invMan.get(), exdate2, 19, INVOICING_DomainCreate, zone_enum_id, regid);


   testChargeSucc(invMan.get(), exdate, 19, INVOICING_DomainRenew, zone_cz_id, regid);
   testChargeSucc(invMan.get(), exdate, 19, INVOICING_DomainRenew, zone_enum_id, regid);

   testChargeSucc(invMan.get(), exdate2, 19, INVOICING_DomainRenew, zone_cz_id, regid);
   testChargeSucc(invMan.get(), exdate2, 19, INVOICING_DomainRenew, zone_enum_id, regid);

}
*/


/*
 * TODO rework
void testCharge2InvoicesWorker(Database::ID zone_id, unsigned op, unsigned period,
        Database::Date taxdate, Database::Date exdate, Money amount, bool should_succ)
{
    // registrar
    Database::ID regid = createTestRegistrar();

    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());
    // add credit - 2 invoices for the same zone:

    if(amount > Money("0")) {
        create2Invoices(invMan.get(), taxdate, zone_id, regid, amount);
    }

    ResultTestCharge res = testChargeWorker(invMan.get(), exdate, 12, INVOICING_DomainRenew, zone_id, regid);
    testChargeEval(res, should_succ);


    //db
    Database::Connection conn = Database::Manager::acquire();
    // zone IDs
    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];


    invMan->charge_operation_auto_price(
              "GeneralEppOperation"
              , zone_cz_id
              , regid
              , 0 //object_id
              , boost::posix_time::second_clock::local_time() //crdate //local timestamp
              , exdate.get() - boost::gregorian::months(1)//date_from //local date
              , exdate.get()// date_to //local date
              , Decimal ("900000"));
}

BOOST_AUTO_TEST_CASE( chargeDomain2Invoices )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    // TODO unused
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    unsigned operation = (unsigned) INVOICING_DomainRenew;
    unsigned period = 12;

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    Money op_price_cz = getOperationPrice(operation, zone_cz_id, period);
       // price for invoices so that 2 are not sufficient
       // cent_amount amount = op_price / 3;

       // price for invoices so that 2 are needed.
       // TODO hardcoded VAT - change
    Money amount = ( op_price_cz * Decimal("1.22")) / Decimal("2");

    testCharge2InvoicesWorker(zone_cz_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount, true);

    Money op_price_enum = getOperationPrice(operation, zone_cz_id, period);
       // price for invoices so that 2 are not sufficient
       // cent_amount amount = op_price / 3;

       // price for invoices so that 2 are needed.
       // TODO hardcoded VAT - change
    Money amount2 = ( op_price_enum * Decimal("1.22")) / Decimal("2");

    testCharge2InvoicesWorker(zone_enum_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount2, true);

}

BOOST_AUTO_TEST_CASE( chargeDomain2InvoicesNotSuff )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    // zone IDs
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    // TODO unused
    Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    unsigned operation = (unsigned) INVOICING_DomainRenew;
    unsigned period = 12;

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();


    Decimal op_price = getOperationPrice(operation, zone_cz_id, period);
       // price for invoices so that 2 are not sufficient
       // cent_amount amount = op_price / 3;

       // price for invoices so that 2 are needed.
       // TODO hardcoded VAT - change
    Decimal amount = ( op_price * Decimal("0.9")) / Decimal("2");

    testCharge2InvoicesWorker(zone_cz_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount, false);


// The same for ENUM
    Decimal op_price_enum = getOperationPrice(operation, zone_enum_id, period);
    Decimal amount2 = ( op_price_enum * Decimal("0.9")) / Decimal("2");

    testCharge2InvoicesWorker(zone_enum_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), amount2, false);

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
    unsigned period = 12;

    Database::Result check = conn.exec_params("SELECT price FROM price_list "
                                                  "WHERE operation_id = $1::integer "
                                                  "AND zone_id = $2::integer "
                                                  "AND valid_from < now()  "
                                                  "AND ( valid_to IS NULL OR valid_to > now() ) ",
                          Database::query_param_list(operation)
                                                  (zone_cz_id));

    if ( check.size() < 1 || check[0][0].isnull()
            || Money(std::string(check[0][0])) == Money("0"))
    {
        BOOST_ERROR("Cannot perform the test - operation not charged");
    }

    check = conn.exec_params("SELECT price FROM price_list "
                                                      "WHERE operation_id = $1::integer "
                                                      "AND zone_id = $2::integer "
                                                      "AND valid_from < now()  "
                                                      "AND ( valid_to IS NULL OR valid_to > now() ) ",
                              Database::query_param_list(operation)
                                                      (zone_enum_id));

    if ( check.size() < 1 || check[0][0].isnull() || Money(std::string(check[0][0])) == Money("0")) {
        BOOST_ERROR("Cannot perform the test - operation not charged");
    }

    unsigned act_year = boost::gregorian::day_clock::universal_day().year();

    testCharge2InvoicesWorker(zone_cz_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), Money("0"), false);

    testCharge2InvoicesWorker(zone_enum_id, operation, period,
            Database::Date(act_year,1,1), Database::Date(act_year + 5, 4, 30), Money("0"), false);

}
*/

// test createAccountInvoices with default values supplied by fred-admin




// thread-safe worker
// TODO use exdate parameter !!!
ResultTestCharge testCreateDomainDirectWorker(ccReg_EPP_i *epp_backend, Database::Date exdate_, unsigned reg_units,
        unsigned operation, Database::ID registrar_id,
        unsigned number, unsigned clientId, int zone_id)
{
    ResultTestCharge ret;

    ret.exdate = exdate_;
    ret.units = reg_units;
    ret.zone_id = zone_id;
    ret.regid = registrar_id;

    ret.success = true;

    ret.test_desc =
        (boost::format("test create domain operation charging - exdate: %1%, reg_units: %2%, operation: %3% , zone_id: %4%, registrar_id: %5%")
        % ret.exdate % ret.units % operation % zone_id % registrar_id).str();

    std::string time_string(TimeStamp::microsec());

    ret.credit_before = Money("0");
    ret.credit_after = Money("0");


    // do the operation
    if ( operation == INVOICING_DomainCreate ) {
        ccReg::Response_var r;

        ccReg::Period_str period;
        period.count = reg_units;
        period.unit = ccReg::unit_year;
        ccReg::EppParams epp_params;
        epp_params.requestID = clientId + number;
        epp_params.sessionID = clientId;
        epp_params.clTRID = "";
        epp_params.XML = "";
        CORBA::String_var crdate;
        // TODO use exdate param
        CORBA::String_var exdate;

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
            throw std::runtime_error("Error received");
        }

    } else if (operation == INVOICING_DomainRenew) {
        throw std::runtime_error("Not implemented");
    } else {
        throw std::runtime_error("Not implemented");
    }

    return ret;
}



class TestCreateDomainDirectThreadedWorker : public ThreadedTestWorker<ResultTestCharge, ChargeTestParams>
{
public:
    typedef ThreadedTestWorker<ResultTestCharge, ChargeTestParams>::ThreadedTestResultQueue queue_type;

    TestCreateDomainDirectThreadedWorker(unsigned number
             , boost::barrier* sb
             , std::size_t thread_group_divisor
             , queue_type* result_queue
             , ChargeTestParams params
                    )
        : ThreadedTestWorker<ResultTestCharge, ChargeTestParams>(number, sb, thread_group_divisor, result_queue, params)
            { }
    // this shouldn't throw
    ResultTestCharge run(const ChargeTestParams &p) {
       unsigned act_year = boost::gregorian::day_clock::universal_day().year();

       Database::Date exdate(act_year + 1, 1, 1);

       ResultTestCharge ret = testCreateDomainDirectWorker(p.epp_backend, exdate
               , 1, INVOICING_DomainCreate, p.regid,   number_, p.clientId, p.zone_id );

       // copy all the parametres to Result
       ret.number = number_;

       return ret;
    }

};


/*
class TestChargeThreadWorker : public ThreadedTestWorker<ResultTestCharge, ChargeTestParams>
{
public:
    typedef ThreadedTestWorker<ResultTestCharge, ChargeTestParams>::ThreadedTestResultQueue queue_type;

    TestChargeThreadWorker(unsigned number
             , boost::barrier* sb
             , std::size_t thread_group_divisor
             , queue_type* result_queue
             , ChargeTestParams params
                    )
        : ThreadedTestWorker<ResultTestCharge, ChargeTestParams>(number, sb, thread_group_divisor, result_queue, params)
            { }
    
    // this shouldn't throw
    ResultTestCharge run(const ChargeTestParams &p) {
       unsigned act_year = boost::gregorian::day_clock::universal_day().year();

       Database::Date exdate(act_year + 1, 1, 1);

       std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

       ResultTestCharge ret = testChargeWorker(invMan.get(), exdate, 24, INVOICING_DomainCreate,
               p.zone_id, p.regid);

       // copy all the parametres to Result
       ret.number = number_;

       return ret;
    }

};



BOOST_AUTO_TEST_CASE(chargeDomainThreaded)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    // Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];

    Database::ID regid = createTestRegistrar();

    Money amount ("20000.00");
    unsigned act_year = boost::gregorian::day_clock::universal_day().year();
    //manager
    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    // add credit for new registrar
    Database::Date taxdate (act_year,1,1);
    Money out_credit;
    Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                    , zone_cz_id//zone
                    , regid//registrar
                    , amount
                    , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    Fred::Credit::add_credit_to_invoice( regid,  zone_cz_id, out_credit, invoiceid);

    ChargeTestParams params;
    params.zone_id = zone_cz_id ;
    params.regid = regid;

// TODO this is it ....
    threadedTest< TestChargeThreadWorker> (params, &testChargeEvalSucc);

}
*/

BOOST_AUTO_TEST_CASE(createDomainDirectThreaded)
{
    
     std::auto_ptr<ccReg_EPP_i> epp = create_epp_backend_object();

     std::string time_string(TimeStamp::microsec());
     std::string noregistrar_handle(std::string("REG-NOTEXISTS")+time_string);
    // registrar
     Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
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
     std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

     Database::Date taxdate (act_year,1,1);
     Money out_credit;
     Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                     , zone_cz_id//zone
                     , registrar_inv_id//registrar
                     , amount
                     , boost::posix_time::ptime(taxdate), out_credit);//price
     BOOST_CHECK_EQUAL(invoiceid != 0,true);
     Fred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);



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
            threadedTest< TestCreateDomainDirectThreadedWorker> (params, &testCreateDomainEvalSucc));

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

    std::auto_ptr<ccReg_EPP_i> myccReg_EPP_i = create_epp_backend_object();

   // ------------------ login

   CORBA::Long clientId = epp_backend_login(myccReg_EPP_i.get(), reg_handle);

   testCreateDomainDirectWorker(myccReg_EPP_i.get(), Database::Date(), 1,
          INVOICING_DomainCreate, regid, 1, clientId, zone_cz_id);
}

BOOST_AUTO_TEST_CASE(testCreateDomainEPPNoCORBA)
{
    // registrar
    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    unsigned long long registrar_inv_id = registrar->getId();
    std::string registrar_handle = registrar->getHandle();

     // ### add credit
     Money amount ("90000.00");
     unsigned act_year = boost::gregorian::day_clock::universal_day().year();

     Database::Connection conn = Database::Manager::acquire();
     Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

     // Database::ID zone_enum_id = conn.exec("select id from zone where fqdn='0.2.4.e164.arpa'")[0][0];
     std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

     Database::Date taxdate (act_year,1,1);
     Money out_credit;
     Database::ID invoiceid = invMan->createDepositInvoice(taxdate //taxdate
                     , zone_cz_id//zone
                     , registrar_inv_id//registrar
                     , amount
                     , boost::posix_time::ptime(taxdate), out_credit);//price
     BOOST_CHECK_EQUAL(invoiceid != 0,true);
     Fred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

     createCzDomain(registrar->getId(), registrar_handle);
}




BOOST_AUTO_TEST_CASE(make_debt)
{
    const Decimal postpaid_op_price ("100000");
    // assuming that price of request is 0.1
    const Decimal unit_price ("0.1");
    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();

    Database::ID reg_id = registrar->getId();
    Database::ID zone_id = get_zone_cz_id();

    date today = day_clock::local_day();
    Database::Date taxdate(today);

    Money price = std::string("5000.00");//money
    Money out_credit;

    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    unsigned long long  invoiceid = invMan->createDepositInvoice(taxdate//taxdate
        , zone_id//zone
        , reg_id//registrar
        , price
        , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    Fred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid);

    Decimal credit_before = get_credit(reg_id, zone_id);

    BOOST_CHECK(
            invMan->charge_operation_auto_price(
         "GeneralEppOperation"
         , zone_id
         , reg_id
         , 0 //object_id
         , boost::posix_time::second_clock::local_time() //crdate //local timestamp
         , today - boost::gregorian::months(1)//date_from //local date
         , today // date_to //local date
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
    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    Database::ID zone_id = get_zone_cz_id();

    Database::ID reg_id = registrar->getId();

    date today = day_clock::local_day();
    Database::Date taxdate(today);

    Money price = std::string("5000.00");//money
    Money out_credit;

    std::auto_ptr<Fred::Invoicing::Manager> invMan(Fred::Invoicing::Manager::create());

    unsigned long long  invoiceid = invMan->createDepositInvoice(taxdate//taxdate
        , zone_id//zone
        , reg_id//registrar
        , price
        , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    Fred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid);

// assuming that price of request is 0.1
    BOOST_CHECK(
            invMan->charge_operation_auto_price(
         "GeneralEppOperation"
         , zone_id
         , reg_id
         , 0 //object_id
         , boost::posix_time::second_clock::local_time() //crdate //local timestamp
         , today - boost::gregorian::months(1)//date_from //local date
         , today // date_to //local date
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
    Fred::Credit::add_credit_to_invoice( reg_id,  zone_id, out_credit, invoiceid2);

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
                       Decimal(vat_ratio * price).round(2, MPD_ROUND_HALF_UP)
                     + Decimal(vat_ratio * recharge).round(2, MPD_ROUND_HALF_UP)
                     - unit_price * postpaid_operation
        ).round(2, MPD_ROUND_HALF_UP);

    vat_ratio * (price + recharge) - unit_price * postpaid_operation;
    Decimal credit_after = get_credit(reg_id, zone_id);

    BOOST_CHECK(credit_after == estimated_credit);

    // TODO maybe try to create a domain, it shouldn't work
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
Fred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);
}
*/



bool default_charge_request_fee(Fred::Invoicing::Manager *manager, Database::ID reg_id)
{
    boost::gregorian::date local_today = boost::gregorian::day_clock::local_day();
    date poll_msg_period_to = date(local_today.year(), local_today.month(), 1);

    return manager->chargeRequestFee(reg_id, poll_msg_period_to);
}


BOOST_AUTO_TEST_CASE(test_charge_request)
{
    Decimal price ("10000");
    Database::ID zone_cz_id = get_zone_cz_id();

    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::auto_ptr<Fred::Invoicing::Manager> invMan(
        Fred::Invoicing::Manager::create());

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
    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();

    std::auto_ptr<Fred::Invoicing::Manager> invMan(
       Fred::Invoicing::Manager::create());

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
    Fred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    Database::ID reg_id = registrar->getId();

    std::auto_ptr<Fred::Invoicing::Manager> invMan(
       Fred::Invoicing::Manager::create());

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

    BOOST_CHECK_EXCEPTION(default_charge_request_fee(invMan.get(), reg_id), std::runtime_error, check_dummy);

}

BOOST_AUTO_TEST_SUITE_END();//TestInvoice

