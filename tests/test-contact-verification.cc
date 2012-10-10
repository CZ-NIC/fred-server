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
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_contactverification_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "mailer_manager.h"
#include "fredlib/contact_verification/contact.h"
#include "fredlib/object_states.h"
#include "contact_verification/contact_verification_impl.h"
#include "checks.h"
#include "contact_verification/contact_verification_checkers.h"

//test-contact-verification.cc

BOOST_AUTO_TEST_SUITE(TestContactVerification)

const std::string server_name = "test-contact-verification";



static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}


BOOST_AUTO_TEST_CASE( test_primary_phone_format_checker )
{
    const std::string CZ_CODE = "+420.";
    const std::string SK_CODE = "+421.";

    /* allowed prefixes */
    {
        std::vector<std::string> allowed_cz = boost::assign::list_of
            ("601")("602")("603")("604")("605")("606")("607")("608")("609")
            ("910")("911")("912")("913")("914")("915")("916")("917")("918")("919");
        /* generate 7xx prefixes */
        for (unsigned int i = 0; i < 10; ++i) {
            for (unsigned int j = 0; j < 10; ++j) {
                allowed_cz.push_back(str(boost::format("7%1%%2%") % i % j));
            }
        }
        for (std::vector<std::string>::iterator it = allowed_cz.begin();
                it != allowed_cz.end(); ++it)
        {
            *it = std::string(CZ_CODE) + (*it);
        }

        std::vector<std::string> allowed_sk = boost::assign::list_of
            ("901")("902")("903")("904")("905")("906")("907")("908")("909")
            ("910")("911")("912")("913")("914")("915")("916")("917")("918")("919")
            ("940")("941")("942")("943")("944")("945")("946")("947")("948")("949")
            ("950")("951")("952")("953")("954")("955")("956")("957")("958")("959");
        for (std::vector<std::string>::iterator it = allowed_sk.begin();
                it != allowed_sk.end(); ++it)
        {
            *it = std::string(SK_CODE) + (*it);
        }

        std::vector<std::string> allowed;
        allowed.insert(allowed.end(), allowed_cz.begin(), allowed_cz.end());
        allowed.insert(allowed.end(), allowed_sk.begin(), allowed_sk.end());

        Fred::Contact::Verification::Contact c;
        for (std::vector<std::string>::const_iterator it = allowed.begin();
                it != allowed.end(); ++it)
        {
            Fred::Contact::Verification::FieldErrorMap errors;
            c.telephone = (*it) + std::string("000000");
            BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
            bool check_result = contact_checker_phone_format(c, errors);
            BOOST_CHECK(check_result == true && errors.empty());
        }
    }

    /* some not allowed */
    {
        std::vector<std::string> not_allowed_cz = boost::assign::list_of
            ("450")("599")("600")("620")("699")("800")("801")
            ("900")("901")("902")("909")("920")("955");
        for (std::vector<std::string>::iterator it = not_allowed_cz.begin();
                it != not_allowed_cz.end(); ++it)
        {
            *it = std::string(CZ_CODE) + (*it);
        }

        std::vector<std::string> not_allowed_sk = boost::assign::list_of
            ("601")("720")("900")("920")("925")("930")("939")("960")("990");
        for (std::vector<std::string>::iterator it = not_allowed_sk.begin();
                it != not_allowed_sk.end(); ++it)
        {
            *it = std::string(SK_CODE) + (*it);
        }

        std::vector<std::string> not_allowed;
        not_allowed.insert(not_allowed.end(), not_allowed_cz.begin(), not_allowed_cz.end());
        not_allowed.insert(not_allowed.end(), not_allowed_sk.begin(), not_allowed_sk.end());


        Fred::Contact::Verification::Contact c;
        for (std::vector<std::string>::const_iterator it = not_allowed.begin();
                it != not_allowed.end(); ++it)
        {
            Fred::Contact::Verification::FieldErrorMap errors;
            c.telephone = (*it) + std::string("000000");
            BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
            bool check_result = contact_checker_phone_format(c, errors);
            BOOST_CHECK(check_result == false
                && errors[Fred::Contact::Verification::field_phone] == Fred::Contact::Verification::INVALID);
        }

    }

    /* too long */
    {
        Fred::Contact::Verification::Contact c;
        Fred::Contact::Verification::FieldErrorMap errors;

        /* CZ_CODE + 6010000000 */
        c.telephone = CZ_CODE + "6010000000";
        BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
        bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(check_result == false
                && errors[Fred::Contact::Verification::field_phone] == Fred::Contact::Verification::INVALID);
    }

    /* too short */
    {
        Fred::Contact::Verification::Contact c;
        Fred::Contact::Verification::FieldErrorMap errors;

        /* CZ_CODE + 60100000 */

        c.telephone = CZ_CODE + "60100000";
        BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
        bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(check_result == false
                && errors[Fred::Contact::Verification::field_phone] == Fred::Contact::Verification::INVALID);
    }

    /* invalid character */
    {
        Fred::Contact::Verification::Contact c;
        Fred::Contact::Verification::FieldErrorMap errors;

        /* CZ_CODE + 60100000a */
        c.telephone = CZ_CODE + "60100000a";
        BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
        bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(check_result == false
                && errors[Fred::Contact::Verification::field_phone] == Fred::Contact::Verification::INVALID);
    }

    /* missing + sign */
    {
        Fred::Contact::Verification::Contact c;
        Fred::Contact::Verification::FieldErrorMap errors;

        std::string phone(CZ_CODE + "601000000");
        c.telephone = phone.substr(1, phone.length());
        BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
        bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(check_result == false
                && errors[Fred::Contact::Verification::field_phone] == Fred::Contact::Verification::INVALID);
    }

    /* not allowed country code */
    {
        Fred::Contact::Verification::Contact c;
        Fred::Contact::Verification::FieldErrorMap errors;

        c.telephone = std::string("+423.601000000");
        BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
        bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(check_result == false
                && errors[Fred::Contact::Verification::field_phone] == Fred::Contact::Verification::INVALID);
    }

    /* empty telephone should pass - there is different check when phone is required */
    {
        Fred::Contact::Verification::Contact c;
        Fred::Contact::Verification::FieldErrorMap errors;

        c.telephone = std::string("");
        BOOST_TEST_MESSAGE("telephone: " << static_cast<std::string>(c.telephone));
        bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(check_result == true && errors.empty());
    }
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


    //check new cci request
    check_public_request_on_contact(fcvc
            , Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION
            , Fred::PublicRequest::PRS_NEW);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check pin2 sms
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

        //check pin1 email
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

    //check answered cci request
    check_public_request_on_contact(fcvc
            , Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION
            , Fred::PublicRequest::PRS_ANSWERED);

    //check new ci request
    check_public_request_on_contact(fcvc
            , Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION
            , Fred::PublicRequest::PRS_NEW);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check pin3 letter
        Database::Result res_ci_letter = conn.exec_params(
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
            , Database::query_param_list(fcvc.handle)
                (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION));
        BOOST_CHECK((res_ci_letter.size() == 1));

        //check conditionally identified contact state
        BOOST_CHECK(Fred::object_has_state(conn.exec_params(
                "select c.id from contact c "
                " join object_registry obr on c.id = obr.id "
                " where obr.name = $1::text "
            ,Database::query_param_list(fcvc.handle))[0][0]
         ,Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));
    }

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get ci request password
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

    //check answered ci request
    check_public_request_on_contact(fcvc
            , Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION
            , Fred::PublicRequest::PRS_ANSWERED);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check identified contact state
        BOOST_CHECK(Fred::object_has_state(
                conn.exec_params(
                "select c.id from contact c "
                " join object_registry obr on c.id = obr.id "
                " where obr.name = $1::text "
            ,Database::query_param_list(fcvc.handle))[0][0]
         ,Fred::ObjectState::IDENTIFIED_CONTACT));
    }

    //check registrar name
    BOOST_CHECK(cv->getRegistrarName(registrar_handle) == "Company A l.t.d");
}

