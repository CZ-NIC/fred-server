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

#include "src/mojeid/mojeid.h"
#include "src/mojeid/mojeid_contact_states.h"

#include "setup_server_decl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_mojeid_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include "src/corba/mailer_manager.h"
#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/object_states.h"
#include "src/contact_verification/contact_verification_impl.h"

#include "checks.h"

//test-mojeid.cc

BOOST_AUTO_TEST_SUITE(TestMojeID)

const std::string server_name = "test-mojeid";

static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

BOOST_AUTO_TEST_CASE( test_create_mojeid )
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

    std::auto_ptr<Fred::Manager> registry_manager_;

    boost::shared_ptr<Fred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));

    const std::auto_ptr<Registry::MojeID::MojeIDImpl> mojeid_pimpl(new Registry::MojeID::MojeIDImpl(server_name, mm));

    std::string registrar_handle = "REG-FRED_A";
    Fred::Contact::Verification::Contact fcvc;
    unsigned long long request_id =0;
    std::string trans_id;
    std::string identification;

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        fcvc.handle=std::string("TESTMOJEID-HANDLE")+xmark;
        fcvc.name=std::string("TESTMOJEID NAME")+xmark;
        fcvc.organization=std::string();
        fcvc.street1=std::string("TESTMOJEID-STR1")+xmark;
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

        trans_id = xmark;
        request_id = boost::lexical_cast<unsigned long long>(xmark);
    }
    //contact_create is called here
    unsigned long long contact_id = mojeid_pimpl->contactCreatePrepare(
            fcvc.handle, fcvc,trans_id.c_str(), request_id, identification);

    mojeid_pimpl->commitPreparedTransaction(trans_id.c_str());

    //check initial MojeID contact state
    check_public_request_on_contact(fcvc
            , Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION
            , Fred::PublicRequest::PRS_NEW);

    std::string password("testtest");
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
                "SELECT password FROM public_request_auth  WHERE identification=$1::text",
                Database::query_param_list(identification));
        if(res_pass.size() == 1)
        {
            password = std::string(res_pass[0][0]);
        }
        else
        {
          BOOST_TEST_MESSAGE( "test password not found");
        }

        //check contact_id
        BOOST_CHECK(contact_id == static_cast<unsigned long long>(
                conn.exec_params("select c.id from contact c "
                   " join object_registry obr on c.id = obr.id "
                   " where obr.name = $1::text "
                   ,Database::query_param_list(fcvc.handle))[0][0]));

        BOOST_CHECK(mojeid_pimpl->getContactId(fcvc.handle) == contact_id);

        //check contact not in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
            , MojeID::ObjectState::MOJEID_CONTACT) == false);

        //check contact not in cci state
        BOOST_CHECK(Fred::object_has_state(contact_id
            , Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT) == false);
    }

    BOOST_TEST_MESSAGE( "password: " << password );

    mojeid_pimpl->processIdentification(identification.c_str()
            , password.c_str(), request_id);

    {
        //check new mojeid cci request
        check_public_request_on_contact(fcvc
                , Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION
                , Fred::PublicRequest::PRS_ANSWERED);

        //check mojeid ci request new (0)
        check_public_request_on_contact(fcvc
                , Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION
                , Fred::PublicRequest::PRS_NEW);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in cci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));

    }

    identification = mojeid_pimpl->getIdentificationInfo(contact_id);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
                "SELECT password FROM public_request_auth  WHERE identification=$1::text",
                Database::query_param_list(identification));
        if(res_pass.size() == 1)
        {
            password = std::string(res_pass[0][0]);
        }
        else
        {
          BOOST_TEST_MESSAGE( "test password not found");
        }
    }

    mojeid_pimpl->processIdentification(identification.c_str()
                , password.c_str(), request_id);

    {
        //check mojeid ci request answered
        check_public_request_on_contact(fcvc
                , Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION
                , Fred::PublicRequest::PRS_ANSWERED);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::IDENTIFIED_CONTACT));

    }

    {//validation request is not there yet, exception expected
        std::stringstream outstr;
        BOOST_CHECK_EXCEPTION(mojeid_pimpl->getValidationPdf(contact_id, outstr)
                , std::exception
                , check_std_exception);
    }

    mojeid_pimpl->createValidationRequest(contact_id
            , request_id);

    {//check validation request
        std::stringstream outstr;
        mojeid_pimpl->getValidationPdf(contact_id, outstr);
        unsigned long size = outstr.str().size();
        BOOST_CHECK(size > 0);
    }

    {//nice init
        Database::Connection conn = Database::Manager::acquire();
        DBSharedPtr  db_sh_ptr;
        db_sh_ptr.reset(new DB(conn));
        HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleRegistryArgs>();
        registry_manager_.reset(Fred::Manager::create(db_sh_ptr
                , registry_args_ptr->restricted_handles));
        registry_manager_->initStates();

        std::auto_ptr<Fred::Document::Manager> doc_manager(
                Fred::Document::Manager::create(
                        registry_args_ptr->docgen_path,
                        registry_args_ptr->docgen_template_path,
                        registry_args_ptr->fileclient_path,
                        ns_args_ptr->get_nameservice_host_port()
                        ));

        std::auto_ptr<Fred::PublicRequest::Manager> request_manager(
                Fred::PublicRequest::Manager::create(
                    registry_manager_->getDomainManager(),
                    registry_manager_->getContactManager(),
                    registry_manager_->getNSSetManager(),
                    registry_manager_->getKeySetManager(),
                    mm.get(),
                    doc_manager.get(),
                    registry_manager_->getMessageManager())
                );

        //check for new validation public request
        unsigned long long validation_public_request_id
            = check_public_request_on_contact(fcvc
                        , Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION
                        , Fred::PublicRequest::PRS_NEW);

         //check contact in ci state
         BOOST_CHECK(Fred::object_has_state(contact_id
         ,Fred::ObjectState::IDENTIFIED_CONTACT));

         //check contact not in cv state
         BOOST_CHECK(Fred::object_has_state(contact_id
         ,Fred::ObjectState::VALIDATED_CONTACT) == false);

        request_manager->processRequest(validation_public_request_id,false,true, request_id);
    }

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check mojeid cv request new (0)
        check_public_request_on_contact(fcvc
                                , Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION
                                , Fred::PublicRequest::PRS_ANSWERED);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::IDENTIFIED_CONTACT));

        //check contact in cv state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::VALIDATED_CONTACT));
    }
}

BOOST_AUTO_TEST_CASE( test_transfer_mojeid )
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

    std::auto_ptr<Fred::Manager> registry_manager_;

    boost::shared_ptr<Fred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));

    const std::auto_ptr<Registry::MojeID::MojeIDImpl> mojeid_pimpl(new Registry::MojeID::MojeIDImpl(server_name, mm));

    std::string registrar_handle = "REG-FRED_A";
    Fred::Contact::Verification::Contact fcvc;
    unsigned long long request_id =0;
    std::string trans_id;
    std::string identification;

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

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
        fcvc.handle=std::string("TESTMOJEID-HANDLE")+xmark;
        fcvc.name=std::string("TESTMOJEID NAME")+xmark;
        fcvc.organization=std::string();
        fcvc.street1=std::string("TESTMOJEID-STR1")+xmark;
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

        trans_id = xmark;
        request_id = boost::lexical_cast<unsigned long long>(xmark);
        Fred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);
    }

    //contact_transfer is called here
    unsigned long long contact_id = mojeid_pimpl->contactTransferPrepare(
                fcvc.handle.c_str(),trans_id.c_str(), request_id, identification);

    mojeid_pimpl->commitPreparedTransaction(trans_id.c_str());

    //check initial MojeID contact state
    check_public_request_on_contact(fcvc
            , Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION
            , Fred::PublicRequest::PRS_NEW);

    std::string password("testtest");
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
                "SELECT password FROM public_request_auth  WHERE identification=$1::text",
                Database::query_param_list(identification));
        if(res_pass.size() == 1)
        {
            password = std::string(res_pass[0][0]);
        }
        else
        {
          BOOST_TEST_MESSAGE( "test password not found");
        }

        //check contact_id
        BOOST_CHECK(contact_id == static_cast<unsigned long long>(
                conn.exec_params("select c.id from contact c "
                   " join object_registry obr on c.id = obr.id "
                   " where obr.name = $1::text "
                   ,Database::query_param_list(fcvc.handle))[0][0]));

        BOOST_CHECK(mojeid_pimpl->getContactId(fcvc.handle) == contact_id);

        //check contact not in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
            , MojeID::ObjectState::MOJEID_CONTACT) == false);

        //check contact not in cci state
        BOOST_CHECK(Fred::object_has_state(contact_id
            , Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT) == false);
    }

    BOOST_TEST_MESSAGE( "password: " << password );

    mojeid_pimpl->processIdentification(identification.c_str()
            , password.c_str(), request_id);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check new mojeid cci request
        check_public_request_on_contact(fcvc
                , Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION
                , Fred::PublicRequest::PRS_ANSWERED);

        //check mojeid ci request new (0)
        check_public_request_on_contact(fcvc
                , Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION
                , Fred::PublicRequest::PRS_NEW);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in cci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));

    }

    identification = mojeid_pimpl->getIdentificationInfo(contact_id);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
                "SELECT password FROM public_request_auth  WHERE identification=$1::text",
                Database::query_param_list(identification));
        if(res_pass.size() == 1)
        {
            password = std::string(res_pass[0][0]);
        }
        else
        {
          BOOST_TEST_MESSAGE( "test password not found");
        }
    }

    mojeid_pimpl->processIdentification(identification.c_str()
                , password.c_str(), request_id);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check mojeid ci request answered
        check_public_request_on_contact(fcvc
                , Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION
                , Fred::PublicRequest::PRS_ANSWERED);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::IDENTIFIED_CONTACT));

    }

    {//validation request is not there yet, exception expected
        std::stringstream outstr;
        BOOST_CHECK_EXCEPTION(mojeid_pimpl->getValidationPdf(contact_id, outstr)
                , std::exception
                , check_std_exception);
    }

    mojeid_pimpl->createValidationRequest(contact_id
            , request_id);

    {//check validation request
        std::stringstream outstr;
        mojeid_pimpl->getValidationPdf(contact_id, outstr);
        unsigned long size = outstr.str().size();
        BOOST_CHECK(size > 0);
    }

    {//nice init
        Database::Connection conn = Database::Manager::acquire();
        DBSharedPtr  db_sh_ptr;
        db_sh_ptr.reset(new DB(conn));
        HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleRegistryArgs>();
        registry_manager_.reset(Fred::Manager::create(db_sh_ptr
                , registry_args_ptr->restricted_handles));
        registry_manager_->initStates();

        std::auto_ptr<Fred::Document::Manager> doc_manager(
                Fred::Document::Manager::create(
                        registry_args_ptr->docgen_path,
                        registry_args_ptr->docgen_template_path,
                        registry_args_ptr->fileclient_path,
                        ns_args_ptr->get_nameservice_host_port()
                        ));

        std::auto_ptr<Fred::PublicRequest::Manager> request_manager(
                Fred::PublicRequest::Manager::create(
                    registry_manager_->getDomainManager(),
                    registry_manager_->getContactManager(),
                    registry_manager_->getNSSetManager(),
                    registry_manager_->getKeySetManager(),
                    mm.get(),
                    doc_manager.get(),
                    registry_manager_->getMessageManager())
                );

        //check for new validation public request
        unsigned long long validation_public_request_id
            = check_public_request_on_contact(fcvc
                        , Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION
                        , Fred::PublicRequest::PRS_NEW);

         //check contact in ci state
         BOOST_CHECK(Fred::object_has_state(contact_id
         ,Fred::ObjectState::IDENTIFIED_CONTACT));

         //check contact not in cv state
         BOOST_CHECK(Fred::object_has_state(contact_id
         ,Fred::ObjectState::VALIDATED_CONTACT) == false);

        request_manager->processRequest(validation_public_request_id,false,true, request_id);
    }

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check mojeid cv request new (0)
        check_public_request_on_contact(fcvc
                                , Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION
                                , Fred::PublicRequest::PRS_ANSWERED);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::IDENTIFIED_CONTACT));

        //check contact in cv state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::VALIDATED_CONTACT));
    }
}

std::pair< ::size_t, std::string > create_mojeid_contact(const std::string &xmark,
    Registry::MojeID::MojeIDImpl &mojeid_pimpl,
    RandomDataGenerator &rdg,
    HandleCorbaNameServiceArgs *ns_args_ptr,
    boost::shared_ptr< Fred::Mailer::Manager > &mm)
{
    std::auto_ptr< Fred::Manager > registry_manager;
    std::string registrar_handle = "REG-FRED_A";
    Fred::Contact::Verification::Contact fcvc;
    unsigned long long request_id = 0;
    std::string trans_id;
    std::string identification;
    std::pair< ::size_t, std::string > result;

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }

        //create test contact
        fcvc.handle = std::string("TESTMOJEID-HANDLE") + xmark;
        result.second = fcvc.handle;
        fcvc.name = std::string("TESTMOJEID NAME") + xmark;
        fcvc.organization = std::string();
        fcvc.street1 = std::string("TESTMOJEID-STR1") + xmark;
        fcvc.city = std::string("Praha");
        fcvc.postalcode = std::string("11150");
        fcvc.country = std::string("CZ");
        fcvc.telephone = std::string("+420.728") + xmark.substr(1, 6);
        fcvc.email = std::string("test") + xmark + "@nic.cz";
        fcvc.ssn = std::string("1980-01-01");
        fcvc.ssntype = std::string("BIRTHDAY");
        fcvc.auth_info = rdg.xnstring(8);

        fcvc.disclosename = true;
        fcvc.discloseorganization = true;
        fcvc.discloseaddress = true;
        fcvc.disclosetelephone = true;
        fcvc.disclosefax = true;
        fcvc.discloseemail = true;
        fcvc.disclosevat = true;
        fcvc.discloseident = true;
        fcvc.disclosenotifyemail = true;

        trans_id = xmark;
        request_id = boost::lexical_cast< unsigned long long >(xmark);
    }
    //contact_create is called here
    const unsigned long long contact_id = mojeid_pimpl.contactCreatePrepare(
        fcvc.handle, fcvc, trans_id.c_str(), request_id, identification);
    result.first = contact_id;

    mojeid_pimpl.commitPreparedTransaction(trans_id.c_str());

    //check initial MojeID contact state
    check_public_request_on_contact(fcvc
            , Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION
            , Fred::PublicRequest::PRS_NEW);

    std::string password("testtest");
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
            "SELECT password FROM public_request_auth  WHERE identification=$1::text",
            Database::query_param_list(identification));
        if (res_pass.size() == 1) {
            password = std::string(res_pass[0][0]);
        }
        else {
          BOOST_TEST_MESSAGE("test password not found");
        }

        //check contact_id
        BOOST_CHECK(contact_id == static_cast< unsigned long long >(
            conn.exec_params("SELECT c.id FROM contact c "
                             "JOIN object_registry obr ON c.id=obr.id "
                             "WHERE obr.name=$1::text",
                             Database::query_param_list(fcvc.handle))[0][0]));

        BOOST_CHECK(mojeid_pimpl.getContactId(fcvc.handle) == contact_id);

        //check contact not in mojeid state
        BOOST_CHECK(!Fred::object_has_state(contact_id,
            MojeID::ObjectState::MOJEID_CONTACT));

        //check contact not in cci state
        BOOST_CHECK(!Fred::object_has_state(contact_id,
            Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));
    }

    BOOST_TEST_MESSAGE("password: " << password);

    mojeid_pimpl.processIdentification(identification.c_str(), password.c_str(), request_id);

    {
        //check new mojeid cci request
        check_public_request_on_contact(fcvc,
            Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION,
            Fred::PublicRequest::PRS_ANSWERED);

        //check mojeid ci request new (0)
        check_public_request_on_contact(fcvc,
            Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION,
            Fred::PublicRequest::PRS_NEW);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in cci state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));

    }

    identification = mojeid_pimpl.getIdentificationInfo(contact_id);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_pass = conn.exec_params(
                "SELECT password FROM public_request_auth  WHERE identification=$1::text",
                Database::query_param_list(identification));
        if (res_pass.size() == 1) {
            password = std::string(res_pass[0][0]);
        }
        else {
          BOOST_TEST_MESSAGE( "test password not found");
        }
    }

    mojeid_pimpl.processIdentification(identification.c_str(), password.c_str(), request_id);

    {
        //check mojeid ci request answered
        check_public_request_on_contact(fcvc,
            Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION,
            Fred::PublicRequest::PRS_ANSWERED);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            Fred::ObjectState::IDENTIFIED_CONTACT));

    }

    {//validation request is not there yet, exception expected
        std::stringstream outstr;
        BOOST_CHECK_EXCEPTION(mojeid_pimpl.getValidationPdf(contact_id, outstr),
            std::exception,
            check_std_exception);
    }

    mojeid_pimpl.createValidationRequest(contact_id, request_id);

    {//check validation request
        std::stringstream outstr;
        mojeid_pimpl.getValidationPdf(contact_id, outstr);
        const ::size_t size = outstr.str().size();
        BOOST_CHECK(0 < size);
    }

    {//nice init
        Database::Connection conn = Database::Manager::acquire();
        DBSharedPtr db_sh_ptr;
        db_sh_ptr.reset(new DB(conn));
        HandleRegistryArgs *registry_args_ptr = CfgArgs::instance()
            ->get_handler_ptr_by_type< HandleRegistryArgs >();
        registry_manager.reset(Fred::Manager::create(db_sh_ptr,
            registry_args_ptr->restricted_handles));
        registry_manager->initStates();

        std::auto_ptr< Fred::Document::Manager > doc_manager(
            Fred::Document::Manager::create(
                registry_args_ptr->docgen_path,
                registry_args_ptr->docgen_template_path,
                registry_args_ptr->fileclient_path,
                ns_args_ptr->get_nameservice_host_port()
                ));

        std::auto_ptr<Fred::PublicRequest::Manager> request_manager(
            Fred::PublicRequest::Manager::create(
                registry_manager->getDomainManager(),
                registry_manager->getContactManager(),
                registry_manager->getNSSetManager(),
                registry_manager->getKeySetManager(),
                mm.get(),
                doc_manager.get(),
                registry_manager->getMessageManager())
            );

        //check for new validation public request
        const unsigned long long validation_public_request_id =
            check_public_request_on_contact(fcvc,
                Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION,
                Fred::PublicRequest::PRS_NEW);

        //check contact in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            Fred::ObjectState::IDENTIFIED_CONTACT));

        //check contact not in cv state
        BOOST_CHECK(!Fred::object_has_state(contact_id,
            Fred::ObjectState::VALIDATED_CONTACT));

        request_manager->processRequest(validation_public_request_id, false, true, request_id);
    }

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check mojeid cv request new (0)
        check_public_request_on_contact(fcvc,
            Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION,
            Fred::PublicRequest::PRS_ANSWERED);

        //check contact in mojeid state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            MojeID::ObjectState::MOJEID_CONTACT));

        //check contact in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            Fred::ObjectState::IDENTIFIED_CONTACT));

        //check contact in cv state
        BOOST_CHECK(Fred::object_has_state(contact_id,
            Fred::ObjectState::VALIDATED_CONTACT));
    }
    return result;
}

struct create_mojeid_contact_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::pair< ::size_t, std::string > contact_a;
    std::pair< ::size_t, std::string > contact_b;
    std::pair< ::size_t, std::string > contact_c;
    std::pair< ::size_t, std::string > contact_d;
    std::set< ::size_t > contacts;
    FakedArgs fa;
    HandleCorbaNameServiceArgs *ns_args_ptr;
    boost::shared_ptr< Fred::Mailer::Manager > mm;
    std::auto_ptr< Registry::MojeID::MojeIDImpl > mojeid_pimpl;

    create_mojeid_contact_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        fa(CfgArgs::instance()->fa),
        ns_args_ptr(CfgArgs::instance()->get_handler_ptr_by_type< HandleCorbaNameServiceArgs >())
    {
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv(),
            ns_args_ptr->nameservice_host,
            ns_args_ptr->nameservice_port,
            ns_args_ptr->nameservice_context);
        mm = boost::shared_ptr< Fred::Mailer::Manager >(new MailerManager(CorbaContainer::get_instance()->getNS()));
        mojeid_pimpl = std::auto_ptr< Registry::MojeID::MojeIDImpl >(new Registry::MojeID::MojeIDImpl(server_name, mm));
        RandomDataGenerator rdg;
        contact_a = create_mojeid_contact(xmark + "0", *mojeid_pimpl, rdg, ns_args_ptr, mm);
        contacts.insert(contact_a.first);
        contact_b = create_mojeid_contact(xmark + "1", *mojeid_pimpl, rdg, ns_args_ptr, mm);
        contacts.insert(contact_b.first);
        contact_c = create_mojeid_contact(xmark + "2", *mojeid_pimpl, rdg, ns_args_ptr, mm);
        contacts.insert(contact_c.first);
        contact_d = create_mojeid_contact(xmark + "3", *mojeid_pimpl, rdg, ns_args_ptr, mm);
        contacts.insert(contact_d.first);
        {
            //get db connection
            Database::Connection conn = Database::Manager::acquire();
            conn.exec_params(
                "UPDATE object_state "
                "SET valid_from=valid_from-INTERVAL '2 HOURS' "
                "WHERE object_id IN ($1::bigint,$2::bigint,$3::bigint,$4::bigint) AND "
                      "state_id=(SELECT id FROM enum_object_states WHERE name='mojeidContact')",
                Database::query_param_list(contact_a.first)
                (contact_b.first)
                (contact_c.first)
                (contact_d.first));
            conn.exec_params(
                "UPDATE object_state "
                "SET valid_from=valid_from-INTERVAL '30 MINUTES' "
                "WHERE object_id IN ($1::bigint,$2::bigint,$3::bigint,$4::bigint) AND "
                      "state_id IN (SELECT id FROM enum_object_states WHERE name IN ("
                                       "'conditionallyIdentifiedContact',"
                                       "'identifiedContact',"
                                       "'validatedContact'))",
                Database::query_param_list(contact_a.first)
                (contact_b.first)
                (contact_c.first)
                (contact_d.first));
        }
    }
    ~create_mojeid_contact_fixture()
    {}
};

