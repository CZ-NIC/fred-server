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
        fcvc.organization=std::string("TESTMOJEID-ORG")+xmark;
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

        //check contact not in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::IDENTIFIED_CONTACT)==false);

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
        fcvc.organization=std::string("TESTMOJEID-ORG")+xmark;
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

        //check contact not in ci state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::IDENTIFIED_CONTACT)==false);

        //check contact in cv state
        BOOST_CHECK(Fred::object_has_state(contact_id
        ,Fred::ObjectState::VALIDATED_CONTACT));
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestMojeID
