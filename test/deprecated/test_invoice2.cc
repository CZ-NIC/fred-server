/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include "src/util/setup_server_decl.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"

#include "libfred/opcontext.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/epp_auth/add_registrar_epp_auth.hh"
#include "libfred/registrar/zone_access/add_registrar_zone_access.hh"

#include "src/deprecated/libfred/invoicing/invoice.hh"

#include "src/bin/corba/mailer_manager.hh"
#include "src/util/time_clock.hh"
#include "src/deprecated/libfred/credit.hh"
#include "src/bin/corba/file_manager_client.hh"
#include "src/deprecated/libfred/banking/bank_common.hh"
#include "src/bin/corba/Admin.hh"
#include "src/bin/corba/EPP.hh"
#include "src/bin/corba/epp/epp_impl.hh"
#include "src/util/types/money.hh"

#include "util/decimal/decimal.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

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

using namespace ::LibFred::Invoicing;

class fixture_exception_handler
{
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
private:
    std::string message_prefix;
    bool rethrow;
};//class fixture_exception_handler

struct db_conn_acquire_fixture
{
    std::unique_ptr<Database::Connection> connp;
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
    "SELECT credit FROM registrar_credit"
    " WHERE zone_id = $1::bigint AND registrar_id =$2::bigint");

static int start_year = 2005;

struct get_operation_price_result_fixture
        : virtual db_conn_acquire_fixture
{
    Database::Result operation_price_result;//operation, price,period, zone.fqdn

    get_operation_price_result_fixture()
    try
    : operation_price_result(connp->exec(
        "SELECT enum_operation.operation, price, quantity, zone.fqdn "
        "FROM price_list "
        "JOIN zone on price_list.zone_id = zone.id "
        "JOIN enum_operation on price_list.operation_id = enum_operation.id "
        "WHERE valid_from < 'now()' AND (valid_to is NULL OR valid_to > 'now()') "
        "ORDER BY valid_from DESC"))

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
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan;

    invoice_manager_fixture()
    try
    : invMan (::LibFred::Invoicing::Manager::create())
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
    ::LibFred::Zone::Manager::ZoneManagerPtr zoneMan;
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
    : zoneMan (::LibFred::Zone::Manager::create())
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
                "insert into price_list (zone_id, operation_id, valid_from, valid_to, price, quantity) "
                " values ( (select id from zone where fqdn = $1::text limit 1) " //cz
                " , (select id from enum_operation where operation = $2::text ) " //CreateDomain,
                " , $3::timestamp , $4::timestamp " //utc timestamp, valid_to might be null
                " , $5::numeric(10,2) , $6::integer)" ; // numeric(10,2) must round to an absolute value less than 10^8 (max +/-99999999.99)

            //test_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")("2009-12-31 23:00:00")
                ("0")("1"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zone_fqdn)
                ("CreateDomain")
                ("2009-12-31 23:00:00")(Database::QPNull)
                ("0")("1"));
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
                ("0")("1"));
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
                ("0")("1"));
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
                ("0")("1"));
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
                ("0")("1"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroperiod_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("140.11")("1"));

            //test_zeroperiod_enum_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroperiod_enum_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("100.11")("1"));
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_zeroperiod_enum_zone_fqdn)
                ("RenewDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("1"));

            //test_differentperiod_zone_fqdn create and renew price
            connp->exec_params(insert_operation_price
                , Database::query_param_list (test_differentperiod_zone_fqdn)
                ("CreateDomain")
                ("2007-09-29 19:00:00")(Database::QPNull)
                ("0")("1"));
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
            connp->exec("update price_list set price = price - 0.11 where zone_id = 1 and operation_id = 2");
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
        ::LibFred::OperationContextCreator ctx;
        const unsigned long long id = ::LibFred::CreateRegistrar(registrar_handle)
                .set_name(registrar_handle + "_Name")
                .set_organization(registrar_handle+"_Organization")
                .set_country("CZ")
                .set_street1(registrar_handle+"_Street1")
                .set_vat_payer(vat)
                .exec(ctx);
        ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(registrar_handle, "", "").exec(ctx);
        ctx.commit_transaction();
        return id;
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

                    ::LibFred::OperationContextCreator ctx;
                    ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, rzzone, rzfromDate).exec(ctx);
                    ctx.commit_transaction();
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

                        ::LibFred::OperationContextCreator ctx;
                        ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, rzzone, rzfromDate).exec(ctx);
                        ctx.commit_transaction();
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

                    ::LibFred::OperationContextCreator ctx;
                    ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, rzzone, rzfromDate).exec(ctx);
                    ctx.commit_transaction();
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

                    ::LibFred::OperationContextCreator ctx;
                    ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, rzzone, rzfromDate).exec(ctx);
                    ctx.commit_transaction();
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

                        ::LibFred::OperationContextCreator ctx;
                        ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, rzzone, rzfromDate).exec(ctx);
                        ctx.commit_transaction();
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


    //count VAT from price without tax using coefficient - local CZ rules
    std::string count_dph( //returning vat rounded half-up to 2 decimal places
        std::string price //Kc incl. vat
        , std::string vat_reverse //vat coeff like 0.1597 for vat 19%
      )
    {

        Database::Result vat_res = connp->exec_params(
                "select round($1::numeric * $2::numeric, 2) "
                ,Database::query_param_list(price)(vat_reverse));

        std::string vat;

        if(vat_res.size() == 1 && vat_res[0].size() == 1)
            vat =std::string(vat_res[0][0]);
        else
            throw std::runtime_error("count_dph error");

        LOGGER.debug (
            boost::format("count_dph price %1% vat_reverse %2% vat %3%")
            % price % vat_reverse % vat);

       return vat;
    }

    std::string test_vat(std::string price, boost::gregorian::date taxdate, unsigned long long registrar_id)
    {
        Database::Result rvat = connp->exec_params("SELECT vat FROM registrar WHERE id=$1::integer",
                Database::query_param_list(registrar_id));

        if(rvat.size() != 1 || rvat[0][0].isnull() ) {
            throw std::runtime_error(
                (boost::format(" Couldn't determine whether the registrar with ID %1% pays VAT.")
                    % registrar_id).str());
        }
        bool pay_vat = rvat[0][0];

        //std::cout << "pay_vat: " << pay_vat << std::endl;

        // VAT  percentage
        std::string vat_percent("0");
        // ratio for reverse VAT calculation
        // price_without_vat = price_with_vat - price_with_vat*vat_reverse
        // it should have 4 decimal places in the DB
        std::string vat_reverse("0");

        Database::Result vat_details = connp->exec_params(
        "select vat, koef::numeric from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
        , Database::query_param_list(taxdate.is_special() ? boost::gregorian::day_clock::universal_day() : taxdate )
        );

        if(vat_details.size() > 1) {
            throw std::runtime_error("Multiple valid VAT values found.");
        } else if(vat_details.size() == 0) {
            throw std::runtime_error("No valid VAT value found.");
        }

        if(vat_details[0][0].isnull() == false)
            vat_percent=std::string(vat_details[0][0]);

        //std::cout << "vat_percent: " << vat_percent << std::endl;

        if(vat_details[0][1].isnull() == false)
            vat_reverse=std::string(vat_details[0][1]);

        //std::cout << "vat_reverse: " << vat_reverse << std::endl;

        return pay_vat ? count_dph(price,vat_reverse) : std::string("0");
    }


    //count VAT from price with tax using coefficient - local CZ rules
    Money count_dph( //returning vat rounded half-up to 2 decimal places
        Money price //Kc incl. vat
        , Decimal vat_reverse //vat coeff like 0.1597 for vat 19%
      )
    {
        Money vat =   price * vat_reverse;
        vat.round_half_up(2);

       LOGGER.debug (
           boost::format("count_dph price %1% vat_reverse %2% vat %3%")
           % price % vat_reverse % vat);

       return vat;
    }

    Money test_vat(Money price, boost::gregorian::date taxdate, unsigned long long registrar_id)
    {
        Database::Result rvat = connp->exec_params("SELECT vat FROM registrar WHERE id=$1::integer",
                Database::query_param_list(registrar_id));

        if(rvat.size() != 1 || rvat[0][0].isnull() ) {
            throw std::runtime_error(
                (boost::format(" Couldn't determine whether the registrar with ID %1% pays VAT.")
                    % registrar_id).str());
        }
        bool pay_vat = rvat[0][0];

        //std::cout << "pay_vat: " << pay_vat << std::endl;

        // VAT  percentage
        Decimal vat_percent("0");
        // ratio for reverse VAT calculation
        // price_without_vat = price_with_vat - price_with_vat*vat_reverse
        // it should have 4 decimal places in the DB
        Decimal vat_reverse("0");

        Database::Result vat_details = connp->exec_params(
        "select vat, koef::numeric from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
        , Database::query_param_list(taxdate.is_special() ? boost::gregorian::day_clock::universal_day() : taxdate )
        );

        if(vat_details.size() > 1) {
            throw std::runtime_error("Multiple valid VAT values found.");
        } else if(vat_details.size() == 0) {
            throw std::runtime_error("No valid VAT value found.");
        }

        if(vat_details[0][0].isnull() == false)
            vat_percent.set_string(vat_details[0][0]);

        //std::cout << "vat_percent: " << vat_percent << std::endl;

        if(vat_details[0][1].isnull() == false)
            vat_reverse.set_string(vat_details[0][1]);

        //std::cout << "vat_reverse: " << vat_reverse << std::endl;

        return pay_vat ? count_dph(price,vat_reverse) : Money("0");
    }

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
                        Money credit0_dec;
                        std::string credit0_str;
                        if(credit0_res.size() ==  1 && credit0_res[0].size() == 1)
                        {
                            credit0_dec.set_string(std::string(credit0_res[0][0]));
                            credit0_str = std::string(credit0_res[0][0]);
                        }
                        else
                        {
                            credit0_dec.set_string("0");
                            credit0_str = std::string("0");
                        }

                        Money credit_vat_dec;
                        std::string credit_vat_str;

                        //add credit
                        int year = start_year;
                        {
                            unsigned long long invoiceid = 0;
                            Database::Date taxdate;
                            taxdate = Database::Date(year,1,1);
                            Money price ("10");
                            Money out_credit;
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price//price
                                    , boost::posix_time::ptime(taxdate), out_credit);//invoice_date
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);

                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                            credit_vat_dec = test_vat(Money("10"),taxdate, registrar_id);
                            credit_vat_str = test_vat(std::string("10"),taxdate, registrar_id);
                        }

                        //credit after
                        Database::Result credit1_res = connp->exec_params(zone_registrar_credit_query
                                , Database::query_param_list(zone_id)(registrar_id));
                        Money credit1_dec;
                        std::string credit1_str;

                        if(credit1_res.size() ==  1 && credit1_res[0].size() == 1)
                        {
                            credit1_dec.set_string( std::string(credit1_res[0][0]));
                            credit1_str = std::string(credit1_res[0][0]);
                        }
                        else
                        {
                            credit1_dec.set_string("0");
                            credit1_str = std::string("0");
                        }

                        if(!(credit1_dec - credit0_dec == Money("10") - credit_vat_dec))
                        {
                            std::cout << "\n!(credit1_dec - credit0_dec == Money(\"10\") - credit_vat_dec) \n"
                                    << "credit1_dec: " << credit1_dec << " credit0_dec: " << credit0_dec
                                    << " credit_vat_dec: " << credit_vat_dec
                                    << " credit_vat_str: " << credit_vat_str
                                    << std::endl;
                        }
                        BOOST_CHECK(credit1_dec - credit0_dec == Money("10") - credit_vat_dec);


                        Database::Result credit_difference_result
                            = connp->exec_params("select $1::numeric - $2::numeric, '10.00'::numeric - $3::numeric "
                            , Database::query_param_list(credit1_str)(credit0_str)(credit_vat_str));
                        std::string credit_diff1_str;
                        std::string credit_diff2_str;
                        if(credit_difference_result.size() == 1 && credit_difference_result[0].size() == 2)
                        {
                            credit_diff1_str = std::string(credit_difference_result[0][0]);
                            credit_diff2_str = std::string(credit_difference_result[0][1]);
                        }
                        else
                        {
                            credit_diff1_str = std::string("0");
                            credit_diff2_str = std::string("0");
                        }

                        BOOST_CHECK(credit_diff1_str.compare(credit_diff2_str) == 0 );
                        if(credit_diff1_str.compare(credit_diff2_str) != 0)
                        std::cout << "credit_diff1_str: " << credit_diff1_str
                                << " credit_diff2_str: " << credit_diff2_str << std::endl;

                    }//for zone_i
                    continue;//don't add other credit
                }//if LOWCREDIT1

                if(std::string(registrar_result[registrar_i][1]).find("LOWCREDIT2") != std::string::npos)
                {
                    for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                    {
                        unsigned long long zone_id ( zone_result[zone_i][0]);

                        //credit before
                        Database::Result credit0_res = connp->exec_params(zone_registrar_credit_query
                                , Database::query_param_list(zone_id)(registrar_id));
                        Money credit0_dec;
                        std::string credit0_str;
                        if(credit0_res.size() ==  1 && credit0_res[0].size() == 1)
                        {
                            credit0_dec.set_string(std::string(credit0_res[0][0]));
                            credit0_str = std::string(credit0_res[0][0]);
                        }
                        else
                        {
                            credit0_dec.set_string("0");
                            credit0_str = std::string("0");
                        }

                        Money credit1_vat_dec;
                        std::string credit1_vat_str;

                        Money credit2_vat_dec;
                        std::string credit2_vat_str;

                        int year = start_year;
                        {
                            unsigned long long invoiceid = 0;
                            Database::Date taxdate;
                            taxdate = Database::Date(year,1,1);
                            Money price ("10");
                            Money out_credit;
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price//price
                                    , boost::posix_time::ptime(taxdate), out_credit);//invoice_date
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);

                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                            credit1_vat_dec = test_vat(Money("10"),taxdate, registrar_id);
                            credit1_vat_str = test_vat(std::string("10"),taxdate, registrar_id);

                            //credit
                            Database::Result credit1_res = connp->exec_params(zone_registrar_credit_query
                                    , Database::query_param_list(zone_id)(registrar_id));
                            Money credit1_dec;
                            std::string credit1_str;
                            if(credit1_res.size() ==  1 && credit1_res[0].size() == 1)
                            {
                                credit1_dec.set_string(std::string(credit1_res[0][0]));
                                credit1_str = std::string(credit1_res[0][0]);
                            }
                            else
                            {
                                credit1_dec.set_string("0");
                                credit1_str = std::string("0");
                            }

                            BOOST_CHECK(credit1_dec - credit0_dec == Money("10") - credit1_vat_dec);

                            Database::Result credit1_difference_result
                                = connp->exec_params("select $1::numeric - $2::numeric, '10.00'::numeric - $3::numeric "
                                , Database::query_param_list(credit1_str)(credit0_str)(credit1_vat_str));
                            std::string credit_diff1_str;
                            std::string credit_diff2_str;
                            if(credit1_difference_result.size() == 1 && credit1_difference_result[0].size() == 2)
                            {
                                credit_diff1_str = std::string(credit1_difference_result[0][0]);
                                credit_diff2_str = std::string(credit1_difference_result[0][1]);
                            }
                            else
                            {
                                credit_diff1_str = std::string("0");
                                credit_diff2_str = std::string("0");
                            }

                            BOOST_CHECK(credit_diff1_str.compare(credit_diff2_str) == 0 );
                            if(credit_diff1_str.compare(credit_diff2_str) != 0)
                            std::cout << "credit_diff1_str: " << credit_diff1_str
                                    << " credit_diff2_str: " << credit_diff2_str << std::endl;


                            taxdate = Database::Date(year,12,31);
                            price = Money("10");//Kc
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price//price
                                    , boost::posix_time::ptime(taxdate)//invoice_date
                                    , out_credit);
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);

                            if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                            credit2_vat_dec = test_vat(Money("10"),taxdate, registrar_id);
                            credit2_vat_str = test_vat(std::string("10"),taxdate, registrar_id);


                            //credit after
                            Database::Result credit2_res = connp->exec_params(zone_registrar_credit_query
                                    , Database::query_param_list(zone_id)(registrar_id));
                            Money credit2_dec;
                            std::string credit2_str;

                            if(credit2_res.size() ==  1 && credit2_res[0].size() == 1)
                            {
                                credit2_dec.set_string( std::string(credit2_res[0][0]));
                                credit2_str = std::string(credit2_res[0][0]);
                            }
                            else
                            {
                                credit2_dec.set_string("0");
                                credit2_str = std::string("0");
                            }

                            BOOST_CHECK(credit2_dec - credit1_dec == Money("10") - credit2_vat_dec);

                            Database::Result credit2_difference_result
                                = connp->exec_params("select $1::numeric - $2::numeric, '10.00'::numeric - $3::numeric "
                                , Database::query_param_list(credit2_str)(credit1_str)(credit2_vat_str));
                            std::string credit_diff3_str;
                            std::string credit_diff4_str;
                            if(credit2_difference_result.size() == 1 && credit2_difference_result[0].size() == 2)
                            {
                                credit_diff3_str = std::string(credit2_difference_result[0][0]);
                                credit_diff4_str = std::string(credit2_difference_result[0][1]);
                            }
                            else
                            {
                                credit_diff3_str = std::string("0");
                                credit_diff4_str = std::string("0");
                            }

                            BOOST_CHECK(credit_diff3_str.compare(credit_diff4_str) == 0 );
                            if(credit_diff3_str.compare(credit_diff4_str) != 0)
                            std::cout << "credit_diff3_str: " << credit_diff3_str
                                    << " credit_diff4_str: " << credit_diff4_str << std::endl;

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
                            Money price ("30000000.00");//Kc
                            Money out_credit;
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price//price
                                    , boost::posix_time::ptime(taxdate), out_credit);//invoice_date
                            ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);
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
                            Money price ("-10000000.00");//Kc
                            Money out_credit;
                            invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                    , zone_id//zone
                                    , registrar_id//registrar
                                    , price//price
                                    , boost::posix_time::ptime(taxdate), out_credit);//invoice_date
                            BOOST_CHECK_EQUAL(invoiceid != 0,true);
                            ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);
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
                        Money price ("50000.00");//Kc
                        Money out_credit;
                        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                , zone_id//zone
                                , registrar_id//registrar
                                , price//price
                                , boost::posix_time::ptime(taxdate), out_credit);//invoice_date
                        BOOST_CHECK_EQUAL(invoiceid != 0,true);
                        ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);

                        if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

                        taxdate = Database::Date(year,6,10);
                        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                , zone_id//zone
                                , registrar_id//registrar
                                , price//price
                                , boost::posix_time::ptime(taxdate), out_credit);//invoice_date
                        BOOST_CHECK_EQUAL(invoiceid != 0,true);
                        ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);

                        if (invoiceid != 0) deposit_invoice_id_vect.push_back(invoiceid);

                        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

                        taxdate = Database::Date(year,12,31);
                        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                                , zone_id//zone
                                , registrar_id//registrar
                                , price//price
                                , boost::posix_time::ptime(taxdate), out_credit);//invoice_date
                        BOOST_CHECK_EQUAL(invoiceid != 0,true);
                        ::LibFred::Credit::add_credit_to_invoice( registrar_id,  zone_id, out_credit, invoiceid);

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

struct create_geneppoperation_fixture
: virtual zone_fixture
, virtual registrar_fixture
, virtual invoice_manager_fixture
{
  create_geneppoperation_fixture()
  {
      try
      {
          boost::gregorian::date tmp_today
                  = boost::posix_time::second_clock::local_time().date();//today

          boost::gregorian::date first_of_this_month = boost::gregorian::date(tmp_today.year()
                  , tmp_today.month(), 1);//first day of current month

          boost::gregorian::date last_day_of_last_month = first_of_this_month
                  - boost::gregorian::days(1);//last day of last month

          boost::gregorian::date first_day_of_last_month = boost::gregorian::date(last_day_of_last_month.year()
                  , last_day_of_last_month.month(), 1);//first day of last month


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

                    //if(std::string(zone_result[zone_i][1]).find("cz") != std::string::npos)
                    {
                      const auto crdate = boost::posix_time::second_clock::universal_time();
                      const auto price_timestamp = crdate;
                      invMan->charge_operation_auto_price(
                                "GeneralEppOperation"
                                , zone_id
                                , registrar_id
                                , 0 //object_id
                                , crdate
                                , first_day_of_last_month//date_from //local date
                                , last_day_of_last_month// date_to //local date
                                , price_timestamp
                                , Decimal ("900000"));
                    }
                  }//for zone_i
                  continue;//don't add other credit
              }//if LOWCREDIT1

              if(std::string(registrar_result[registrar_i][1]).find("LOWCREDIT2") != std::string::npos)
              {
                  for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                  {
                      unsigned long long zone_id ( zone_result[zone_i][0]);
                      //if(std::string(zone_result[zone_i][1]).find("cz") != std::string::npos)
                      {
                        const auto crdate = boost::posix_time::second_clock::universal_time();
                        const auto price_timestamp = crdate;
                        invMan->charge_operation_auto_price(
                                  "GeneralEppOperation"
                                  , zone_id
                                  , registrar_id
                                  , 0 //object_id
                                  , crdate
                                , first_day_of_last_month//date_from //local date
                                , last_day_of_last_month// date_to //local date
                                  , price_timestamp
                                  , Decimal ("800000"));
                      }


                  }//for zone_i
                  continue;//don't add other credit
              }//if LOWCREDIT2

              //big credit
              if(std::string(registrar_result[registrar_i][1]).find("BIGCREDIT") != std::string::npos)
              {
                  for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                  {
                      unsigned long long zone_id ( zone_result[zone_i][0]);

                      //if(std::string(zone_result[zone_i][1]).find("cz") != std::string::npos)
                      {
                        const auto crdate = boost::posix_time::second_clock::universal_time();
                        const auto price_timestamp = crdate;
                        invMan->charge_operation_auto_price(
                                  "GeneralEppOperation"
                                  , zone_id
                                  , registrar_id
                                  , 0 //object_id
                                  , crdate
                                , first_day_of_last_month//date_from //local date
                                , last_day_of_last_month// date_to //local date
                                  , price_timestamp
                                  , Decimal ("700000"));
                      }
                  }//for zone_i
                  continue;//don't add other credit
              }//if BIGCREDIT

              //negative credit
              if(std::string(registrar_result[registrar_i][1]).find("NEGCREDIT") != std::string::npos)
              {
                  for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
                  {
                      unsigned long long zone_id ( zone_result[zone_i][0]);

                      //if(std::string(zone_result[zone_i][1]).find("cz") != std::string::npos)
                      {
                        const auto crdate = boost::posix_time::second_clock::universal_time();
                        const auto price_timestamp = crdate;
                        invMan->charge_operation_auto_price(
                                  "GeneralEppOperation"
                                  , zone_id
                                  , registrar_id
                                  , 0 //object_id
                                  , crdate
                                , first_day_of_last_month//date_from //local date
                                , last_day_of_last_month// date_to //local date
                                  , price_timestamp
                                  , Decimal ("600000"));
                      }

                  }//for zone_i
                  continue;//don't add other credit
              }//if NEGCREDIT

              //add a lot of credit
              for(std::size_t zone_i = 0; zone_i < zone_result.size(); ++zone_i)
              {
                  unsigned long long zone_id ( zone_result[zone_i][0]);

                  //if(std::string(zone_result[zone_i][1]).find("cz") != std::string::npos)
                  {
                    const auto crdate = boost::posix_time::second_clock::universal_time();
                    const auto price_timestamp = crdate;
                    invMan->charge_operation_auto_price(
                              "GeneralEppOperation"
                              , zone_id
                              , registrar_id
                              , 0 //object_id
                              , crdate
                            , first_day_of_last_month//date_from //local date
                            , last_day_of_last_month// date_to //local date
                              , price_timestamp
                              , Decimal ("500000"));
                  }

              }//for zone_i
          }//for registrar
      }
      catch(...)
      {
          fixture_exception_handler("create_geneppoperation_fixture ctor exception", true)();
      }
}
protected:
  ~create_geneppoperation_fixture()
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
    , virtual create_geneppoperation_fixture
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