struct StateHistory
{
    StateHistory(::uint8_t history):present(history & 0x01),
                                    changed(((history & 0x1f) != 0x1f) &&
                                            ((history & 0x1f) != 0x00)) { }
    const bool present;
    const bool changed;
};
/* 
 * -2h ___-1h  ___0h
 *  __|   |___|     state
 *  |......|......|
 *  0 1 1 0 0 1 1 1 history
 */
StateHistory set_object_state_history(::size_t object_id, ::uint8_t history, const std::string &state)
{
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params(
        "DELETE FROM object_state "
        "WHERE object_id=$1::bigint AND "
              "state_id=(SELECT id FROM enum_object_states WHERE name=$2::text)",
        Database::query_param_list(object_id)(state));
    if (state == "conditionallyIdentifiedContact") {
        history |= 0x01;//conditionallyIdentifiedContact mustn't absent
    }
    if (history == 0) {
        return StateHistory(history);
    }
    Database::query_param_list param(object_id);
    const ::size_t state_id =
        static_cast< ::size_t >(
            conn.exec_params("SELECT id FROM enum_object_states WHERE name=$1::text",
                             Database::query_param_list(state))[0][0]);
    param(state_id);
    std::ostringstream values;
    bool previous = false;
    for (int idx = 0; idx < 8; ++idx) {
        const ::uint8_t mask = 0x80 >> idx;
        const bool current = history & mask;
        if (previous != current) {
            const int dT = ((120 * (7 - idx)) / 7);
            std::ostringstream in_past;
            in_past << dT << " MINUTES";
            param(in_past.str());
            if (current) {
                if (!values.str().empty()) {
                    values << ",";
                }
                values << "($1::bigint,$2::bigint,NOW()-$" << param.size() << "::INTERVAL,";
            }
            else {
                values << "NOW()-$" << param.size() << "::INTERVAL)";
            }
            previous = current;
        }
    }
    if (previous) {
        values << "NULL)";
    }
    const std::string sql = "INSERT INTO object_state "
                                "(object_id,state_id,valid_from,valid_to) VALUES" +
                            values.str();
    conn.exec_params(sql, param);
    return StateHistory(history);
}

