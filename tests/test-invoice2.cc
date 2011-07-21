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

#include "setup_server_decl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_rifd_args.h"

#include "fredlib/registrar.h"
#include "fredlib/invoicing/invoice.h"
#include "mailer_manager.h"
#include "time_clock.h"
#include "file_manager_client.h"
#include "fredlib/banking/bank_common.h"
#include "corba/Admin.hh"
#include "corba/EPP.hh"
#include "epp/epp_impl.h"

#include "util/decimal/decimal.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

using namespace Fred::Invoicing;

class fixture_exception_handler
{
    std::string message_prefix;
    bool rethrow;
public:
    virtual ~fixture_exception_handler(){}

    fixture_exception_handler(const char* _message_prefix, bool _rethrow = true)
            : message_prefix(_message_prefix)
            , rethrow(_rethrow)
    {}
    void operator ()()
    {
        try
        {
            throw;
        }
        catch(CORBA::TRANSIENT& ex)
        {
            try
            {
                BOOST_TEST_MESSAGE( (message_prefix+" CORBA::TRANSIENT: " + ex._name()) );
            }
            catch(...) { if(rethrow) throw; }
        }
        catch(CORBA::SystemException& ex)
        {
            try
            {
                BOOST_TEST_MESSAGE( (message_prefix+" CORBA::SystemException: " + ex._name()) );
            }
            catch(...){ if(rethrow) throw; }
        }
        catch(CORBA::Exception& ex)
        {
            try
            {
                BOOST_TEST_MESSAGE( (message_prefix+" CORBA::Exception: " + ex._name()) );
            }
            catch(...){ if(rethrow) throw; }
        }
        catch(omniORB::fatalException& fe)
        {
            try
            {
                BOOST_TEST_MESSAGE( (
                    message_prefix+" omniORB::fatalException: "
                    + std::string("  file: ") + std::string(fe.file())
                    + std::string("  line: ") + boost::lexical_cast<std::string>(fe.line())
                    + std::string("  mesg: ") + std::string(fe.errmsg()) ) );
            }
            catch(...){ if(rethrow) throw; }
        }
        catch (std::exception &ex)
        {
            try
            {
                BOOST_TEST_MESSAGE( (message_prefix+" std::exception: " + ex.what()) );
            }
            catch(...){ if(rethrow) throw; }
        }
        catch (...)
        {
            try
            {
                BOOST_TEST_MESSAGE( (message_prefix+" unknown exception") );
            }
            catch(...){ if(rethrow) throw; }
        }
    }//operator()
};//class fixture_exception_handler

struct db_conn_acquire_fixture
{
    std::auto_ptr<Database::Connection> connp;
    db_conn_acquire_fixture()
    try
        : connp( new Database::Connection((Database::Manager::acquire())) )
    {}
    catch(...)
    {
        fixture_exception_handler("db_conn_acquire_fixture ctor exception", true)();
    }
protected:
    ~db_conn_acquire_fixture()
    {
        try
        {   //Database::Connection dtor may throw, too bad
            connp.reset(static_cast<Database::Connection*>(0));
        }
        catch(...)
        {
            fixture_exception_handler("db_conn_acquire_fixture dtor exception1", false)();
        }
    }
};

static std::string zone_registrar_credit_query (
        "select COALESCE(SUM(credit), 0) from invoice "
        " where zone = $1::bigint and registrarid =$2::bigint "
        " group by registrarid, zone ");

static int start_year = 2005;

struct get_operation_price_result_fixture
        : virtual db_conn_acquire_fixture
{
    Database::Result operation_price_result;//operation, price,period, zone.fqdn

    get_operation_price_result_fixture()
    try
    : operation_price_result(connp->exec(
        "SELECT enum_operation.operation, price , period, zone.fqdn FROM price_list "
        " join zone on price_list.zone = zone.id "
        " join enum_operation on price_list.operation = enum_operation.id "
        " WHERE valid_from < 'now()' "
        " and ( valid_to is NULL or valid_to > 'now()' ) "
        " order by valid_from desc "))

    {}
    catch(...)
    {
        fixture_exception_handler("get_operation_price_result_fixture ctor exception", true)();
    }

protected:
    ~get_operation_price_result_fixture()
    {}
};

struct corba_init_fixture
{
    corba_init_fixture()
    {
        try
        {
            //corba config
            FakedArgs fa = CfgArgs::instance()->fa;
            //conf pointers
            HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                        get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
            CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
                    , ns_args_ptr->nameservice_host
                    , ns_args_ptr->nameservice_port
                    , ns_args_ptr->nameservice_context);
        }
        catch(...)
        {
            fixture_exception_handler("corba_init_fixture ctor exception", true)();
        }
    }
protected:
    ~corba_init_fixture()
    {}
};

struct invoice_manager_fixture
{
    std::auto_ptr<Fred::Invoicing::Manager> invMan;

    invoice_manager_fixture()
    try
    : invMan (Fred::Invoicing::Manager::create())
    {}
    catch(...)
    {
        fixture_exception_handler("invoice_manager_fixture ctor exception", true)();
    }

protected:
    ~invoice_manager_fixture()
    {}
};

struct create_zone_fixture
{
    Fred::Zone::Manager::ZoneManagerPtr zoneMan;
    std::string test_zone_fqdn;
    std::string test_enum_zone_fqdn;
    std::string test_zeroprice_zone_fqdn;
    std::string test_zeroprice_enum_zone_fqdn;
    std::string test_zeroperiod_zone_fqdn;
    std::string test_zeroperiod_enum_zone_fqdn;
    std::string test_differentperiod_zone_fqdn;
    std::string test_differentperiod_enum_zone_fqdn;
    std::string test_nooperationprice_zone_fqdn;
    std::string test_nooperationprice_enum_zone_fqdn;

    create_zone_fixture()
    try
    : zoneMan (Fred::Zone::Manager::create())
      , test_zone_fqdn(std::string("zone")+TimeStamp::microsec())
      , test_enum_zone_fqdn(std::string("zone")+TimeStamp::microsec()+".e164.arpa")
      , test_zeroprice_zone_fqdn(std::string("zone-zeroprice")+TimeStamp::microsec())
      , test_zeroprice_enum_zone_fqdn(std::string("zone-zeroprice")+TimeStamp::microsec()+".e164.arpa")
      , test_zeroperiod_zone_fqdn(std::string("zone-zeroperiod")+TimeStamp::microsec())
      , test_zeroperiod_enum_zone_fqdn(std::string("zone-zeroperiod")+TimeStamp::microsec()+".e164.arpa")
      , test_differentperiod_zone_fqdn(std::string("zone-differentperiod")+TimeStamp::microsec())
      , test_differentperiod_enum_zone_fqdn(std::string("zone-differentperiod")+TimeStamp::microsec()+".e164.arpa")
      , test_nooperationprice_zone_fqdn(std::string("zone-nooperationprice")+TimeStamp::microsec())
      , test_nooperationprice_enum_zone_fqdn(std::string("zone-nooperationprice")+TimeStamp::microsec()+".e164.arpa")
    {
        zoneMan->addZone(test_zone_fqdn);
        zoneMan->addZone(test_enum_zone_fqdn);
        zoneMan->addZone(test_zeroprice_zone_fqdn);
        zoneMan->addZone(test_zeroprice_enum_zone_fqdn);
        zoneMan->addZone(test_zeroperiod_zone_fqdn);
        zoneMan->addZone(test_zeroperiod_enum_zone_fqdn);
        zoneMan->addZone(test_differentperiod_zone_fqdn);
        zoneMan->addZone(test_differentperiod_enum_zone_fqdn);
        zoneMan->addZone(test_nooperationprice_zone_fqdn);
        zoneMan->addZone(test_nooperationprice_enum_zone_fqdn);
    }
    catch(...)
    {
        fixture_exception_handler("create_zone_fixture ctor exception", true)();
    }

protected:
    ~create_zone_fixture()
    {}
};

struct zone_fixture
    : virtual db_conn_acquire_fixture
      , virtual create_zone_fixture
{
        Database::Result zone_result;

    zone_fixture()
    try
    : zone_result(connp->exec_params(
        "select id, fqdn from zone where fqdn = $1::text or fqdn = $2::text "
            " or fqdn = $3::text or fqdn = $4::text "
            " or fqdn = $5::text or fqdn = $6::text "
            " or fqdn = $7::text or fqdn = $8::text "
            " or fqdn = $9::text or fqdn = $10::text "
        , Database::query_param_list (test_zone_fqdn)(test_enum_zone_fqdn)
        (test_zeroprice_zone_fqdn)(test_zeroprice_enum_zone_fqdn)
        (test_zeroperiod_zone_fqdn)(test_zeroperiod_enum_zone_fqdn)
        (test_differentperiod_zone_fqdn)(test_differentperiod_enum_zone_fqdn)
        (test_nooperationprice_zone_fqdn)(test_nooperationprice_enum_zone_fqdn)
        ))
    {}
    catch(...)
    {
        fixture_exception_handler("zone_fixture ctor exception", true)();
    }

protected:
    ~zone_fixture()
    {}
};


struct operation_price_fixture
    : virtual db_conn_acquire_fixture
    , virtual zone_fixture
{
    operation_price_fixture()
    {
        try
        {
            std::string insert_operation_price =
                "insert into price_list (zone, operation, valid_from, valid_to, price, period) "
                " values ( (select id from zone where fqdn = $1::text limit 1) " //cz
                " , (select id from enum_operation where operation = $2::text ) " //CreateDomain,
                " , $3::timestamp , $4::timestamp " //utc timestamp, valid_to might be null
                " , $5::numeric(10,2) , $6::integer)" ; // numeric(10,2) must round to an absolute value less than 10^8 (max +/-99999999.99)

            //test_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")("2009-12-31 23:00:00")
                ("0")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zone_fqdn)
                ("CreateDomain")
                ("2009-12-31 23:00:00")(Database::QPNull)
                ("0")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")("2009-12-31 23:00:00")
                ("190")("12"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zone_fqdn)
                ("RenewDomain")
                ("2009-12-31 23:00:00")("2011-01-31 23:00:00")
                ("155")("12"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zone_fqdn)
                ("RenewDomain")
                ("2011-01-31 23:00:00")(Database::QPNull)
                ("140")("12"));

            //test_enum_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_enum_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_enum_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("1")("12"));

            //test_zeroprice_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroprice_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroprice_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("12"));

            //test_zeroprice_enum_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroprice_enum_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroprice_enum_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("12"));

            //test_zeroperiod_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroperiod_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroperiod_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("140.11")("0"));

            //test_zeroperiod_enum_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroperiod_enum_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("100.11")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroperiod_enum_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("0"));

            //test_differentperiod_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_differentperiod_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("0"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_differentperiod_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("140.11")("3"));

            //test_differentperiod_enum_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_differentperiod_enum_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("100.11")("15"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_differentperiod_enum_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("200.22")("15"));
        }
        catch(...)
        {
            fixture_exception_handler("operation_price_fixture ctor exception", true)();
        }
    }
protected:
    ~operation_price_fixture()
    {
        try
        {
            connp->exec("update price_list set price = price - 0.11 where zone = 1 and operation = 2");
        }
        catch(...)
        {
            fixture_exception_handler("operation_price_fixture dtor exception", false)();
        }
    }
};


struct try_insert_invoice_prefix_fixture
    : virtual zone_fixture
    , virtual invoice_manager_fixture
{
    try_insert_invoice_prefix_fixture()
    {
        try
        {
            BOOST_TEST_MESSAGE(std::string("zone_result.size(): ")+boost::lexical_cast<std::string>(zone_result.size()));

            for (std::size_t zone_i = 0; zone_i < zone_result.size() ; ++zone_i)
            {
                unsigned long long zone_id(zone_result[zone_i][0]);

                for (int year = start_year; year < boost::gregorian::day_clock::universal_day().year() + 3 ; ++year)
                {
                    try{
                    invMan->insertInvoicePrefix(
                             zone_id//zoneId
                            , 0//type
                            , year//year
                            , year*zone_id*100000//prefix
                            );
                    }catch(...){}

                    try{
                    invMan->insertInvoicePrefix(
                            zone_id//zoneId
                            , 1//type
                            , year//year
                            , year*zone_id*100000 + 10000//prefix
                            );
                    }catch(...){}
                }//for
            }//for zone_i
        }
        catch(...)
        {
            fixture_exception_handler("try_insert_invoice_prefix_fixture ctor exception", true)();
        }
    }

protected:
    ~try_insert_invoice_prefix_fixture()
    {}
};


