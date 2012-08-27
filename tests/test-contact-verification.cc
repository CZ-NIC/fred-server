/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
#include <sstream>
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

#include "time_clock.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"

#include "contact_verification/public_request_contact_verification_impl.h"

#include "setup_server_decl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_contactverification_args.h"



//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "mailer_manager.h"
#include "fredlib/contact_verification/contact.h"
#include "contact_verification/contact_verification_impl.h"

//test-contact-verification.cc

BOOST_AUTO_TEST_SUITE(TestContactVerification)

const std::string server_name = "test-contact-verification";



static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}


BOOST_AUTO_TEST_CASE( test_contact_verification )
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

    boost::shared_ptr<Fred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));
    const std::auto_ptr<Registry::Contact::Verification::ContactVerificationImpl> cv(
        new Registry::Contact::Verification::ContactVerificationImpl(server_name, mm));

    std::string registrar_handle = "REG-FRED_A";
    Fred::Contact::Verification::Contact fcvc;
    unsigned long long request_id =0;
    std::string another_request_id;
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //Database::Transaction trans(conn);

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        fcvc.handle=std::string("TESTCV-HANDLE")+xmark;
        fcvc.name=std::string("TESTCV NAME")+xmark;
        fcvc.organization=std::string("TESTCV-ORG")+xmark;
        fcvc.street1=std::string("TESTCV-STR1")+xmark;
        fcvc.city=std::string("Praha");
        fcvc.postalcode=std::string("11150");
        fcvc.country=std::string("CZ");
        fcvc.telephone=std::string("+420.728")+xmark;
        fcvc.email=std::string("test")+xmark+"@nic.cz";
        fcvc.ssn=std::string("1980-01-01");
        fcvc.ssntype=std::string("BIRTHDAY");
        fcvc.auth_info=rdg.xnstring(8);
        //unsigned long long contact_hid =

        fcvc.disclosename = true;
        fcvc.discloseorganization = true;
        fcvc.discloseaddress = true;
        fcvc.disclosetelephone = true;
        fcvc.disclosefax = true;
        fcvc.discloseemail = true;
        fcvc.disclosevat = true;
        fcvc.discloseident = true;
        fcvc.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);
        //trans.commit();
    }

    cv->createConditionalIdentification(fcvc.handle, registrar_handle
            , request_id, another_request_id);

    //check cci request
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res_cci_request = conn.exec_params(
            "select pr.status from object_registry obr "
            " join public_request_objects_map prom on obr.id = prom.object_id "
            " join public_request_auth pra on prom.request_id = pra.id "
            " join public_request pr on pr.id=pra.id "
            " join enum_public_request_type eprt on pr.request_type = eprt.id "
            " where obr.name = $1::text and eprt.name = $2::text "
            , Database::query_param_list(fcvc.handle)
                (Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
        BOOST_CHECK((res_cci_request.size() == 1)
                && (static_cast<int>(res_cci_request[0][0]) == Fred::PublicRequest::PRS_NEW));

        Database::Result res_cci_sms = conn.exec_params(
                "select pr.id ,  eprt.*, prmm.*, ma.*, mt.* from public_request pr "
                " join enum_public_request_type eprt on pr.request_type = eprt.id "
                " join public_request_auth pra on pr.id = pra.id "
                " join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                " join message_archive ma on ma.id = prmm.message_archive_id "
                " join message_type mt on mt.id = ma.message_type_id "
                " where pra.identification = $1::text and eprt.name = $2::text "
                " and mt.type='contact_verification_pin2' "
            , Database::query_param_list(another_request_id)
                (Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
        BOOST_CHECK((res_cci_sms.size() == 1));

        Database::Result res_cci_email = conn.exec_params(
                "select pr.id ,  eprt.*, prmm.*, ma.*, mt.* from public_request pr "
                " join enum_public_request_type eprt on pr.request_type = eprt.id "
                " join public_request_auth pra on pr.id = pra.id "
                " join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                " join mail_archive ma on ma.id = prmm.mail_archive_id "
                " join mail_type mt on mt.id = ma.mailtype "
                " where pra.identification = $1::text and eprt.name = $2::text "
                " and mt.name = 'conditional_contact_identification' "
            , Database::query_param_list(another_request_id)
                (Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
        BOOST_CHECK((res_cci_email.size() == 1));
    }

    BOOST_TEST_MESSAGE( "identification: " << another_request_id );

    std::string password("testtest");
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
                "SELECT password FROM public_request_auth  WHERE identification=$1::text",
                Database::query_param_list(another_request_id));
        if(res_pass.size() == 1)
        {
            password = std::string(res_pass[0][0]);
        }
        else
        {
          BOOST_TEST_MESSAGE( "test password not found");
        }
    }

    BOOST_TEST_MESSAGE( "password: " << password );

    cv->processConditionalIdentification(another_request_id
            , password, request_id);

    //check cci request 1
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res_cci_request = conn.exec_params(
            "select pr.status from object_registry obr "
            " join public_request_objects_map prom on obr.id = prom.object_id "
            " join public_request_auth pra on prom.request_id = pra.id "
            " join public_request pr on pr.id=pra.id "
            " join enum_public_request_type eprt on pr.request_type = eprt.id "
            " where obr.name = $1::text and eprt.name = $2::text "
            , Database::query_param_list(fcvc.handle)
                (Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
        BOOST_CHECK((res_cci_request.size() == 1)
                && (static_cast<int>(res_cci_request[0][0]) == Fred::PublicRequest::PRS_ANSWERED));
    }
    //check ci request 0
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res_cci_request = conn.exec_params(
            "select pr.status from object_registry obr "
            " join public_request_objects_map prom on obr.id = prom.object_id "
            " join public_request_auth pra on prom.request_id = pra.id "
            " join public_request pr on pr.id=pra.id "
            " join enum_public_request_type eprt on pr.request_type = eprt.id "
            " where obr.name = $1::text and eprt.name = $2::text "
            , Database::query_param_list(fcvc.handle)
                (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION));
        BOOST_CHECK((res_cci_request.size() == 1)
                && (static_cast<int>(res_cci_request[0][0]) == Fred::PublicRequest::PRS_NEW));

        Database::Result res_cci_letter = conn.exec_params(

                "select obr.name,pr.id ,  eprt.*, prmm.*, ma.*, mt.* from object_registry obr "
                " join public_request_objects_map prom on obr.id = prom.object_id "
                " join public_request_auth pra on prom.request_id = pra.id "
                " join public_request pr on pr.id=pra.id "
                " join enum_public_request_type eprt on pr.request_type = eprt.id "
                " join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                " join message_archive ma on ma.id = prmm.message_archive_id "
                " join message_type mt on mt.id = ma.message_type_id "
                " where obr.name = $1::text and eprt.name = $2::text and "
                " mt.type='contact_verification_pin3' "
            , Database::query_param_list(another_request_id)
                (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION));
        BOOST_CHECK((res_cci_letter.size() == 1));
    }





    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
            "select pra.password from object_registry obr "
            " join public_request_objects_map prom on obr.id = prom.object_id "
            " join public_request_auth pra on prom.request_id = pra.id "
            " join public_request pr on pr.id=pra.id "
            " join enum_public_request_type eprt on pr.request_type = eprt.id "
            " where obr.name = $1::text "//'TESTCV-HANDLE356681'
            " and eprt.name = $2::text "//'contact_identification'
            , Database::query_param_list(fcvc.handle)
                (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION));
        if(res_pass.size() == 1)
        {
            password = std::string(res_pass[0][0]);
        }
        else
        {
          BOOST_TEST_MESSAGE( "test password not found");
        }
    }

    BOOST_TEST_MESSAGE( "password: " << password );

    cv->processIdentification(fcvc.handle, password, request_id);

    //check ci request 1
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res_cci_request = conn.exec_params(
            "select pr.status from object_registry obr "
            " join public_request_objects_map prom on obr.id = prom.object_id "
            " join public_request_auth pra on prom.request_id = pra.id "
            " join public_request pr on pr.id=pra.id "
            " join enum_public_request_type eprt on pr.request_type = eprt.id "
            " where obr.name = $1::text and eprt.name = $2::text "
            , Database::query_param_list(fcvc.handle)
                (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION));
        BOOST_CHECK((res_cci_request.size() == 1)
                && (static_cast<int>(res_cci_request[0][0]) == Fred::PublicRequest::PRS_ANSWERED));
    }


    BOOST_CHECK(cv->getRegistrarName(registrar_handle) == "Company A l.t.d");
}



BOOST_AUTO_TEST_SUITE_END();//TestContactVerification