typedef std::set< std::string > CurrentState;
struct ChangedCurrentState
{
    bool changed;
    CurrentState state;
};

ChangedCurrentState set_contact_state_history(::size_t object_id, const ::uint8_t history[3])
{
    ChangedCurrentState result;
    result.changed = false;
    for (::size_t idx = 0; idx < 3; ++idx) {
        static const char *const state[] = {
            "conditionallyIdentifiedContact",
            "identifiedContact",
            "validatedContact"};
        const StateHistory sh = set_object_state_history(object_id, history[idx], state[idx]);
        result.changed |= sh.changed;
        if (sh.present) {
            result.state.insert(state[idx]);
        }
    }
    result.state.insert("mojeidContact");
    return result;
}

BOOST_FIXTURE_TEST_CASE(get_contacts_state_changes, create_mojeid_contact_fixture)
{
    typedef Registry::MojeID::ContactStateData StateData;
    typedef std::vector< Registry::MojeID::ContactStateData > StatesData;
    typedef Registry::MojeID::ContactStateData::StateValidFrom StateValidFrom;
    try {
        const StatesData states = mojeid_pimpl->getContactsStateChanges(1);
        for (StatesData::const_iterator data_ptr = states.begin(); data_ptr != states.end(); ++data_ptr) {
            if (contacts.count(data_ptr->contact_id) == 0) {
                continue;
            }
            StateData state_data = mojeid_pimpl->getContactState(data_ptr->contact_id);
            BOOST_CHECK(data_ptr->state.find("mojeidContact") != data_ptr->state.end());
            BOOST_CHECK(data_ptr->contact_id == state_data.contact_id);
            BOOST_CHECK(data_ptr->state.size() == state_data.state.size());
            for (StateValidFrom::const_iterator state_ptr = data_ptr->state.begin(); state_ptr != data_ptr->state.end(); ++state_ptr) {
                BOOST_CHECK(state_data.state.count(state_ptr->first) == 1);
                BOOST_CHECK(state_data.state[state_ptr->first] == state_ptr->second);
            }
        }
    }
    catch (const std::exception &e) {
        BOOST_TEST_MESSAGE(std::string("unexpected exception: ") + e.what());
    }
    for (int history = 0; history <= 0xff; ++history) {
        const ::uint8_t csha[3] = {::uint8_t(history), ::uint8_t(0), ::uint8_t(0)};
        const ChangedCurrentState ca = set_contact_state_history(contact_a.first, csha);
        const ::uint8_t cshb[3] = {::uint8_t(0), ::uint8_t(history), ::uint8_t(0)};
        const ChangedCurrentState cb = set_contact_state_history(contact_b.first, cshb);
        const ::uint8_t cshc[3] = {::uint8_t(0), ::uint8_t(0), ::uint8_t(history)};
        const ChangedCurrentState cc = set_contact_state_history(contact_c.first, cshc);
        const ::uint8_t cshd[3] = {::uint8_t(history), ::uint8_t(history), ::uint8_t(history)};
        const ChangedCurrentState cd = set_contact_state_history(contact_d.first, cshd);
        const StatesData states = mojeid_pimpl->getContactsStateChanges(1);
        bool ca_find = !ca.changed;
        bool cb_find = !cb.changed;
        bool cc_find = !cc.changed;
        bool cd_find = !cd.changed;
        for (StatesData::const_iterator data_ptr = states.begin(); data_ptr != states.end(); ++data_ptr) {
            BOOST_CHECK(data_ptr->state.find("mojeidContact") != data_ptr->state.end());
            if (data_ptr->contact_id == contact_a.first) {
                BOOST_CHECK(ca.changed);
                if (ca.changed) {
                    ca_find = true;
                    BOOST_CHECK(data_ptr->state.size() == ca.state.size());
                    if (data_ptr->state.size() == ca.state.size()) {
                        for (StateValidFrom::const_iterator state_ptr = data_ptr->state.begin(); state_ptr != data_ptr->state.end(); ++state_ptr) {
                            BOOST_CHECK(ca.state.count(state_ptr->first) == 1);
                        }
                    }
                }
            }

            if (data_ptr->contact_id == contact_b.first) {
                BOOST_CHECK(cb.changed);
                if (cb.changed) {
                    cb_find = true;
                    BOOST_CHECK(data_ptr->state.size() == cb.state.size());
                    if (data_ptr->state.size() == cb.state.size()) {
                        for (StateValidFrom::const_iterator state_ptr = data_ptr->state.begin(); state_ptr != data_ptr->state.end(); ++state_ptr) {
                            BOOST_CHECK(cb.state.count(state_ptr->first) == 1);
                        }
                    }
                }
            }

            if (data_ptr->contact_id == contact_c.first) {
                BOOST_CHECK(cc.changed);
                if (cc.changed) {
                    cc_find = true;
                    BOOST_CHECK(data_ptr->state.size() == cc.state.size());
                    if (data_ptr->state.size() == cc.state.size()) {
                        for (StateValidFrom::const_iterator state_ptr = data_ptr->state.begin(); state_ptr != data_ptr->state.end(); ++state_ptr) {
                            BOOST_CHECK(cc.state.count(state_ptr->first) == 1);
                        }
                    }
                }
            }

            if (data_ptr->contact_id == contact_d.first) {
                BOOST_CHECK(cd.changed);
                if (cd.changed) {
                    cd_find = true;
                    BOOST_CHECK(data_ptr->state.size() == cd.state.size());
                    if (data_ptr->state.size() == cd.state.size()) {
                        for (StateValidFrom::const_iterator state_ptr = data_ptr->state.begin(); state_ptr != data_ptr->state.end(); ++state_ptr) {
                            BOOST_CHECK(cd.state.count(state_ptr->first) == 1);
                        }
                    }
                }
            }
        }
        BOOST_CHECK(ca_find);
        BOOST_CHECK(cb_find);
        BOOST_CHECK(cc_find);
        BOOST_CHECK(cd_find);
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestMojeID
