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
#include <utility>
#include <vector>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include "test/deprecated/test_common_registry.hh"
#include "src/util/types/money.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"

#include "test/deprecated/test_invoice_common.hh"

#include <boost/test/unit_test.hpp>

#include "src/util/time_clock.hh"
#include "src/deprecated/libfred/credit.hh"
#include "src/bin/corba/file_manager_client.hh"

#include "src/util/corba_wrapper_decl.hh"

#include "src/deprecated/libfred/banking/bank_common.hh"


using namespace ::LibFred::Invoicing;

BOOST_AUTO_TEST_SUITE(TestInvoiceAccount)


// test createAccountInvoices with default values supplied by fred-admin
BOOST_AUTO_TEST_CASE( createAccountInvoices_defaultValues )
{

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create());

    boost::gregorian::date local = day_clock::local_day();		
    boost::gregorian::date todate(local.year(), local.month(), 1);
    boost::gregorian::date fromdate(todate - months(1));
    boost::gregorian::date taxdate(todate);

    invMan->createAccountInvoices( std::string("cz")
        , taxdate
        , fromdate
        // , boost::gregorian::date()//from_date not set
        , todate, boost::posix_time::ptime(todate));
}

BOOST_AUTO_TEST_CASE( createAccountInvoice_request1 )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    conn.exec("update price_list set price = 140 from zone where price_list.zone_id = zone.id and zone.fqdn='cz' and price_list.operation_id = 2");
    init_corba_container();
    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::string registrar_handle =     registrar->getHandle();
    unsigned long long registrar_inv_id = registrar->getId();
    std::string time_string(TimeStamp::microsec());
    try_insert_invoice_prefix();
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create());
    //add credit
    unsigned long long invoiceid = 0;
    Database::Date taxdate;
    taxdate = Database::Date(2010,1,1);
    Money price = std::string("50000.00");//money
    Money out_credit;
    invoiceid = invMan->createDepositInvoice(taxdate//taxdate
            , zone_cz_id//zone
            , registrar_inv_id//registrar
            , price
            , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

    //try get epp reference
    ccReg::EPP_var epp_ref;
    epp_ref = ccReg::EPP::_narrow(CorbaContainer::get_instance()->nsresolve("EPP"));

    //login
    CORBA::ULongLong clientId = 0;
    ccReg::Response_var r;

    std::string test_domain_fqdn(std::string("testdomain") + time_string);

    try
    {
        CORBA::String_var registrar_handle_var = CORBA::string_dup(registrar_handle.c_str());
        CORBA::String_var passwd_var = CORBA::string_dup("");
        CORBA::String_var new_passwd_var = CORBA::string_dup("");
        CORBA::String_var cltrid_var = CORBA::string_dup("omg");
        CORBA::String_var xml_var = CORBA::string_dup("<omg/>");
        CORBA::String_var cert_var = CORBA::string_dup("");

        r = epp_ref->ClientLogin(
            registrar_handle_var,passwd_var,new_passwd_var,cltrid_var,
            xml_var,clientId,0,cert_var,ccReg::EN);

        if (r->code != 1000 || !clientId) {
            std::cerr << "Cannot connect: " << r->code << std::endl;
            throw std::runtime_error("Cannot connect ");
        }



        for (int i =0 ; i < 10; i+=2)
        {
            ccReg::Period_str period;
            period.count = 1;
            period.unit = ccReg::unit_year;
            ccReg::EppParams epp_params;
            epp_params.requestID = clientId + i;
            epp_params.loginID = clientId;
            epp_params.clTRID = "";
            epp_params.XML = "";
            CORBA::String_var crdate;
            CORBA::String_var exdate;

            r = epp_ref->DomainCreate(
                    (test_domain_fqdn + boost::lexical_cast<std::string>(i) + ".cz").c_str(), // fqdn
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

            ++i;

            ccReg::EppParams epp_params_renew;
            epp_params_renew.requestID = clientId+i;
            epp_params_renew.loginID = clientId;
            epp_params_renew.clTRID = "";
            epp_params_renew.XML = "";

            period.unit = ccReg::unit_year;
            period.count = 3;
            CORBA::String_var exdate1;
            r = epp_ref->DomainRenew(
                    (test_domain_fqdn + boost::lexical_cast<std::string>(i-1)+".cz").c_str(), // fqdn
                    exdate,//curExpDate
                    period, //Period_str
                    exdate1,//out timestamp exDate,
                    epp_params_renew,//in EppParams params,
                    ccReg::ExtensionList()//in ExtensionList ext
                    );

        }

    }//try
    catch(ccReg::EPP::EppError &_epp_error)
    {
        handle_epp_exception(_epp_error);
    }
    catch(CORBA::TRANSIENT&)
    {
        Logging::Manager::instance_ref().error("Caught exception CORBA::TRANSIENT -- unable to contact the server." );
        std::cerr << "Caught exception CORBA::TRANSIENT -- unable to contact the "
             << "server." << std::endl;
        throw;
    }
    catch(CORBA::SystemException& ex)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::SystemException: ")+ex._name() );
        std::cerr << "Caught CORBA::SystemException" << ex._name() << std::endl;
        throw;
    }
    catch(CORBA::Exception& ex)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::Exception: ")+ex._name() );
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
        throw;
    }
    catch(omniORB::fatalException& fe)
    {
        std::string errmsg = std::string("Caught omniORB::fatalException: ")
                        + std::string("  file: ") + std::string(fe.file())
                        + std::string("  line: ") + boost::lexical_cast<std::string>(fe.line())
                        + std::string("  mesg: ") + std::string(fe.errmsg());
        Logging::Manager::instance_ref().error(errmsg);
        std::cerr << errmsg  << std::endl;
        throw;
    }

    {
        boost::gregorian::date todate( day_clock::local_day() );
        boost::gregorian::date fromdate( todate - months(1) );
        boost::gregorian::date taxdate(todate);


        invMan->charge_operation_auto_price(
                         "GeneralEppOperation"
                         , zone_cz_id
                         , registrar_inv_id
                         , 0 //object_id
                         , boost::posix_time::second_clock::universal_time() //crdate //utc timestamp
                         , todate - boost::gregorian::months(1)//date_from //local date
                         , todate// date_to //local date
                         , Decimal ("100000"));


        invMan->createAccountInvoice( registrar_handle, std::string("cz")
            , taxdate
            , fromdate //from_date not set
            , todate, boost::posix_time::ptime(todate));

    }

    {

        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
                   ->get_handler_ptr_by_type<HandleRegistryArgs>();

        std::string corbaNS =ns_args_ptr->nameservice_host
                + ":"
                + boost::lexical_cast<std::string>(ns_args_ptr->nameservice_port);




        std::unique_ptr<::LibFred::Document::Manager> docMan(
                  ::LibFred::Document::Manager::create(
                      registry_args_ptr->docgen_path
                      , registry_args_ptr->docgen_template_path
                      , registry_args_ptr->fileclient_path
                      , corbaNS)
                  );

        //manager init
        MailerManager mailMan(CorbaContainer::get_instance()->getNS());
        std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
            ::LibFred::Invoicing::Manager::create(
            docMan.get(),&mailMan));

        FileManagerClient fm_client(
                 CorbaContainer::get_instance()->getNS());

        //call archive invoices and get processed invoice ids
        InvoiceIdVect inv_id_vect = invMan->archiveInvoices(true);
    }


}

BOOST_AUTO_TEST_CASE(archiveAccountInvoice)
{
    init_corba_container();
    Database::Connection conn = Database::Manager::acquire();
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
               ->get_handler_ptr_by_type<HandleRegistryArgs>();
    std::string corbaNS =ns_args_ptr->nameservice_host
            + ":"
            + boost::lexical_cast<std::string>(ns_args_ptr->nameservice_port);
    std::unique_ptr<::LibFred::Document::Manager> docMan(
              ::LibFred::Document::Manager::create(
                  registry_args_ptr->docgen_path
                  , registry_args_ptr->docgen_template_path
                  , registry_args_ptr->fileclient_path
                  , corbaNS)
              );
    //manager init
    MailerManager mailMan(CorbaContainer::get_instance()->getNS());
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create(
        docMan.get(),&mailMan));
    FileManagerClient fm_client(
             CorbaContainer::get_instance()->getNS());
    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::string registrar_handle =     registrar->getHandle();
    unsigned long long registrar_inv_id = registrar->getId();

    BOOST_TEST_MESSAGE(registrar_handle);


    for(unsigned long long iter_account = 0; iter_account < 2 ; ++iter_account)
    {
        //add credit
        unsigned long long invoiceid = 0;
        Database::Date taxdate;
        taxdate = Database::Date(2010,1,1);
        Money price = std::string("1000.00");//money
        Money out_credit;
        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                , zone_cz_id//zone
                , registrar_inv_id//registrar
                , price
                , boost::posix_time::ptime(taxdate), out_credit);//price
        BOOST_CHECK_EQUAL(invoiceid != 0,true);
        ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

        try
        {//create domain
            namespace ip = boost::asio::ip;

            for(unsigned long long i = 0; i < 3 ; ++i)
            {
                std::string time_string(TimeStamp::microsec());
                time_string += "i";
                time_string += boost::lexical_cast<std::string>(i);
                std::string xmark(time_string);
                std::string admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark);
                std::string registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark);

                ::LibFred::OperationContextCreator ctx;

                ::LibFred::Contact::PlaceAddress place;
                place.street1 = std::string("STR1") + xmark;
                place.city = "Praha";
                place.postalcode = "11150";
                place.country = "CZ";
                ::LibFred::CreateContact(admin_contact2_handle,registrar_handle)
                    .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
                    .set_disclosename(true)
                    .set_place(place)
                    .set_discloseaddress(true)
                    .exec(ctx);

                ::LibFred::CreateContact(registrant_contact_handle,registrar_handle)
                    .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
                    .set_disclosename(true)
                    .set_place(place)
                    .set_discloseaddress(true)
                    .exec(ctx);

                std::string test_nsset_handle("invarchtestnsset");
                test_nsset_handle += time_string;
                ::LibFred::CreateNsset(test_nsset_handle, registrar_handle)
                   .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                       (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                       (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                       )
                       .set_tech_contacts(Util::vector_of<std::string>(admin_contact2_handle))
                       .exec(ctx);

                std::string test_domain_fqdn("invarchtest");
                test_domain_fqdn += time_string;
                test_domain_fqdn += ".cz";

                BOOST_TEST_MESSAGE(test_domain_fqdn);

                ::LibFred::CreateDomain(test_domain_fqdn, registrar_handle, registrant_contact_handle)
                .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
                .set_nsset(test_nsset_handle)
                .set_expiration_date(day_clock::universal_day() + boost::gregorian::years(1))
                .exec(ctx);

                unsigned long long domain_id = static_cast<unsigned long long>(
                        ctx.get_conn().exec_params("select id from object_registry where name = $1::text"
                        , Database::query_param_list(test_domain_fqdn))[0][0]);

                ctx.commit_transaction();

                invMan->charge_operation_auto_price(
                                 "CreateDomain"
                                 , zone_cz_id
                                 , registrar_inv_id
                                 , domain_id //object_id
                                 , boost::posix_time::second_clock::universal_time() //crdate //utc timestamp
                                 , day_clock::local_day() - boost::gregorian::months(1)//date_from //local date
                                 , boost::gregorian::date()// date_to //local date
                                 , Decimal ("1"));//1 year

                invMan->charge_operation_auto_price(
                                 "RenewDomain"
                                 , zone_cz_id
                                 , registrar_inv_id
                                 , domain_id //object_id
                                 , boost::posix_time::second_clock::universal_time() //crdate //utc timestamp
                                 , day_clock::local_day() - boost::gregorian::months(1)//date_from //local date
                                 , day_clock::local_day() + boost::gregorian::months(11)// date_to //local date
                                 , Decimal ("1"));//1 year



            }
            invMan->charge_operation_auto_price(
                      "GeneralEppOperation"
                      , zone_cz_id
                      , registrar_inv_id
                      , 0 //object_id
                      , boost::posix_time::second_clock::universal_time() //crdate //utc timestamp
                      , day_clock::local_day() - boost::gregorian::months(12) - boost::gregorian::months(1)//date_from //local date
                      , day_clock::local_day()// date_to //local date
                      , Decimal ("90"));
        }
        catch(boost::exception& ex)
        {
            BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
            throw;
        }
        catch(std::exception& ex)
        {
            BOOST_TEST_MESSAGE(ex.what());
            throw;
        }

        {//account invoice
            boost::gregorian::date todate( day_clock::local_day() );
            boost::gregorian::date fromdate( todate - months(1) );
            boost::gregorian::date taxdate(todate);


            invMan->createAccountInvoice( registrar_handle, std::string("cz")
                , taxdate
                , fromdate //from_date not set
                , todate, boost::posix_time::ptime(todate));
        }
        //call archive invoices and get processed invoice ids
        InvoiceIdVect inv_id_vect = invMan->archiveInvoices(true);
    }
}

BOOST_AUTO_TEST_CASE( createAccountInvoice_request2 )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    conn.exec("update price_list set price = 140 from zone where price_list.zone_id = zone.id and zone.fqdn='cz' and price_list.operation_id = 2");
    init_corba_container();
    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::string registrar_handle =     registrar->getHandle();
    unsigned long long registrar_inv_id = registrar->getId();
    std::string time_string(TimeStamp::microsec());
    try_insert_invoice_prefix();
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create());
    //add credit
    unsigned long long invoiceid = 0;
    Database::Date taxdate;
    taxdate = Database::Date(2010,1,1);
    Money price = std::string("50000.00");//money
    Money out_credit;
    invoiceid = invMan->createDepositInvoice(taxdate//taxdate
            , zone_cz_id//zone
            , registrar_inv_id//registrar
            , price
            , boost::posix_time::ptime(taxdate), out_credit);//price
    BOOST_CHECK_EQUAL(invoiceid != 0,true);
    ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

    //try get epp reference
    ccReg::EPP_var epp_ref;
    epp_ref = ccReg::EPP::_narrow(CorbaContainer::get_instance()->nsresolve("EPP"));

    //login
    CORBA::ULongLong clientId = 0;
    ccReg::Response_var r;

    std::string test_domain_fqdn(std::string("testdomain2") + time_string);

    try
    {
        CORBA::String_var registrar_handle_var = CORBA::string_dup(registrar_handle.c_str());
        CORBA::String_var passwd_var = CORBA::string_dup("");
        CORBA::String_var new_passwd_var = CORBA::string_dup("");
        CORBA::String_var cltrid_var = CORBA::string_dup("omg");
        CORBA::String_var xml_var = CORBA::string_dup("<omg/>");
        CORBA::String_var cert_var = CORBA::string_dup("");

        r = epp_ref->ClientLogin(
            registrar_handle_var,passwd_var,new_passwd_var,cltrid_var,
            xml_var,clientId,0,cert_var,ccReg::EN);

        if (r->code != 1000 || !clientId) {
            std::cerr << "Cannot connect: " << r->code << std::endl;
            throw std::runtime_error("Cannot connect ");
        }



        for (int i =0 ; i < 10; i+=2)
        {
            ccReg::Period_str period;
            period.count = 1;
            period.unit = ccReg::unit_year;
            ccReg::EppParams epp_params;
            epp_params.requestID = clientId + i;
            epp_params.loginID = clientId;
            epp_params.clTRID = "";
            epp_params.XML = "";
            CORBA::String_var crdate;
            CORBA::String_var exdate;

            r = epp_ref->DomainCreate(
                    (test_domain_fqdn+boost::lexical_cast<std::string>(i)+".cz").c_str(), // fqdn
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

            ++i;

            ccReg::EppParams epp_params_renew;
            epp_params_renew.requestID = clientId+i;
            epp_params_renew.loginID = clientId;
            epp_params_renew.clTRID = "";
            epp_params_renew.XML = "";

            period.unit = ccReg::unit_year;
            period.count = 3;
            CORBA::String_var exdate1;
            r = epp_ref->DomainRenew(
                    (test_domain_fqdn+boost::lexical_cast<std::string>(i-1)+".cz").c_str(), // fqdn
                    exdate,//curExpDate
                    period, //Period_str
                    exdate1,//out timestamp exDate,
                    epp_params_renew,//in EppParams params,
                    ccReg::ExtensionList()//in ExtensionList ext
                    );

        }

    }//try
    catch(ccReg::EPP::EppError &_epp_error)
    {
        handle_epp_exception(_epp_error);
    }
    catch(CORBA::TRANSIENT&)
    {
        Logging::Manager::instance_ref().error("Caught exception CORBA::TRANSIENT -- unable to contact the server." );
        std::cerr << "Caught exception CORBA::TRANSIENT -- unable to contact the "
             << "server." << std::endl;
        throw;
    }
    catch(CORBA::SystemException& ex)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::SystemException: ")+ex._name() );
        std::cerr << "Caught CORBA::SystemException" << ex._name() << std::endl;
        throw;
    }
    catch(CORBA::Exception& ex)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::Exception: ")+ex._name() );
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
        throw;
    }
    catch(omniORB::fatalException& fe)
    {
        std::string errmsg = std::string("Caught omniORB::fatalException: ")
                        + std::string("  file: ") + std::string(fe.file())
                        + std::string("  line: ") + boost::lexical_cast<std::string>(fe.line())
                        + std::string("  mesg: ") + std::string(fe.errmsg());
        Logging::Manager::instance_ref().error(errmsg);
        std::cerr << errmsg  << std::endl;
        throw;
    }

    {
        boost::gregorian::date todate( day_clock::local_day() );
        boost::gregorian::date fromdate( todate - months(1) );
        boost::gregorian::date taxdate(todate);

        invMan->charge_operation_auto_price(
                         "GeneralEppOperation"
                         , zone_cz_id
                         , registrar_inv_id
                         , 0 //object_id
                         , boost::posix_time::second_clock::universal_time() //crdate //utc timestamp
                         , todate - boost::gregorian::months(1)//date_from //local date
                         , todate// date_to //local date
                         , Decimal ("490000"));


        invMan->createAccountInvoice( registrar_handle, std::string("cz")
            , taxdate
            , fromdate //from_date not set
            , todate, boost::posix_time::ptime(todate));

    }

    {

        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
                   ->get_handler_ptr_by_type<HandleRegistryArgs>();

        std::string corbaNS =ns_args_ptr->nameservice_host
                + ":"
                + boost::lexical_cast<std::string>(ns_args_ptr->nameservice_port);




        std::unique_ptr<::LibFred::Document::Manager> docMan(
                  ::LibFred::Document::Manager::create(
                      registry_args_ptr->docgen_path
                      , registry_args_ptr->docgen_template_path
                      , registry_args_ptr->fileclient_path
                      , corbaNS)
                  );

        //manager init
        MailerManager mailMan(CorbaContainer::get_instance()->getNS());
        std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
            ::LibFred::Invoicing::Manager::create(
            docMan.get(),&mailMan));

        FileManagerClient fm_client(
                 CorbaContainer::get_instance()->getNS());

        //call archive invoices and get processed invoice ids
        InvoiceIdVect inv_id_vect = invMan->archiveInvoices(true);
    }


}




BOOST_AUTO_TEST_CASE( createAccountInvoices_registrar )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    //set operation price
    conn.exec_params("update price_list set price = price + 0.11 where zone_id = $1::bigint and operation_id = 2", 
	Database::query_param_list(zone_cz_id));

    Database::Result renew_operation_price_result = conn.exec_params(
        "SELECT price, quantity FROM price_list WHERE valid_from < 'now()'"
        " and ( valid_to is NULL or valid_to > 'now()' ) "
        "and operation_id = 2 and zone_id = $1::bigint "
        "order by valid_from desc limit 1", Database::query_param_list(zone_cz_id));

    Money renew_operation_price = std::string(renew_operation_price_result[0][0]);


    //std::cout<< "create_operation_price: " << create_operation_price 
    //    << "renew_operation_price: " << renew_operation_price << std::endl;

    init_corba_container();

    // handle which doesn't match  any registrar
    std::string time_string(TimeStamp::microsec());
    std::string noregistrar_handle(std::string("REG-NOTEXIST")+time_string);

    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    std::string registrar_handle =     registrar->getHandle();
    unsigned long long registrar_inv_id = registrar->getId();

    try_insert_invoice_prefix();

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create());


    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        unsigned long long invoiceid = 0;
        Database::Date taxdate;

        taxdate = Database::Date(year,1,1);
        Money price = std::string("50000.00");//money

        Money out_credit;
        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                , zone_cz_id//zone
                , registrar_inv_id//registrar
                , price
                , boost::posix_time::ptime(taxdate), out_credit);//price
        BOOST_CHECK_EQUAL(invoiceid != 0,true);
        ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

        taxdate = Database::Date(year,6,10);
        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                , zone_cz_id//zone
                , registrar_inv_id//registrar
                , price
                , boost::posix_time::ptime(taxdate), out_credit);//price
        BOOST_CHECK_EQUAL(invoiceid != 0,true);
        ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);

        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

        taxdate = Database::Date(year,12,31);
        invoiceid = invMan->createDepositInvoice(taxdate//taxdate
                , zone_cz_id//zone
                , registrar_inv_id//registrar
                , price
                , boost::posix_time::ptime(taxdate), out_credit);//price
        BOOST_CHECK_EQUAL(invoiceid != 0,true);
        ::LibFred::Credit::add_credit_to_invoice( registrar_inv_id,  zone_cz_id, out_credit, invoiceid);


        //std::cout << "deposit invoice id: " << invoiceid << " year: " << year << " price: " << price << " registrar_handle: " << registrar_handle <<  " registrar_inv_id: " << registrar_inv_id << std::endl;

    }//for createDepositInvoice

     // credit before
    Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
                       , Database::query_param_list(zone_cz_id)(registrar_inv_id));
    Money credit_before("0");
    if(credit_res.size() ==  1 && credit_res[0].size() == 1)
        credit_before = std::string(credit_res[0][0]);

//    std::cout << "\ncreateAccountInvoices_registrar: " << registrar_handle
//            << " credit before: " << credit_before << std::endl;


    //try get epp reference
    ccReg::EPP_var epp_ref;
    epp_ref = ccReg::EPP::_narrow(
        CorbaContainer::get_instance()->nsresolve("EPP"));

    //login
    CORBA::ULongLong clientId = 0;
    ccReg::Response_var r;

    std::string test_domain_fqdn(std::string("tdomain")+time_string);
    Money test_operation_price ("0");

    try
    {
        CORBA::String_var registrar_handle_var = CORBA::string_dup(registrar_handle.c_str());
        CORBA::String_var passwd_var = CORBA::string_dup("");
        CORBA::String_var new_passwd_var = CORBA::string_dup("");
        CORBA::String_var cltrid_var = CORBA::string_dup("omg");
        CORBA::String_var xml_var = CORBA::string_dup("<omg/>");
        CORBA::String_var cert_var = CORBA::string_dup("");

        r = epp_ref->ClientLogin(
            registrar_handle_var,passwd_var,new_passwd_var,cltrid_var,
            xml_var,clientId,0,cert_var,ccReg::EN);

        if (r->code != 1000 || !clientId) {
            std::cerr << "Cannot connect: " << r->code << std::endl;
            throw std::runtime_error("Cannot connect ");
        }



        for (int i =0 ; i < 5000; i+=2)
        {
            ccReg::Period_str period;
            period.count = 1;
            period.unit = ccReg::unit_year;
            ccReg::EppParams epp_params;
            epp_params.requestID = clientId + i;
            epp_params.loginID = clientId;
            epp_params.clTRID = "";
            epp_params.XML = "";
            CORBA::String_var crdate;
            CORBA::String_var exdate;

            r = epp_ref->DomainCreate(
                    (test_domain_fqdn+"i"+boost::lexical_cast<std::string>(i)+".cz").c_str(), // fqdn
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
            test_operation_price+=renew_operation_price;

            ++i;

            ccReg::EppParams epp_params_renew;
            epp_params_renew.requestID = clientId+i;
            epp_params_renew.loginID = clientId;
            epp_params_renew.clTRID = "";
            epp_params_renew.XML = "";


            period.count = 3;
            CORBA::String_var exdate1;
            r = epp_ref->DomainRenew(
                    (test_domain_fqdn+"i"+boost::lexical_cast<std::string>(i-1)+".cz").c_str(), // fqdn
                    exdate,//curExpDate
                    period, //Period_str
                    exdate1,//out timestamp exDate,
                    epp_params_renew,//in EppParams params,
                    ccReg::ExtensionList()//in ExtensionList ext
                    );
            test_operation_price+=Decimal("3")*renew_operation_price;
        }

    }//try
    catch(ccReg::EPP::EppError &_epp_error)
    {
        handle_epp_exception(_epp_error);
    }
    catch(CORBA::TRANSIENT&)
    {
        Logging::Manager::instance_ref().error("Caught exception CORBA::TRANSIENT -- unable to contact the server." );
        std::cerr << "Caught exception CORBA::TRANSIENT -- unable to contact the "
             << "server." << std::endl;
        throw;
    }
    catch(CORBA::SystemException& ex)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::SystemException: ")+ex._name() );
        std::cerr << "Caught CORBA::SystemException" << ex._name() << std::endl;
        throw;
    }
    catch(CORBA::Exception& ex)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::Exception: ")+ex._name() );
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
        throw;
    }
    catch(omniORB::fatalException& fe)
    {
        std::string errmsg = std::string("Caught omniORB::fatalException: ")
                        + std::string("  file: ") + std::string(fe.file())
                        + std::string("  line: ") + boost::lexical_cast<std::string>(fe.line())
                        + std::string("  mesg: ") + std::string(fe.errmsg());
        Logging::Manager::instance_ref().error(errmsg);
        std::cerr << errmsg  << std::endl;
        throw;
    }

    // credit after
    Money credit_after_renew ("0");
    Database::Result credit_res3 = conn.exec_params(zone_registrar_credit_query
                           , Database::query_param_list(zone_cz_id)(registrar_inv_id));
    if(credit_res3.size() ==  1 && credit_res3[0].size() == 1)
        credit_after_renew = std::string(credit_res3[0][0]);
    //std::cout << "\n\t credit after renew: " << credit_after_renew << std::endl;

    //debug print
    if(credit_before - credit_after_renew != test_operation_price)
    {
        /*
        std::cout << "\ncredit_before: " << credit_before
                << " credit_after_renew: " << credit_after_renew
                << " test_operation_price: " << test_operation_price
                << std::endl;
                */
    }
    BOOST_CHECK(credit_before - credit_after_renew == test_operation_price);

    {
        date taxdate(boost::gregorian::from_simple_string((Database::Date(2001,1,31)).to_string()));
        date todate ( boost::gregorian::from_simple_string((Database::Date(2001,2,1)).to_string()));
        date fromdate(todate - months(1));

        invMan->createAccountInvoice( registrar_handle, std::string("cz")
            , taxdate 
            , fromdate//from_date not set
            , todate 
            , boost::posix_time::ptime(todate));
    }

    {   
        boost::gregorian::date todate( day_clock::local_day() );
        boost::gregorian::date fromdate( todate - months(1) );
        boost::gregorian::date taxdate(todate);

        invMan->createAccountInvoice( registrar_handle, std::string("cz")
            , taxdate
            , fromdate//from_date not set
            , todate 
            , boost::posix_time::ptime(todate));
    }

    {
        boost::gregorian::date todate( day_clock::local_day() );
        boost::gregorian::date fromdate( todate - months(1) );
        boost::gregorian::date taxdate(todate);

        BOOST_CHECK_EXCEPTION(
            invMan->createAccountInvoice( noregistrar_handle, std::string("cz")
                , taxdate
                , fromdate//from_date not set
                , todate
                , boost::posix_time::ptime(todate)
            )
            , std::exception
            , check_std_exception_createAccountInvoice );
    }

    //set operation price back
    conn.exec_params("update price_list set price = price - 0.11 where zone_id = $1::bigint and operation_id = 2", Database::query_param_list(zone_cz_id));
}


BOOST_AUTO_TEST_CASE( archiveInvoices_no_init )
{

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create());

    BOOST_CHECK_EXCEPTION(
    invMan->archiveInvoices(false)
    , std::exception, check_std_exception_archiveInvoices);
}

BOOST_AUTO_TEST_CASE( archiveInvoices )
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    init_corba_container();

    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
               ->get_handler_ptr_by_type<HandleRegistryArgs>();

    std::string corbaNS =ns_args_ptr->nameservice_host
            + ":"
            + boost::lexical_cast<std::string>(ns_args_ptr->nameservice_port);




    std::unique_ptr<::LibFred::Document::Manager> docMan(
              ::LibFred::Document::Manager::create(
                  registry_args_ptr->docgen_path
                  , registry_args_ptr->docgen_template_path
                  , registry_args_ptr->fileclient_path
                  , corbaNS)
              );

    //manager init
    MailerManager mailMan(CorbaContainer::get_instance()->getNS());
    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
        ::LibFred::Invoicing::Manager::create(
        docMan.get(),&mailMan));

    FileManagerClient fm_client(
             CorbaContainer::get_instance()->getNS());

    //call archive invoices and get processed invoice ids
    InvoiceIdVect inv_id_vect = invMan->archiveInvoices(false);

    //read processed invoices query
    std::string inv_query(
        "select i.zone_id, ((i.crdate at time zone 'UTC') at time zone 'Europe/Prague')::date, i.taxdate, i.prefix, i.registrar_id " // 0 - 4
        ", i.balance, i.operations_price, i.vat, i.total, i.totalvat, i.invoice_prefix_id, i.file , i.filexml " // 5 - 12
        ", ip.typ " // 13
        " from invoice_prefix ip join invoice i on i.invoice_prefix_id = ip.id where ");

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
            inv_query += " i.id = $" + boost::lexical_cast<std::string>(inv_query_params.size()) +"::bigint ";
        }
        else
        {//next id
            inv_query_params.push_back(*i);
            inv_query += "or i.id = $" + boost::lexical_cast<std::string>(inv_query_params.size()) +"::bigint ";
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
            ::LibFred::Banking::XMLparser parser;
            if (!parser.parse(xml)) throw std::runtime_error("parser error");

            ::LibFred::Banking::XMLnode root = parser.getRootNode();
            if (root.getName().compare("invoice") != 0) throw std::runtime_error("root xml element name is not \"invoice\"");

            ::LibFred::Banking::XMLnode delivery = root.getChild("delivery");
            if (delivery.getName().compare("delivery") != 0) throw std::runtime_error("xml element name is not \"delivery\"");

            ::LibFred::Banking::XMLnode payment = root.getChild("payment");
            if (payment.getName().compare("payment") != 0) throw std::runtime_error("xml element name is not \"payment\"");

            BOOST_CHECK(payment.getChild("invoice_number").getValue().compare(std::string(invoice_res[i][3])//invoice prefix
                        )==0);

            BOOST_CHECK(payment.getChild("invoice_date").getValue().compare(std::string(invoice_res[i][1])//invoice crdate::date
                        )==0);

            if(payment.hasChild("advance_payment_date"))
            {
                BOOST_CHECK(payment.getChild("advance_payment_date").getValue().compare(std::string(invoice_res[i][2])//invoice taxdate
                        )==0);
            }

            if(payment.hasChild("tax_point"))
            {
                BOOST_CHECK(payment.getChild("tax_point").getValue().compare(std::string(invoice_res[i][2])//invoice taxdate
                        )==0);
            }


            ::LibFred::Banking::XMLnode vat_rates = delivery.getChild("vat_rates");
            if (vat_rates.getName().compare("vat_rates") != 0) throw std::runtime_error("xml element name is not \"vat_rates\"");

            /*
            ::LibFred::Banking::XMLnode entry = vat_rates.getChild("entry");
            if (entry.getName().compare("entry") != 0) throw std::runtime_error("xml element name is not \"entry\"");

            if (invoice_prefix_typ == 1) //std::cout << "acc invoice" << std::endl
                ;

            //not working this way
            if(vat_rates.hasChild("entry"))
            {
                ::LibFred::Banking::XMLnode entry = vat_rates.getChild("entry");
                if (entry.getName().compare("entry") != 0) throw std::runtime_error("xml element name is not \"entry\"");

                if(
                        (entry.getChild("vatperc").getValue().compare(std::string(invoice_res[i][7])//invoice vat
                                )!=0)
                        
                        || ((entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][5])//invoice credit
                                )!=0) 
                                && (entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][6])//invoice price
                                    )!=0)
                                && (entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][8])//invoice total
                                    )!=0))
			    
			
                        || ((entry.getChild("vat").getValue().compare(std::string(invoice_res[i][9])//invoice totalvat
                                )!=0)
                                && (std::string("0.00").compare(std::string(invoice_res[i][9])//invoice totalvat
                                    )!=0))

                        || ((entry.getChild("total").getValue().compare(std::string(invoice_res[i][6])//invoice price
                                )!=0)
                                && (std::string("0.00").compare(std::string(invoice_res[i][8])//invoice total
                                    )!=0))
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
                
                BOOST_CHECK(
			    ((entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][5])//invoice credit
                            )==0)
                    || (entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][6])//invoice price
                            )==0)
                    || (entry.getChild("basetax").getValue().compare(std::string(invoice_res[i][8])//invoice total
                            )==0)) );

                BOOST_CHECK(
			    ((entry.getChild("vat").getValue().compare(std::string(invoice_res[i][9])//invoice totalvat
                            )==0)
                    || (std::string("0.00").compare(std::string(invoice_res[i][9])//invoice totalvat
                            )==0)) );

                BOOST_CHECK(
                    ((entry.getChild("total").getValue().compare(std::string(invoice_res[i][6])//invoice price
                        )==0)
                    || (std::string("0.00").compare(std::string(invoice_res[i][8])//invoice total
                        )==0)) );

            }
*/
            ::LibFred::Banking::XMLnode sumarize = delivery.getChild("sumarize");
            if (sumarize.getName().compare("sumarize") != 0) throw std::runtime_error("xml element name is not \"sumarize\"");


/*
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

                    << " db prefix typ: " << std::string( invoice_res[i][13])//invoice prefix typ

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
*/


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

BOOST_AUTO_TEST_SUITE_END(); // TestInvoiceAccount