struct registrar_fixture
    : virtual db_conn_acquire_fixture
      , virtual zone_fixture
{

    Database::Result registrar_result;


    unsigned long long create_test_registrar(const std::string& registrar_handle, bool vat = true)
    {
        //BOOST_TEST_MESSAGE( std::string("Create test registrar: ")+registrar_handle);
        Fred::Registrar::Manager::AutoPtr regMan
                 = Fred::Registrar::Manager::create(DBSharedPtr());
        Fred::Registrar::Registrar::AutoPtr registrar
                = regMan->createRegistrar();
        registrar->setName(registrar_handle+"_Name");
        registrar->setOrganization(registrar_handle+"_Organization");
        registrar->setHandle(registrar_handle);
        registrar->setCountry("CZ");
        registrar->setStreet1(registrar_handle+"_Street1");
        registrar->setVat(vat);
        Fred::Registrar::ACL* registrar_acl = registrar->newACL();
        registrar_acl->setCertificateMD5("");
        registrar_acl->setPassword("");
        registrar->save();
        return registrar->getId();
    }//create_test_registrar


    registrar_fixture()
    {
        std::string registrar_query(
                "select id, handle "
                " from registrar "
                " where 1=2 "
                );
        Database::QueryParams registrar_query_params;

        try
        {
            std::string time_string(TimeStamp::microsec());


            for(int in_zone = 0; in_zone < 2; ++in_zone)
            {
                std::string registrar_handle
                    = std::string("REG-FRED_NOCREDIT_");
                registrar_handle += (in_zone ? "INZONE_" : "NOTINZONE_");
                registrar_handle += "VAT_";
                registrar_handle += time_string;
                registrar_query_params.push_back(
                    create_test_registrar(registrar_handle));
                registrar_query += " or id=$"
                    +boost::lexical_cast<std::string>(registrar_query_params.size())
                    +"::bigint";

                if(in_zone)//add registrar into zone
                for(std::size_t i = 0 ; i < zone_result.size(); ++i)
                {
                    std::string rzzone (zone_result[i][1]);
                    Database::Date rzfromDate;
                    Database::Date rztoDate;

                    Fred::Registrar::addRegistrarZone(registrar_handle, rzzone, rzfromDate, rztoDate);
                }
            }//for nocredit

            //low credit registrars
            for(int invoice_num = 1; invoice_num < 3; ++invoice_num)
            {
                for(int vat = 0; vat < 2; ++vat )
                {
                    std::string registrar_handle("REG-FRED_");
                    registrar_handle += (invoice_num==1 ? "LOWCREDIT1_" : "LOWCREDIT2_");
                    registrar_handle += "INZONE_";
                    registrar_handle += (vat ? "VAT_" : "NOVAT_");
                    registrar_handle += time_string;

                    unsigned long long registrar_id;

                    //BOOST_TEST_MESSAGE( std::string("Registrar: ")+registrar_handle);

                    registrar_id = create_test_registrar(registrar_handle, vat);

                    registrar_query_params.push_back(registrar_id);
                    registrar_query += " or id=$"
                            +boost::lexical_cast<std::string>(registrar_query_params.size())
                            +"::bigint";

                    for(std::size_t i = 0 ; i < zone_result.size(); ++i)
                    {
                        std::string rzzone (zone_result[i][1]);
                        Database::Date rzfromDate;
                        Database::Date rztoDate;

                        Fred::Registrar::addRegistrarZone(registrar_handle, rzzone, rzfromDate, rztoDate);
                    }

                }//for vat
            }//for invoice_num

            //big credit registrar
            for(int vat = 0; vat < 2; ++vat )
            {
                std::string registrar_handle("REG-FRED_");
                registrar_handle += "BIGCREDIT_";
                registrar_handle += "INZONE_";
                registrar_handle += (vat ? "VAT_" : "NOVAT_");
                registrar_handle += time_string;

                unsigned long long registrar_id;

                //BOOST_TEST_MESSAGE( std::string("Registrar: ")+registrar_handle);

                registrar_id = create_test_registrar(registrar_handle, vat);

                registrar_query_params.push_back(registrar_id);
                registrar_query += " or id=$"
                        +boost::lexical_cast<std::string>(registrar_query_params.size())
                        +"::bigint";

                for(std::size_t i = 0 ; i < zone_result.size(); ++i)
                {
                    std::string rzzone (zone_result[i][1]);
                    Database::Date rzfromDate;
                    Database::Date rztoDate;

                    Fred::Registrar::addRegistrarZone(registrar_handle, rzzone, rzfromDate, rztoDate);
                }
            }//for vat

            //negative credit registrar
            for(int vat = 0; vat < 2; ++vat )
            {
                std::string registrar_handle("REG-FRED_");
                registrar_handle += "NEGCREDIT_";
                registrar_handle += "INZONE_";
                registrar_handle += (vat ? "VAT_" : "NOVAT_");
                registrar_handle += time_string;

                unsigned long long registrar_id;

                //BOOST_TEST_MESSAGE( std::string("Registrar: ")+registrar_handle);

                registrar_id = create_test_registrar(registrar_handle, vat);

                registrar_query_params.push_back(registrar_id);
                registrar_query += " or id=$"
                        +boost::lexical_cast<std::string>(registrar_query_params.size())
                        +"::bigint";

                for(std::size_t i = 0 ; i < zone_result.size(); ++i)
                {
                    std::string rzzone (zone_result[i][1]);
                    Database::Date rzfromDate;
                    Database::Date rztoDate;

                    Fred::Registrar::addRegistrarZone(registrar_handle, rzzone, rzfromDate, rztoDate);
                }

            }//for vat


            for(int in_zone = 0; in_zone < 2; ++in_zone)
            {
                for(int vat = 0; vat < 2; ++vat )
                {
                    std::string registrar_handle("REG-FRED_");
                    registrar_handle += (in_zone ? "INZONE_" : "NOTINZONE_");
                    registrar_handle += (vat ? "VAT_" : "NOVAT_");
                    registrar_handle += time_string;

                    unsigned long long registrar_id;

                    //BOOST_TEST_MESSAGE( std::string("Registrar: ")+registrar_handle);

                    registrar_id = create_test_registrar(registrar_handle, vat);

                    registrar_query_params.push_back(registrar_id);
                    registrar_query += " or id=$"
                            +boost::lexical_cast<std::string>(registrar_query_params.size())
                            +"::bigint";

                    if(in_zone)//add registrar into zone
                    for(std::size_t i = 0 ; i < zone_result.size(); ++i)
                    {
                        std::string rzzone (zone_result[i][1]);
                        Database::Date rzfromDate;
                        Database::Date rztoDate;

                        Fred::Registrar::addRegistrarZone(registrar_handle, rzzone, rzfromDate, rztoDate);
                    }

                }//for vat
            }//for in_zone

            registrar_query += " order by id";

            registrar_result=connp->exec_params(registrar_query, registrar_query_params);
            //BOOST_TEST_MESSAGE( std::string("Registrars query: ") + registrar_query+ " size: " +(boost::lexical_cast<std::string>(registrar_result.size())) );
            for(std::size_t i = 0; i < registrar_result.size(); ++i)
            {
                BOOST_CHECK(registrar_query_params[i].get_data().compare(std::string(registrar_result[i][0])) == 0);
                //BOOST_TEST_MESSAGE(std::string("id: ") + std::string(registrar_result[i][0])+ " handle: " + std::string(registrar_result[i][1]));
            }
        }
        catch(...)
        {
            fixture_exception_handler("registrar_fixture ctor exception", true)();
        }
    }
protected:
    ~registrar_fixture()
    {}
};


struct create_deposit_invoice_fixture
  : virtual zone_fixture
  , virtual registrar_fixture
  , virtual invoice_manager_fixture
{
    std::vector<unsigned long long> deposit_invoice_id_vect;

    create_deposit_invoice_fixture()
    {
        try
        {
            for(std::size_t registrar_i = 0; registrar_i < registrar_result.size(); ++registrar_i)
            {
                //if nocredit registrar
                if(std::string(registrar_result[registrar_i][1]).find("NOCREDIT") != std::string::npos)
                    continue;//don't add credit

                unsigned long long registrar_id (registrar_result[registrar_i][0]);

                //if lowcredit registrar
                if(std::string(registrar_result[registrar_i][1]).find("LOWCREDIT1") != std::string::npos)
                {
                    for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                    {
                        unsigned long long zone_id ( zone_result[zone_i][0]);

                        //credit before
                        Database::Result credit0_res = connp->exec_params(zone_registrar_credit_query
                                , Database::query_param_list(zone_id)(registrar_id));
                        std::string credit0_from_query;
                        if(credit0_res.size() ==  1 && credit0_res[0].size() == 1)
                            credit0_from_query = std::string(credit0_res[0][0]);

                        //add credit
                        int year = start_year;
                        {
                            unsigned long long invoiceid = 0;
                            Database::Date taxdate;
                            taxdate = Database::Date(year,1,1);
                            unsigned long price = 1000UL;//cents
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price);//price
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);
                        }

                        //credit after
                        Database::Result credit1_res = connp->exec_params(zone_registrar_credit_query
                                , Database::query_param_list(zone_id)(registrar_id));
                        std::string credit1_from_query;
                        if(credit0_res.size() ==  1 && credit1_res[0].size() == 1)
                            credit1_from_query = std::string(credit1_res[0][0]);

                    }//for zone_i
                    continue;//don't add other credit
                }//if LOWCREDIT1

                if(std::string(registrar_result[registrar_i][1]).find("LOWCREDIT2") != std::string::npos)
                {
                    for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                    {
                        unsigned long long zone_id ( zone_result[zone_i][0]);
                        int year = start_year;
                        {
                            unsigned long long invoiceid = 0;
                            Database::Date taxdate;
                            taxdate = Database::Date(year,1,1);
                            unsigned long price = 1000UL;//cents
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price);//price
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                            taxdate = Database::Date(year,12,31);
                            price = 1000UL;//cents
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price);//price
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                        }//createDepositInvoice
                    }//for zone_i
                    continue;//don't add other credit
                }//if LOWCREDIT2

                //big credit
                if(std::string(registrar_result[registrar_i][1]).find("BIGCREDIT") != std::string::npos)
                {
                    for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                    {
                        unsigned long long zone_id ( zone_result[zone_i][0]);
                        for (int year = start_year; year < boost::gregorian::day_clock::universal_day().year() + 3 ; ++year)
                        {
                            unsigned long long invoiceid = 0;
                            Database::Date taxdate;
                            taxdate = Database::Date(year,1,1);
                            unsigned long price = 3000000000UL;//cents
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price);//price
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);
                        }//createDepositInvoice
                    }//for zone_i
                    continue;//don't add other credit
                }//if BIGCREDIT

                //negative credit
                if(std::string(registrar_result[registrar_i][1]).find("NEGCREDIT") != std::string::npos)
                {
                    for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                    {
                        unsigned long long zone_id ( zone_result[zone_i][0]);
                        for (int year = start_year; year < boost::gregorian::day_clock::universal_day().year() + 3 ; ++year)
                        {
                            unsigned long long invoiceid = 0;
                            Database::Date taxdate;
                            taxdate = Database::Date(year,1,1);
                            signed long long price = -3000000000LL;//cents
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price);//price
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);
                        }//createDepositInvoice
                    }//for zone_i
                    continue;//don't add other credit
                }//if NEGCREDIT

                //add a lot of credit
                for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                {
                    unsigned long long zone_id ( zone_result[zone_i][0]);

                    for (int year = start_year; year < boost::gregorian::day_clock::universal_day().year() + 3 ; ++year)
                    {
                        unsigned long long invoiceid = 0;
                        Database::Date taxdate;

                        taxdate = Database::Date(year,1,1);
                        unsigned long price = 5000000UL;//cents

                        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                , zone_id//zone
                                , registrar_id//registrar
                                , price);//price
                        BOOST_CHECK_EQUAL(invoiceid != 0,true);

                        if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

                        taxdate = Database::Date(year,6,10);
                        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                , zone_id//zone
                                , registrar_id//registrar
                                , price);//price
                        BOOST_CHECK_EQUAL(invoiceid != 0,true);

                        if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

                        taxdate = Database::Date(year,12,31);
                        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                , zone_id//zone
                                , registrar_id//registrar
                                , price);//price
                        BOOST_CHECK_EQUAL(invoiceid != 0,true);

                        if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

                    }//for createDepositInvoice
                }//for zone_i
            }//for registrar
        }
        catch(...)
        {
            fixture_exception_handler("create_deposit_invoice_fixture ctor exception", true)();
        }
}

protected:
    ~create_deposit_invoice_fixture()
    {}
};


struct Case_invoice_registrar1_Fixture
    : virtual db_conn_acquire_fixture
    , virtual operation_price_fixture
    , virtual get_operation_price_result_fixture
    , virtual corba_init_fixture
    , virtual zone_fixture
    , virtual registrar_fixture
    , virtual try_insert_invoice_prefix_fixture
    , virtual create_deposit_invoice_fixture
{
    int j;

    Case_invoice_registrar1_Fixture()
        : j( 0 )
    {
        BOOST_TEST_MESSAGE( "Case_invoice_registrar1_Fixture setup fixture" );
    }

    ~Case_invoice_registrar1_Fixture()
    {
        BOOST_TEST_MESSAGE( "Case_invoice_registrar1_Fixture teardown fixture" );
    }
};

BOOST_AUTO_TEST_SUITE( TestInvoice2 )

const std::string server_name = "test-invoice2";


BOOST_FIXTURE_TEST_CASE( invoice_registrar1, Case_invoice_registrar1_Fixture )
{
    //db
    BOOST_CHECK(static_cast<bool>(connp->exec("select 1")[0][0]));
}

BOOST_AUTO_TEST_SUITE_END();//TestInvoice2