struct Case_contact_verification_in_threads_Fixture
{
    unsigned long long registrar_id;
    Fred::Contact::Verification::Contact fcvc;
    std::string registrar_handle;
    std::string my_new_password;
    std::string my_new_another_request_id;
    unsigned identification_procesed_counter;
    std::string ci_request_password;

    //init
    Case_contact_verification_in_threads_Fixture()
    :registrar_handle("REG-FRED_A")
    , identification_procesed_counter(0)
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

        unsigned long long request_id =0;
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

            registrar_id = res_reg[0][0];

            RandomDataGenerator rdg;

            //create test contact
            std::string xmark = rdg.xnumstring(6);
            fcvc.handle=std::string("TESTCVT-HANDLE")+xmark;
            fcvc.name=std::string("TESTCVT NAME")+xmark;
            fcvc.organization=std::string("TESTCVT-ORG")+xmark;
            fcvc.street1=std::string("TESTCVT-STR1")+xmark;
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
        }
    }

    //destroy
    virtual ~Case_contact_verification_in_threads_Fixture()
    {}
};

//synchronization using barriers
struct sync_barriers
{
    std::size_t thread_number;
    boost::barrier barrier1;
    boost::barrier barrier2;
    boost::mutex guard;

    sync_barriers(std::size_t _thread_number)
        : thread_number(_thread_number)
        , barrier1(thread_number)
        , barrier2(thread_number)
    {}
};

struct ThreadResult
{
    unsigned number;//thread number
    unsigned ret;//return code
    std::string desc;//some closer description
    ThreadResult()
    : number(0)
      , ret(std::numeric_limits<unsigned>::max())
      , desc("empty result")
      {}
};

typedef concurrent_queue<ThreadResult > ThreadResultQueue;

//thread functor
class ContactVerificationTestThreadWorker
{
public:

    ContactVerificationTestThreadWorker(unsigned number,unsigned sleep_time
            , sync_barriers* sb_ptr
            , Case_contact_verification_in_threads_Fixture* fixture_ptr
            , ThreadResultQueue* result_queue_ptr = 0, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , fixture_ptr_(fixture_ptr)
            , rdg_(seed)
            , rsq_ptr (result_queue_ptr)
    {}

    void operator()()
    {
        ThreadResult res;
        res.number = number_;
        res.ret = 0;
        res.desc = std::string("ok");

        try
        {
            boost::shared_ptr<Fred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));
            const std::auto_ptr<Registry::Contact::Verification::ContactVerificationImpl> cv(
                new Registry::Contact::Verification::ContactVerificationImpl(server_name, mm));
            unsigned long long request_id =0;
            std::string another_request_id;

            assert(sb_ptr_ != 0);

            //std::cout << "waiting: " << number_ << std::endl;
            sb_ptr_->barrier1.wait();//wait for other synced threads
            //std::cout << "start: " << number_ << std::endl;

            //call some impl
            cv->createConditionalIdentification(fixture_ptr_->fcvc.handle, fixture_ptr_->registrar_handle
                    , request_id, another_request_id);

            sb_ptr_->barrier2.wait();//wait for other synced threads

            std::string mypassword("testtest");
            int my_public_request_status=0;

            if (number_ == 0)//check in first thread only once
            {
                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                //check new cci request
                check_public_request_on_contact(fixture_ptr_->fcvc
                        , Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION
                        , Fred::PublicRequest::PRS_NEW);

                //check invalidated cci request
                Database::Result res_invalid_cci_request = conn.exec_params(
                    "select pr.status from object_registry obr "
                    " join public_request_objects_map prom on obr.id = prom.object_id "
                    " join public_request_auth pra on prom.request_id = pra.id "
                    " join public_request pr on pr.id=pra.id "
                    " join enum_public_request_status eprs on eprs.id = pr.status "
                    " join enum_public_request_type eprt on pr.request_type = eprt.id "
                    " where obr.name = $1::text and eprt.name = $2::text and eprs.name = 'invalidated'"
                    , Database::query_param_list(fixture_ptr_->fcvc.handle)
                        (Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
                BOOST_CHECK(res_invalid_cci_request.size() == (sb_ptr_->thread_number - 1));

                //check pin2 sms
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

                //check pin1 email
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

            {//run in every thread worker
                boost::mutex::scoped_lock lock(sb_ptr_->guard);

                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                Database::Result res_pass = conn.exec_params(
                        "SELECT pra.password, pr.status FROM public_request_auth pra "
                        " JOIN public_request pr ON pr.id=pra.id "
                        " JOIN enum_public_request_status eprs ON eprs.id = pr.status "
                        " WHERE identification=$1::text"
                        , Database::query_param_list(another_request_id));
                if(res_pass.size() == 1)
                {
                    mypassword = std::string(res_pass[0][0]);
                    my_public_request_status = res_pass[0][1];
                    BOOST_TEST_MESSAGE( "mypassword: " << mypassword
                            << " my_public_request_status: " << my_public_request_status);
                }
                else
                {
                    BOOST_TEST_MESSAGE( "test mypassword not found");
                }
            }

            sb_ptr_->barrier1.wait();//wait for other synced threads

            if(my_public_request_status == Fred::PublicRequest::PRS_NEW)
            {
                fixture_ptr_->my_new_another_request_id = another_request_id;
                fixture_ptr_->my_new_password = mypassword;
            }
            else if(my_public_request_status == Fred::PublicRequest::PRS_INVALID)
            {
            BOOST_CHECK_EXCEPTION(cv->processConditionalIdentification(another_request_id
                    , mypassword, request_id)
                        , std::exception
                        , check_std_exception);
            }
            else
            {
                const std::string errmsg = "my public request is not in expected state new or invalidated";
                BOOST_FAIL(errmsg);
                throw std::runtime_error(errmsg);
            }

            sb_ptr_->barrier2.wait();//wait for other synced threads

            try
            {
                cv->processConditionalIdentification(fixture_ptr_->my_new_another_request_id
                        , fixture_ptr_->my_new_password, request_id);

                //identification processing should happen only once, all other threads should throw exception
                boost::mutex::scoped_lock lock(sb_ptr_->guard);
                fixture_ptr_->identification_procesed_counter+=1;
            }
            catch(std::exception&){}//ignoring std exceptions

            sb_ptr_->barrier1.wait();//wait for other synced threads

            if (number_ == 0)//check in first thread only once
            {
                BOOST_CHECK(fixture_ptr_->identification_procesed_counter==1);

                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                //check cci request answered (1)
                check_public_request_on_contact(fixture_ptr_->fcvc
                        , Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION
                        , Fred::PublicRequest::PRS_ANSWERED);

                //check ci request new 0
                check_public_request_on_contact(fixture_ptr_->fcvc
                        , Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION
                        , Fred::PublicRequest::PRS_NEW);

                //check pin3 letter
                Database::Result res_ci_letter = conn.exec_params(
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
                    , Database::query_param_list(fixture_ptr_->fcvc.handle)
                        (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION));
                BOOST_CHECK((res_ci_letter.size() == 1));

                //check conditionally identified contact state
                BOOST_CHECK(Fred::object_has_state(conn.exec_params(
                        "select c.id from contact c "
                        " join object_registry obr on c.id = obr.id "
                        " where obr.name = $1::text "
                    ,Database::query_param_list(fixture_ptr_->fcvc.handle))[0][0]
                 ,Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));

                //get ci request password
                Database::Result res_ci_request_password = conn.exec_params(
                    "select pra.password from object_registry obr "
                    " join public_request_objects_map prom on obr.id = prom.object_id "
                    " join public_request_auth pra on prom.request_id = pra.id "
                    " join public_request pr on pr.id=pra.id "
                    " join enum_public_request_type eprt on pr.request_type = eprt.id "
                    " where obr.name = $1::text "
                    " and eprt.name = $2::text "//'contact_identification'
                    , Database::query_param_list(fixture_ptr_->fcvc.handle)
                        (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION));

                BOOST_REQUIRE(res_ci_request_password.size() == 1);

                if(res_ci_request_password.size() == 1)
                {
                    fixture_ptr_->ci_request_password = std::string(res_ci_request_password[0][0]);
                }
                else
                {
                  BOOST_TEST_MESSAGE( "test password not found");
                }

                fixture_ptr_->identification_procesed_counter = 0;//init for processIdentification test
            }

            sb_ptr_->barrier2.wait();//wait for other synced threads

            try
            {
                cv->processIdentification(fixture_ptr_->fcvc.handle, fixture_ptr_->ci_request_password, request_id);
                //identification processing should happen only once, all other threads should throw exception
                boost::mutex::scoped_lock lock(sb_ptr_->guard);
                fixture_ptr_->identification_procesed_counter+=1;
            }
            catch(std::exception&){}//ignoring std exceptions

            if (number_ == 0)//check in first thread only once
            {
                BOOST_CHECK(fixture_ptr_->identification_procesed_counter==1);

                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                //check ci request answered (1)
                check_public_request_on_contact(fixture_ptr_->fcvc
                        , Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION
                        , Fred::PublicRequest::PRS_ANSWERED);

                //check identified contact state
                BOOST_CHECK(Fred::object_has_state(
                        conn.exec_params(
                        "select c.id from contact c "
                        " join object_registry obr on c.id = obr.id "
                        " where obr.name = $1::text "
                    ,Database::query_param_list(fixture_ptr_->fcvc.handle))[0][0]
                 ,Fred::ObjectState::IDENTIFIED_CONTACT));
            }
        }
        catch(const std::exception& ex)
        {
            BOOST_TEST_MESSAGE("exception 1 in operator() thread number: " << number_
                    << " reason: " << ex.what() );
            res.ret = 134217728;
            res.desc = std::string(ex.what());
            return;
        }
        catch(...)
        {
            BOOST_TEST_MESSAGE("exception 2 in operator() thread number: " << number_ );
            res.ret = 268435456;
            res.desc = std::string("unknown exception");
            return;
        }

        if(rsq_ptr) rsq_ptr->push(res);
        //std::cout << "end: " << number_ << std::endl;
    }

private:
    //need only defaultly constructible members here
    unsigned    number_;//thred identification
    unsigned    sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    Case_contact_verification_in_threads_Fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr; //result queue non-owning pointer
};//class ContactVerificationTestThreadWorker

BOOST_FIXTURE_TEST_CASE( contact_verification_in_threads, Case_contact_verification_in_threads_Fixture )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<ContactVerificationTestThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    //synchronization barriers instance
    sync_barriers sb(thread_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(ContactVerificationTestThreadWorker(i,3,&sb
            , dynamic_cast<Case_contact_verification_in_threads_Fixture*>(this)
            , &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE( "threads end result_queue.size(): " << result_queue.size() );

    for(unsigned i = 0; i < thread_number; ++i)
    {
        ThreadResult thread_result;
        if(!result_queue.try_pop(thread_result)) {
            continue;
        }

        BOOST_TEST_MESSAGE( "result.ret: " << thread_result.ret );

        if(thread_result.ret != 0)
        {
            BOOST_FAIL( thread_result.desc
                << " thread number: " << thread_result.number
                << " return code: " << thread_result.ret
                << " description: " << thread_result.desc);
        }
    }//for i
}


BOOST_AUTO_TEST_SUITE_END();//TestContactVerification
