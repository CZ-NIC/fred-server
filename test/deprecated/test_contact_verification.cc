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

#include "src/util/time_clock.hh"
#include "util/random_data_generator.hh"
#include "src/util/concurrent_queue.hh"

#include "src/backend/contact_verification/public_request_contact_verification_impl.hh"

#include "src/util/setup_server_decl.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_contactverification_args.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>
#include <utility>

#include "src/bin/corba/mailer_manager.hh"
#include "src/deprecated/libfred/contact_verification/contact.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "src/backend/contact_verification/contact_verification_impl.hh"
#include "test/deprecated/checks.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_checkers.hh"
#include "util/idn_utils.hh"
//test-contact-verification.cc

BOOST_AUTO_TEST_SUITE(TestContactVerification_old)

const std::string server_name = "test-contact-verification";



static bool check_std_exception(const std::exception& e)
{
    return e.what()[0] != '\0';
}


BOOST_AUTO_TEST_CASE(test_primary_phone_format_checker)
{
    const std::string CZ_CODE = "+420.";
    const std::string SK_CODE = "+421.";

    /* allowed prefixes */
    {
        std::vector<std::string> allowed_cz =
                { "601", "602", "603", "604", "605", "606", "607", "608", "609",
                  "910", "911", "912", "913", "914", "915", "916", "917", "918", "919" };
        /* generate 7xx prefixes */
        for (unsigned int i = 0; i < 10; ++i)
        {
            for (unsigned int j = 0; j < 10; ++j)
            {
                allowed_cz.push_back(str(boost::format("7%1%%2%") % i % j));
            }
        }
        for (std::vector<std::string>::iterator it = allowed_cz.begin(); it != allowed_cz.end(); ++it)
        {
            *it = std::string(CZ_CODE) + (*it);
        }

        std::vector<std::string> allowed_sk =
                { "901", "902", "903", "904", "905", "906", "907", "908", "909",
                  "910", "911", "912", "913", "914", "915", "916", "917", "918", "919",
                  "940", "941", "942", "943", "944", "945", "946", "947", "948", "949",
                  "950", "951", "952", "953", "954", "955", "956", "957", "958", "959" };
        for (std::vector<std::string>::iterator it = allowed_sk.begin(); it != allowed_sk.end(); ++it)
        {
            *it = std::string(SK_CODE) + (*it);
        }

        std::vector<std::string> allowed;
        allowed.insert(allowed.end(), allowed_cz.begin(), allowed_cz.end());
        allowed.insert(allowed.end(), allowed_sk.begin(), allowed_sk.end());

        ::LibFred::Contact::Verification::Contact c;
        for (const auto& prefix : allowed)
        {
            ::LibFred::Contact::Verification::FieldErrorMap errors;
            c.telephone = prefix + "000000";
            BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
            const bool check_result = contact_checker_phone_format(c, errors);
            BOOST_CHECK(check_result && errors.empty());
        }
    }

    /* some not allowed */
    {
        std::vector<std::string> not_allowed_cz =
                { "450", "599", "600", "620", "699", "800", "801",
                  "900", "901", "902", "909", "920", "955" };
        for (std::vector<std::string>::iterator it = not_allowed_cz.begin(); it != not_allowed_cz.end(); ++it)
        {
            *it = std::string(CZ_CODE) + (*it);
        }

        std::vector<std::string> not_allowed_sk = { "601", "720", "900", "920", "925", "930", "939", "960", "990" };
        for (std::vector<std::string>::iterator it = not_allowed_sk.begin(); it != not_allowed_sk.end(); ++it)
        {
            *it = std::string(SK_CODE) + (*it);
        }

        std::vector<std::string> not_allowed;
        not_allowed.insert(not_allowed.end(), not_allowed_cz.begin(), not_allowed_cz.end());
        not_allowed.insert(not_allowed.end(), not_allowed_sk.begin(), not_allowed_sk.end());


        ::LibFred::Contact::Verification::Contact c;
        for (const auto& prefix : not_allowed)
        {
            ::LibFred::Contact::Verification::FieldErrorMap errors;
            c.telephone = prefix + "000000";
            BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
            const bool check_result = contact_checker_phone_cz_sk_format(c, errors);
            BOOST_CHECK(!check_result &&
                        (errors[::LibFred::Contact::Verification::field_phone] == ::LibFred::Contact::Verification::INVALID));
        }

    }

    /* too long */
    {
        ::LibFred::Contact::Verification::Contact c;
        ::LibFred::Contact::Verification::FieldErrorMap errors;

        /* CZ_CODE + 6010000000 */
        c.telephone = CZ_CODE + "6010000000";
        BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
        const bool check_result = contact_checker_phone_cz_sk_format(c, errors);
        BOOST_CHECK(!check_result &&
                    (errors[::LibFred::Contact::Verification::field_phone] == ::LibFred::Contact::Verification::INVALID));
    }

    /* too short */
    {
        ::LibFred::Contact::Verification::Contact c;
        ::LibFred::Contact::Verification::FieldErrorMap errors;

        /* CZ_CODE + 60100000 */

        c.telephone = CZ_CODE + "60100000";
        BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
        const bool check_result = contact_checker_phone_cz_sk_format(c, errors);
        BOOST_CHECK(!check_result &&
                    (errors[::LibFred::Contact::Verification::field_phone] == ::LibFred::Contact::Verification::INVALID));
    }

    /* invalid character */
    {
        ::LibFred::Contact::Verification::Contact c;
        ::LibFred::Contact::Verification::FieldErrorMap errors;

        /* CZ_CODE + 60100000a */
        c.telephone = CZ_CODE + "60100000a";
        BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
        const bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(!check_result &&
                    (errors[::LibFred::Contact::Verification::field_phone] == ::LibFred::Contact::Verification::INVALID));
    }

    /* missing + sign */
    {
        ::LibFred::Contact::Verification::Contact c;
        ::LibFred::Contact::Verification::FieldErrorMap errors;

        std::string phone(CZ_CODE + "601000000");
        c.telephone = phone.substr(1, phone.length());
        BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
        const bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(!check_result &&
                    (errors[::LibFred::Contact::Verification::field_phone] == ::LibFred::Contact::Verification::INVALID));
    }

    /* not allowed country code */
    {
        ::LibFred::Contact::Verification::Contact c;
        ::LibFred::Contact::Verification::FieldErrorMap errors;

        c.telephone = std::string("+423.601000000");
        BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
        const bool check_result = contact_checker_phone_cz_sk_format(c, errors);
        BOOST_CHECK(!check_result &&
                    (errors[::LibFred::Contact::Verification::field_phone] == ::LibFred::Contact::Verification::INVALID));
    }

    /* empty telephone should pass - there is different check when phone is required */
    {
        ::LibFred::Contact::Verification::Contact c;
        ::LibFred::Contact::Verification::FieldErrorMap errors;

        c.telephone = std::string("");
        BOOST_TEST_MESSAGE("telephone: " << c.telephone.get_value());
        const bool check_result = contact_checker_phone_format(c, errors);
        BOOST_CHECK(check_result && errors.empty());
    }
}

/**
testing email checker
*/
BOOST_AUTO_TEST_CASE(test_email_check)
{
    ::LibFred::Contact::Verification::Contact test_contact;
    ::LibFred::Contact::Verification::FieldErrorMap err_map;

    test_contact.email = Nullable<std::string>("");
    BOOST_CHECK(::LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("test@nic.cz");
    BOOST_CHECK(::LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("test@nicž.cz");
    BOOST_CHECK(::LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("test@localhost");
    BOOST_CHECK(!LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("@nic.cz");
    BOOST_CHECK(!LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("test@");
    BOOST_CHECK(!LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("@");
    BOOST_CHECK(!LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    // max length of email in mojeid is 200
    test_contact.email = Nullable<std::string>("a@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.usaaaa");
    BOOST_CHECK(::LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("a@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.usaaaaa");
    BOOST_CHECK(!LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("test1@дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.ве.рф");
    BOOST_CHECK(::LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));

    test_contact.email = Nullable<std::string>("test1@дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.векж.нобёз.дольор.еюж.едквюэ.дёжкэрэ.эю.век.рф");
    BOOST_CHECK(!LibFred::Contact::Verification::contact_checker_email_format(test_contact, err_map));
}


BOOST_AUTO_TEST_CASE(test_contact_verification)
{
    //corba config
    FakedArgs fa = CfgArgs::instance()->fa;
    //conf pointers
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(
            fa.get_argc(),
            fa.get_argv(),
            ns_args_ptr->nameservice_host,
            ns_args_ptr->nameservice_port,
            ns_args_ptr->nameservice_context);

    std::shared_ptr<::LibFred::Mailer::Manager> mm = std::make_shared<MailerManager>(CorbaContainer::get_instance()->getNS());
    const auto cv = std::make_unique<Fred::Backend::ContactVerification::ContactVerificationImpl>(server_name, mm);

    const std::string registrar_handle = "REG-FRED_A";
    ::LibFred::Contact::Verification::Contact fcvc;
    unsigned long long request_id = 0;
    std::string another_request_id;
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //Database::Transaction trans(conn);

        //get registrar id
        const Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if (res_reg.size() == 0)
        {
            throw std::runtime_error("Registrar does not exist");
        }
        const unsigned long long registrar_id = static_cast<unsigned long long>(res_reg[0][0]);

        RandomDataGenerator rdg;

        //create test contact
        const std::string xmark = rdg.xnumstring(6);
        fcvc.handle = "TESTCV-HANDLE" + xmark;
        fcvc.name = "TESTCV NAME" + xmark;
        fcvc.organization = "TESTCV-ORG" + xmark;
        fcvc.street1 = "TESTCV-STR1" + xmark;
        fcvc.city = "Praha";
        fcvc.postalcode = "11150";
        fcvc.country = "CZ";
        fcvc.telephone = "+420.728" + xmark;
        fcvc.email = "test" + xmark + "@nic.cz";
        fcvc.ssn = "1980-01-01";
        fcvc.ssntype = "BIRTHDAY";
        fcvc.auth_info = rdg.xnstring(8);
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

        ::LibFred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);
        //trans.commit();
    }

    cv->createConditionalIdentification(fcvc.handle, registrar_handle, request_id, another_request_id);


    //check new cci request
    check_public_request_on_contact(
            fcvc,
            Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION,
            LibFred::PublicRequest::PRS_OPENED);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check pin2 sms
        const Database::Result res_cci_sms = conn.exec_params(
                "select 0 "
                "from public_request pr "
                "join enum_public_request_type eprt on pr.request_type = eprt.id "
                "join public_request_auth pra on pr.id = pra.id "
                "join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                "join message_archive ma on ma.id = prmm.message_archive_id "
                "join message_type mt on mt.id = ma.message_type_id "
                "where pra.identification = $1::text and eprt.name = $2::text and mt.type='contact_verification_pin2'",
                Database::query_param_list(another_request_id)
                    (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
        BOOST_CHECK_EQUAL(res_cci_sms.size(), 1);

        //check pin1 email
        const Database::Result res_cci_email = conn.exec_params(
                "select 0 "
                "from public_request pr "
                "join enum_public_request_type eprt on pr.request_type = eprt.id "
                "join public_request_auth pra on pr.id = pra.id "
                "join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                "join mail_archive ma on ma.id = prmm.mail_archive_id "
                "join mail_type mt on mt.id = ma.mail_type_id "
                "where pra.identification = $1::text and "
                      "eprt.name = $2::text and "
                      "mt.name = 'conditional_contact_identification'",
                Database::query_param_list(another_request_id)
                    (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
        BOOST_CHECK_EQUAL(res_cci_email.size(), 1);
    }

    BOOST_TEST_MESSAGE("identification: " << another_request_id);

    std::string password = "testtest";
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        const Database::Result res_pass = conn.exec_params(
                "SELECT password FROM public_request_auth  WHERE identification=$1::text",
                Database::query_param_list(another_request_id));
        if (res_pass.size() == 1)
        {
            password = static_cast<std::string>(res_pass[0][0]);
        }
        else
        {
            BOOST_TEST_MESSAGE("test password not found");
        }
    }

    BOOST_TEST_MESSAGE("password: " << password);

    cv->processConditionalIdentification(another_request_id, password, request_id);

    //check resolved cci request
    check_public_request_on_contact(
            fcvc,
            Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION,
            LibFred::PublicRequest::PRS_RESOLVED);

    //check opened ci request
    check_public_request_on_contact(
            fcvc,
            Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION,
            LibFred::PublicRequest::PRS_OPENED);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check pin3 letter
        const Database::Result res_ci_letter = conn.exec_params(
                "select 0 "
                "from object_registry obr "
                "join public_request_objects_map prom on obr.id = prom.object_id "
                "join public_request_auth pra on prom.request_id = pra.id "
                "join public_request pr on pr.id=pra.id "
                "join enum_public_request_type eprt on pr.request_type = eprt.id "
                "join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                "join message_archive ma on ma.id = prmm.message_archive_id "
                "join message_type mt on mt.id = ma.message_type_id "
                "where obr.name = $1::text and eprt.name = $2::text and mt.type='contact_verification_pin3'",
                Database::query_param_list(fcvc.handle)
                    (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION));
        BOOST_CHECK_EQUAL(res_ci_letter.size(), 1);

        //check conditionally identified contact state
        BOOST_CHECK(::LibFred::object_has_state(
                static_cast<int>(conn.exec_params(
                        "select c.id "
                        "from contact c "
                        "join object_registry obr on c.id = obr.id "
                        "where obr.name = $1::text",
                        Database::query_param_list(fcvc.handle))[0][0]),
                ::LibFred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));
    }

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get ci request password
        const Database::Result res_pass = conn.exec_params(
            "select pra.password "
            "from object_registry obr "
            "join public_request_objects_map prom on obr.id = prom.object_id "
            "join public_request_auth pra on prom.request_id = pra.id "
            "join public_request pr on pr.id=pra.id "
            "join enum_public_request_type eprt on pr.request_type = eprt.id "
            "where obr.name = $1::text and "//'TESTCV-HANDLE356681'
                  "eprt.name = $2::text",//'contact_identification'
            Database::query_param_list(fcvc.handle)
                (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION));
        if (res_pass.size() == 1)
        {
            password = static_cast<std::string>(res_pass[0][0]);
        }
        else
        {
          BOOST_TEST_MESSAGE("test password not found");
        }
    }

    BOOST_TEST_MESSAGE("password: " << password);

    cv->processIdentification(fcvc.handle, password, request_id);

    //check resolved ci request
    check_public_request_on_contact(
            fcvc,
            Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION,
            LibFred::PublicRequest::PRS_RESOLVED);

    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //check identified contact state
        BOOST_CHECK(::LibFred::object_has_state(
                static_cast<int>(conn.exec_params(
                        "select c.id "
                        "from contact c "
                        "join object_registry obr on c.id = obr.id "
                        "where obr.name = $1::text",
                        Database::query_param_list(fcvc.handle))[0][0]),
                ::LibFred::ObjectState::IDENTIFIED_CONTACT));
    }

    //check registrar name
    BOOST_CHECK_EQUAL(cv->getRegistrarName(registrar_handle), "Company A l.t.d");
}

struct Case_contact_verification_in_threads_Fixture
{
    unsigned long long registrar_id;
    ::LibFred::Contact::Verification::Contact fcvc;
    std::string registrar_handle;
    std::string my_new_password;
    std::string my_new_another_request_id;
    unsigned identification_procesed_counter;
    std::string ci_request_password;

    //init
    Case_contact_verification_in_threads_Fixture()
        : registrar_handle("REG-FRED_A"),
          identification_procesed_counter(0)
    {
        //corba config
        FakedArgs fa = CfgArgs::instance()->fa;
        //conf pointers
        const HandleCorbaNameServiceArgs* const ns_args_ptr =
                CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(
                fa.get_argc(),
                fa.get_argv(),
                ns_args_ptr->nameservice_host,
                ns_args_ptr->nameservice_port,
                ns_args_ptr->nameservice_context);

        unsigned long long request_id = 0;
        {
            //get db connection
            Database::Connection conn = Database::Manager::acquire();

            //get registrar id
            const Database::Result res_reg = conn.exec_params(
                    "SELECT id FROM registrar WHERE handle=$1::text",
                    Database::query_param_list(registrar_handle));
            if (res_reg.size() == 0)
            {
                throw std::runtime_error("Registrar does not exist");
            }

            registrar_id = static_cast<unsigned long long>(res_reg[0][0]);

            RandomDataGenerator rdg;

            //create test contact
            const std::string xmark = rdg.xnumstring(6);
            fcvc.handle = "TESTCVT-HANDLE" + xmark;
            fcvc.name = "TESTCVT NAME" + xmark;
            fcvc.organization = "TESTCVT-ORG" + xmark;
            fcvc.street1 = "TESTCVT-STR1" + xmark;
            fcvc.city = "Praha";
            fcvc.postalcode = "11150";
            fcvc.country = "CZ";
            fcvc.telephone = "+420.728" + xmark;
            fcvc.email = "test" + xmark + "@nic.cz";
            fcvc.ssn = "1980-01-01";
            fcvc.ssntype = "BIRTHDAY";
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

            ::LibFred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);
        }
    }

    //destroy
    virtual ~Case_contact_verification_in_threads_Fixture()
    {}
};

//synchronization using barriers
struct sync_barriers
{
    std::size_t number_of_threads;
    boost::barrier barrier1;
    boost::barrier barrier2;
    boost::mutex guard;

    sync_barriers(std::size_t _threads)
        : number_of_threads(_threads),
          barrier1(number_of_threads),
          barrier2(number_of_threads)
    {}
};

struct ThreadResult
{
    unsigned number;//thread number
    unsigned ret;//return code
    std::string desc;//some closer description
    ThreadResult()
        : number(0),
          ret(std::numeric_limits<unsigned>::max()),
          desc("empty result")
      {}
};

typedef concurrent_queue<ThreadResult> ThreadResultQueue;

//thread functor
class ContactVerificationTestThreadWorker
{
public:
    ContactVerificationTestThreadWorker(
            unsigned number,
            unsigned sleep_time,
            sync_barriers* sb_ptr,
            Case_contact_verification_in_threads_Fixture* fixture_ptr,
            ThreadResultQueue* result_queue_ptr = nullptr,
            unsigned seed = 0)
        : number_(number),
          sleep_time_(sleep_time),
          sb_ptr_(sb_ptr),
          fixture_ptr_(fixture_ptr),
          rdg_(seed),
          rsq_ptr_(result_queue_ptr)
    {}

    void operator()()
    {
        ThreadResult res;
        res.number = number_;
        res.ret = 0;
        res.desc = "ok";
        int number_of_entered_barriers = 0;

        try
        {
            std::shared_ptr<::LibFred::Mailer::Manager> mm = std::make_shared<MailerManager>(CorbaContainer::get_instance()->getNS());
            const auto cv = std::make_unique<Fred::Backend::ContactVerification::ContactVerificationImpl>(server_name, mm);
            unsigned long long request_id = 0;
            std::string another_request_id;

            assert(sb_ptr_ != nullptr);

            sb_ptr_->barrier1.wait();//wait for other synced threads
            ++number_of_entered_barriers;

            //call some impl
            cv->createConditionalIdentification(
                    fixture_ptr_->fcvc.handle,
                    fixture_ptr_->registrar_handle,
                    request_id,
                    another_request_id);

            sb_ptr_->barrier2.wait();//wait for other synced threads
            ++number_of_entered_barriers;

            std::string mypassword("testtest");
            int my_public_request_status = 0;

            if (number_ == 0)//check in first thread only once
            {
                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                //check new cci request
                check_public_request_on_contact(
                        fixture_ptr_->fcvc,
                        Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION,
                        LibFred::PublicRequest::PRS_OPENED);

                //check invalidated cci request
                const Database::Result res_invalid_cci_request = conn.exec_params(
                    "select 0 "
                    "from object_registry obr "
                    "join public_request_objects_map prom on obr.id = prom.object_id "
                    "join public_request_auth pra on prom.request_id = pra.id "
                    "join public_request pr on pr.id=pra.id "
                    "join enum_public_request_status eprs on eprs.id = pr.status "
                    "join enum_public_request_type eprt on pr.request_type = eprt.id "
                    "where obr.name = $1::text and eprt.name = $2::text and eprs.name = 'invalidated'",
                    Database::query_param_list(fixture_ptr_->fcvc.handle)
                        (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
                BOOST_CHECK_EQUAL(res_invalid_cci_request.size(), (sb_ptr_->number_of_threads - 1));

                //check pin2 sms
                const Database::Result res_cci_sms = conn.exec_params(
                        "select 0 "
                        "from public_request pr "
                        "join enum_public_request_type eprt on pr.request_type = eprt.id "
                        "join public_request_auth pra on pr.id = pra.id "
                        "join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                        "join message_archive ma on ma.id = prmm.message_archive_id "
                        "join message_type mt on mt.id = ma.message_type_id "
                        "where pra.identification = $1::text and "
                              "eprt.name = $2::text and "
                              "mt.type='contact_verification_pin2'",
                        Database::query_param_list(another_request_id)
                            (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
                BOOST_CHECK_EQUAL(res_cci_sms.size(), 1);

                //check pin1 email
                const Database::Result res_cci_email = conn.exec_params(
                        "select 0 "
                        "from public_request pr "
                        "join enum_public_request_type eprt on pr.request_type = eprt.id "
                        "join public_request_auth pra on pr.id = pra.id "
                        "join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                        "join mail_archive ma on ma.id = prmm.mail_archive_id "
                        "join mail_type mt on mt.id = ma.mail_type_id "
                        "where pra.identification = $1::text and "
                              "eprt.name = $2::text and "
                              "mt.name = 'conditional_contact_identification'",
                        Database::query_param_list(another_request_id)
                            (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION));
                BOOST_CHECK_EQUAL(res_cci_email.size(), 1);
            }

            {//run in every thread worker
                boost::mutex::scoped_lock lock(sb_ptr_->guard);

                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                const Database::Result res_pass = conn.exec_params(
                        "SELECT pra.password, pr.status "
                        "FROM public_request_auth pra "
                        "JOIN public_request pr ON pr.id=pra.id "
                        "JOIN enum_public_request_status eprs ON eprs.id = pr.status "
                        "WHERE identification=$1::text",
                        Database::query_param_list(another_request_id));
                if (res_pass.size() == 1)
                {
                    mypassword = static_cast<std::string>(res_pass[0][0]);
                    my_public_request_status = static_cast<int>(res_pass[0][1]);
                    //BOOST_TEST_MESSAGE("mypassword: " << mypassword << " my_public_request_status: " << my_public_request_status);
                }
                else
                {
                    BOOST_TEST_MESSAGE("test mypassword not found");
                }
            }

            sb_ptr_->barrier1.wait();//wait for other synced threads
            ++number_of_entered_barriers;

            if (my_public_request_status == LibFred::PublicRequest::PRS_OPENED)
            {
                fixture_ptr_->my_new_another_request_id = another_request_id;
                fixture_ptr_->my_new_password = mypassword;
            }
            else if (my_public_request_status == LibFred::PublicRequest::PRS_INVALIDATED)
            {
                BOOST_CHECK_EXCEPTION(cv->processConditionalIdentification(another_request_id, mypassword, request_id),
                                      std::exception,
                                      check_std_exception);
            }
            else
            {
                const std::string errmsg = "my public request is not in expected state opened or invalidated";
                BOOST_FAIL(errmsg);
                throw std::runtime_error(errmsg);
            }

            sb_ptr_->barrier2.wait();//wait for other synced threads
            ++number_of_entered_barriers;

            try
            {
                cv->processConditionalIdentification(
                        fixture_ptr_->my_new_another_request_id,
                        fixture_ptr_->my_new_password,
                        request_id);

                //identification processing should happen only once, all other threads should throw exception
                boost::mutex::scoped_lock lock(sb_ptr_->guard);
                fixture_ptr_->identification_procesed_counter += 1;
            }
            catch (const std::exception&) { }//ignoring std exceptions

            sb_ptr_->barrier1.wait();//wait for other synced threads
            ++number_of_entered_barriers;

            if (number_ == 0)//check in first thread only once
            {
                BOOST_CHECK_EQUAL(fixture_ptr_->identification_procesed_counter, 1);

                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                //check cci request resolved (1)
                check_public_request_on_contact(
                        fixture_ptr_->fcvc,
                        Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION,
                        LibFred::PublicRequest::PRS_RESOLVED);

                //check ci request new 0
                check_public_request_on_contact(
                        fixture_ptr_->fcvc,
                        Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION,
                        LibFred::PublicRequest::PRS_OPENED);

                //check pin3 letter
                const Database::Result res_ci_letter = conn.exec_params(
                        "select 0 "
                        "from object_registry obr "
                        "join public_request_objects_map prom on obr.id = prom.object_id "
                        "join public_request_auth pra on prom.request_id = pra.id "
                        "join public_request pr on pr.id=pra.id "
                        "join enum_public_request_type eprt on pr.request_type = eprt.id "
                        "join public_request_messages_map prmm on prmm.public_request_id = pr.id "
                        "join message_archive ma on ma.id = prmm.message_archive_id "
                        "join message_type mt on mt.id = ma.message_type_id "
                        "where obr.name = $1::text and "
                              "eprt.name = $2::text and "
                              "mt.type='contact_verification_pin3'",
                        Database::query_param_list(fixture_ptr_->fcvc.handle)
                            (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION));
                BOOST_CHECK_EQUAL(res_ci_letter.size(), 1);

                //check conditionally identified contact state
                BOOST_CHECK(::LibFred::object_has_state(static_cast<int>(
                        conn.exec_params(
                                "select c.id "
                                "from contact c "
                                "join object_registry obr on c.id = obr.id "
                                "where obr.name = $1::text",
                                Database::query_param_list(fixture_ptr_->fcvc.handle))[0][0]),
                        ::LibFred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT));

                //get ci request password
                const Database::Result res_ci_request_password = conn.exec_params(
                    "select pra.password "
                    "from object_registry obr "
                    "join public_request_objects_map prom on obr.id = prom.object_id "
                    "join public_request_auth pra on prom.request_id = pra.id "
                    "join public_request pr on pr.id=pra.id "
                    "join enum_public_request_type eprt on pr.request_type = eprt.id "
                    "where obr.name = $1::text and "
                          "eprt.name = $2::text",//'contact_identification'
                    Database::query_param_list(fixture_ptr_->fcvc.handle)
                        (Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION));

                BOOST_REQUIRE(res_ci_request_password.size() == 1);

                fixture_ptr_->ci_request_password = static_cast<std::string>(res_ci_request_password[0][0]);
                fixture_ptr_->identification_procesed_counter = 0;//init for processIdentification test
            }

            sb_ptr_->barrier2.wait();//wait for other synced threads
            ++number_of_entered_barriers;

            try
            {
                cv->processIdentification(fixture_ptr_->fcvc.handle, fixture_ptr_->ci_request_password, request_id);
                //identification processing should happen only once, all other threads should throw exception
                boost::mutex::scoped_lock lock(sb_ptr_->guard);
                fixture_ptr_->identification_procesed_counter += 1;
            }
            catch (const std::exception&) { }//ignoring std exceptions

            if (number_ == 0)//check in first thread only once
            {
                BOOST_CHECK_EQUAL(fixture_ptr_->identification_procesed_counter, 1);

                //get db connection
                Database::Connection conn = Database::Manager::acquire();

                //check ci request resolved (1)
                check_public_request_on_contact(
                        fixture_ptr_->fcvc,
                        Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION,
                        LibFred::PublicRequest::PRS_RESOLVED);

                //check identified contact state
                BOOST_CHECK(::LibFred::object_has_state(
                        static_cast<int>(conn.exec_params(
                                "select c.id "
                                "from contact c "
                                "join object_registry obr on c.id = obr.id "
                                "where obr.name = $1::text",
                                Database::query_param_list(fixture_ptr_->fcvc.handle))[0][0]),
                        ::LibFred::ObjectState::IDENTIFIED_CONTACT));
            }
        }
        catch (const std::exception& e)
        {
            //BOOST_TEST_MESSAGE("exception 1 in operator() thread number: " << number_ << " reason: " << e.what());
            res.ret = 134217728;
            res.desc = e.what();
        }
        catch (...)
        {
            //BOOST_TEST_MESSAGE("exception 2 in operator() thread number: " << number_);
            res.ret = 268435456;
            res.desc = "unknown exception";
        }

        while (number_of_entered_barriers < 6)
        {
            if ((number_of_entered_barriers % 2) == 0)
            {
                sb_ptr_->barrier1.wait();//wait for other synced threads
            }
            else
            {
                sb_ptr_->barrier2.wait();//wait for other synced threads
            }
            ++number_of_entered_barriers;
        }
        if (rsq_ptr_ != nullptr)
        {
            rsq_ptr_->guarded_access().push(res);
        }
        //std::cout << "end: " << number_ << std::endl;
    }
private:
    //need only defaultly constructible members here
    unsigned number_;//thred identification
    unsigned sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    Case_contact_verification_in_threads_Fixture* fixture_ptr_;
    RandomDataGenerator rdg_;
    ThreadResultQueue* rsq_ptr_; //result queue non-owning pointer
};

BOOST_FIXTURE_TEST_CASE(contact_verification_in_threads, Case_contact_verification_in_threads_Fixture)
{
    const HandleThreadGroupArgs* const thread_args_ptr =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleThreadGroupArgs>();

    const std::size_t number_of_threads = thread_args_ptr->number_of_threads;
    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<ContactVerificationTestThreadWorker> tw_vector;
    tw_vector.reserve(number_of_threads);

    //synchronization barriers instance
    sync_barriers sb(number_of_threads);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        tw_vector.emplace_back(
                i,
                3,
                &sb,
                dynamic_cast<Case_contact_verification_in_threads_Fixture*>(this),
                &result_queue);
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();//result_queue is accessible without locking

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " << result_queue.unguarded_access().size());

    while (!result_queue.unguarded_access().empty())
    {
        const auto thread_result = result_queue.unguarded_access().pop();

        BOOST_TEST_MESSAGE("result.ret: " << thread_result.ret);

        if (thread_result.ret != 0)
        {
            BOOST_FAIL( thread_result.desc
                << " thread number: " << thread_result.number
                << " return code: " << thread_result.ret
                << " description: " << thread_result.desc);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_contact_create_update_info)
{
    //corba config
    FakedArgs fa = CfgArgs::instance()->fa;
    //conf pointers
    HandleCorbaNameServiceArgs* const ns_args_ptr = CfgArgs::instance()->
        get_handler_ptr_by_type< HandleCorbaNameServiceArgs >();
    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv(),
        ns_args_ptr->nameservice_host,
        ns_args_ptr->nameservice_port,
        ns_args_ptr->nameservice_context);

    std::shared_ptr<::LibFred::Mailer::Manager> mm = std::make_shared<MailerManager>(CorbaContainer::get_instance()->getNS());
    const auto cv = std::make_unique<Fred::Backend::ContactVerification::ContactVerificationImpl>(server_name, mm);

    //get db connection
    Database::Connection conn = Database::Manager::acquire();

    //get registrar id
    static const std::string registrar_handle = "REG-FRED_A";
    const Database::Result res_reg = conn.exec_params(
        "SELECT id FROM registrar WHERE handle=$1::text", Database::query_param_list(registrar_handle));
    if (res_reg.size() == 0)
    {
        throw std::runtime_error("Registrar does not exist");
    }
    const unsigned long long registrar_id = static_cast< unsigned long long >(res_reg[0][0]);

    RandomDataGenerator rdg;

    //create test contact
    ::LibFred::Contact::Verification::Contact fcvc;
    const std::string xmark = rdg.xnumstring(6);
    fcvc.handle = "TESTCCUI-HANDLE" + xmark;
    fcvc.name = "TESTCCUI NAME" + xmark;
    fcvc.organization = "TESTCCUI-ORG" + xmark;
    fcvc.street1 = "TESTCCUI-STR1" + xmark;
    fcvc.city = "Praha";
    fcvc.postalcode = "11150";
    fcvc.country = "CZ";
    fcvc.telephone = "+420.728" + xmark;
    fcvc.email = "test" + xmark + "@nic.cz";
    fcvc.ssn = "1980-01-01";
    fcvc.ssntype = "BIRTHDAY";
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

    ::LibFred::Contact::Verification::ContactAddress mailing_addr;
    mailing_addr.type = "MAILING";
    mailing_addr.street1 = fcvc.street1;
    mailing_addr.city = fcvc.city;
    mailing_addr.postalcode = fcvc.postalcode;
    mailing_addr.country = fcvc.country;

    ::LibFred::Contact::Verification::ContactAddress billing_addr;
    billing_addr.type = "BILLING";
    billing_addr.street1 = fcvc.street1;
    billing_addr.city = fcvc.city;
    billing_addr.postalcode = fcvc.postalcode;
    billing_addr.country = fcvc.country;

    ::LibFred::Contact::Verification::ContactAddress shipping_addr;
    shipping_addr.type = "SHIPPING";
    shipping_addr.company_name = "Doručovací s.r.o.";
    shipping_addr.street1 = fcvc.street1;
    shipping_addr.city = fcvc.city;
    shipping_addr.postalcode = fcvc.postalcode;
    shipping_addr.country = fcvc.country;

    fcvc.addresses.push_back(mailing_addr);
    fcvc.addresses.push_back(billing_addr);
    fcvc.addresses.push_back(shipping_addr);
    BOOST_CHECK_EQUAL(fcvc.addresses.size(), 3);

    enum { DEFAULT_REQUEST_ID = 0 };
    static const unsigned long long request_id = DEFAULT_REQUEST_ID;

    ::LibFred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);

    ::LibFred::Contact::Verification::Contact fcvc_res = ::LibFred::Contact::Verification::contact_info(fcvc.id);
    BOOST_CHECK_EQUAL(fcvc.addresses.size(), fcvc_res.addresses.size());

    typedef std::vector<::LibFred::Contact::Verification::ContactAddress> CAddresses;
    for (CAddresses::const_iterator ptr_src = fcvc.addresses.begin();
         ptr_src != fcvc.addresses.end(); ++ptr_src)
    {
        bool found = false;
        for (CAddresses::const_iterator ptr_dst = fcvc_res.addresses.begin();
            ptr_dst != fcvc_res.addresses.end(); ++ptr_dst)
        {
            if (ptr_src->type == ptr_dst->type)
            {
                BOOST_CHECK(*ptr_src == *ptr_dst);
                found = true;
                break;
            }
        }
        BOOST_CHECK(found);
    }

    for (CAddresses::iterator pa = fcvc.addresses.begin(); pa != fcvc.addresses.end(); ++pa)
    {
        if (pa->type == "BILLING")
        {
            fcvc.addresses.erase(pa);
            break;
        }
    }

    for (CAddresses::iterator pa = fcvc.addresses.begin(); pa != fcvc.addresses.end(); ++pa)
    {
        if (pa->type == "SHIPPING")
        {
            pa->city = Nullable<std::string>();
            pa->postalcode = Nullable<std::string>();
            break;
        }
    }

    ::LibFred::Contact::Verification::contact_update(request_id, registrar_id, fcvc);
    fcvc_res = ::LibFred::Contact::Verification::contact_info(fcvc.id);
    BOOST_CHECK_EQUAL(fcvc.addresses.size(), fcvc_res.addresses.size());

    for (CAddresses::const_iterator ptr_src = fcvc.addresses.begin();
         ptr_src != fcvc.addresses.end(); ++ptr_src)
    {
        bool found = false;
        for (CAddresses::const_iterator ptr_dst = fcvc_res.addresses.begin();
            ptr_dst != fcvc_res.addresses.end(); ++ptr_dst)
        {
            if (ptr_src->type == ptr_dst->type)
            {
                BOOST_CHECK(*ptr_src == *ptr_dst);
                found = true;
                break;
            }
        }
        BOOST_CHECK(found);
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestContactVerification
