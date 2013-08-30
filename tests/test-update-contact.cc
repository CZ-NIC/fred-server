/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/contact/update_contact.h"
#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/info_contact_history.h"
#include "fredlib/contact/info_contact_compare.h"
#include "fredlib/opexception.h"
#include "util/util.h"

#include "fredlib/contact_verification/contact.h"
#include "fredlib/object_states.h"
#include "contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestUpdateContact)

const std::string server_name = "test-update-contact";

struct update_contact_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;

    update_contact_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(test_contact_handle,registrar_handle)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~update_contact_fixture()
    {}
};


/**
 * test UpdateContact
 * test UpdateContact construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(update_contact, update_contact_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput info_data_1 = Fred::InfoContact(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactHistoryOutput> history_info_data_1 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid, registrar_handle).exec(ctx);

    //update_registrar_handle check
    BOOST_CHECK(info_data_1.info_contact_data.update_registrar_handle.isnull());

    //update_time
    BOOST_CHECK(info_data_1.info_contact_data.update_time.isnull());

    //history check
    BOOST_CHECK(history_info_data_1.at(0) == info_data_1);

    BOOST_MESSAGE(std::string("history_info_data_1.at(0).info_contact_data.crhistoryid: ") + boost::lexical_cast<std::string>(history_info_data_1.at(0).info_contact_data.crhistoryid ));
    BOOST_MESSAGE(std::string("info_data_1.info_contact_data.historyid: ") + boost::lexical_cast<std::string>(info_data_1.info_contact_data.historyid ));

    BOOST_CHECK(history_info_data_1.at(0).info_contact_data.crhistoryid == info_data_1.info_contact_data.historyid);

    //empty update
    Fred::UpdateContact(test_contact_handle, registrar_handle).exec(ctx);

    Fred::InfoContactOutput info_data_2 = Fred::InfoContact(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactHistoryOutput> history_info_data_2 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid, registrar_handle).exec(ctx);

    Fred::InfoContactOutput info_data_1_with_changes = info_data_1;

    //updated historyid
    BOOST_CHECK(info_data_1.info_contact_data.historyid !=info_data_2.info_contact_data.historyid);
    info_data_1_with_changes.info_contact_data.historyid = info_data_2.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_2.info_contact_data.update_registrar_handle));
    info_data_1_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_1_with_changes.info_contact_data.update_time = info_data_2.info_contact_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_1_with_changes == info_data_2);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_2.at(0) == info_data_2);
    BOOST_CHECK(history_info_data_2.at(1) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_2.at(1).info_contact_data == history_info_data_1.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_2.at(1).next_historyid == history_info_data_2.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_2.at(0).info_contact_data.crhistoryid == info_data_2.info_contact_data.crhistoryid);

    Fred::UpdateContact(test_contact_handle//handle
            , registrar_handle//registrar
            , Optional<std::string>()//sponsoring registrar
            , Optional<std::string>()//authinfo
            , Optional<std::string>()//name
            , Optional<std::string>()//organization
            , Optional<std::string>()//street1
            , Optional<std::string>()//street2
            , Optional<std::string>()//street3
            , Optional<std::string>()//city
            , Optional<std::string>()//stateorprovince
            , Optional<std::string>()//postalcode
            , Optional<std::string>()//country
            , Optional<std::string>()//telephone
            , Optional<std::string>()//fax
            , Optional<std::string>()//email
            , Optional<std::string>()//notifyemail
            , Optional<std::string>()//vat
            , Optional<std::string>()//ssntype
            , Optional<std::string>()//ssn
            , Optional<bool>()//disclosename
            , Optional<bool>()//discloseorganization
            , Optional<bool>()//discloseaddress
            , Optional<bool>()//disclosetelephone
            , Optional<bool>()//disclosefax
            , Optional<bool>()//discloseemail
            , Optional<bool>()//disclosevat
            , Optional<bool>()//discloseident
            , Optional<bool>()//disclosenotifyemail
            , Optional<unsigned long long>() //logd_request_id
            ).exec(ctx);

    Fred::InfoContactOutput info_data_3 = Fred::InfoContact(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactHistoryOutput> history_info_data_3 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid, registrar_handle).exec(ctx);

    Fred::InfoContactOutput info_data_2_with_changes = info_data_2;

    //updated historyid
    BOOST_CHECK(info_data_2.info_contact_data.historyid !=info_data_3.info_contact_data.historyid);
    info_data_2_with_changes.info_contact_data.historyid = info_data_3.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_3.info_contact_data.update_registrar_handle));
    info_data_2_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_2_with_changes.info_contact_data.update_time = info_data_3.info_contact_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_2_with_changes == info_data_3);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_3.at(0) == info_data_3);
    BOOST_CHECK(history_info_data_3.at(1) == info_data_2);
    BOOST_CHECK(history_info_data_3.at(2) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_3.at(1).info_contact_data == history_info_data_2.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_3.at(1).next_historyid == history_info_data_3.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_3.at(0).info_contact_data.crhistoryid == info_data_3.info_contact_data.crhistoryid);

    Fred::UpdateContact(test_contact_handle//handle
            , registrar_handle//registrar
                , Optional<std::string>(registrar_handle)//sponsoring registrar
                , Optional<std::string>("passwd")//authinfo
                , Optional<std::string>("Test Name")//name
                , Optional<std::string>("Test o.r.g.")//organization
                , Optional<std::string>("Str 1")//street1
                , Optional<std::string>("str2")//street2
                , Optional<std::string>()//street3
                , Optional<std::string>("Prague")//city
                , Optional<std::string>()//stateorprovince
                , Optional<std::string>("11150")//postalcode
                , Optional<std::string>("Czech Republic")//country
                , Optional<std::string>("+420.123456789")//telephone
                , Optional<std::string>()//fax
                , Optional<std::string>("test@nic.cz")//email
                , Optional<std::string>("notif-test@nic.cz")//notifyemail
                , Optional<std::string>("7805962556")//vat is TID
                , Optional<std::string>("ICO")//ssntype
                , Optional<std::string>("7805962556")//ssn
                , Optional<bool>(true)//disclosename
                , Optional<bool>(true)//discloseorganization
                , Optional<bool>(true)//discloseaddress
                , Optional<bool>(true)//disclosetelephone
                , Optional<bool>(false)//disclosefax
                , Optional<bool>(true)//discloseemail
                , Optional<bool>(true)//disclosevat
                , Optional<bool>(true)//discloseident
                , Optional<bool>(false)//disclosenotifyemail
                , Optional<unsigned long long>(0) //logd_request_id
                ).exec(ctx);

    Fred::InfoContactOutput info_data_4 = Fred::InfoContact(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactHistoryOutput> history_info_data_4 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid, registrar_handle).exec(ctx);

    Fred::InfoContactOutput info_data_3_with_changes = info_data_3;

    //updated historyid
    BOOST_CHECK(info_data_3.info_contact_data.historyid !=info_data_4.info_contact_data.historyid);
    info_data_3_with_changes.info_contact_data.historyid = info_data_4.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_contact_data.update_registrar_handle));
    info_data_3_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_contact_data.sponsoring_registrar_handle));
    info_data_3_with_changes.info_contact_data.sponsoring_registrar_handle = registrar_handle;

    //updated update_time
    info_data_3_with_changes.info_contact_data.update_time = info_data_4.info_contact_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_3.info_contact_data.authinfopw != info_data_4.info_contact_data.authinfopw);
    BOOST_CHECK(std::string("passwd") == info_data_4.info_contact_data.authinfopw);
    info_data_3_with_changes.info_contact_data.authinfopw = std::string("passwd");
    info_data_3_with_changes.info_contact_data.name = std::string("Test Name");
    info_data_3_with_changes.info_contact_data.organization = std::string("Test o.r.g.");
    info_data_3_with_changes.info_contact_data.street1 = std::string("Str 1");
    info_data_3_with_changes.info_contact_data.street2 = std::string("str2");
    info_data_3_with_changes.info_contact_data.city = std::string("Prague");
    info_data_3_with_changes.info_contact_data.postalcode = std::string("11150");
    info_data_3_with_changes.info_contact_data.country = std::string("CZ");
    info_data_3_with_changes.info_contact_data.telephone = std::string("+420.123456789");
    info_data_3_with_changes.info_contact_data.email = std::string("test@nic.cz");
    info_data_3_with_changes.info_contact_data.notifyemail = std::string("notif-test@nic.cz");
    info_data_3_with_changes.info_contact_data.vat = std::string("7805962556");
    info_data_3_with_changes.info_contact_data.ssntype = std::string("ICO");
    info_data_3_with_changes.info_contact_data.ssn = std::string("7805962556");

    info_data_3_with_changes.info_contact_data.disclosename = true;
    info_data_3_with_changes.info_contact_data.discloseorganization = true;
    info_data_3_with_changes.info_contact_data.discloseaddress = true;
    info_data_3_with_changes.info_contact_data.disclosetelephone = true;
    info_data_3_with_changes.info_contact_data.disclosefax = false;
    info_data_3_with_changes.info_contact_data.discloseemail = true;
    info_data_3_with_changes.info_contact_data.disclosevat = true;
    info_data_3_with_changes.info_contact_data.discloseident = true;
    info_data_3_with_changes.info_contact_data.disclosenotifyemail = false;

    //check changes made by last update
    BOOST_CHECK(info_data_3_with_changes == info_data_4);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_4.at(0) == info_data_4);
    BOOST_CHECK(history_info_data_4.at(1) == info_data_3);
    BOOST_CHECK(history_info_data_4.at(2) == info_data_2);
    BOOST_CHECK(history_info_data_4.at(3) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_4.at(1).info_contact_data == history_info_data_3.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_4.at(1).next_historyid == history_info_data_4.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_4.at(0).info_contact_data.crhistoryid == info_data_4.info_contact_data.crhistoryid);

    Fred::UpdateContact(test_contact_handle, registrar_handle)
    .set_sponsoring_registrar(registrar_handle)
    .set_authinfo("passw")
    .set_name("Test Name")
    .set_organization("Test o.r.g.")
    .set_street1("Str 1")
    .set_street2("str2")
    .set_street3("")
    .set_city("Prague")
    .set_postalcode("11150")
    .set_telephone("+420.123456789")
    .set_fax("")
    .set_email("test@nic.cz")
    .set_notifyemail("notif-test@nic.cz")
    .set_vat("7805962556")
    .set_ssntype("ICO")
    .set_ssn("7805962556")
    .set_disclosename(true)
    .set_discloseorganization(true)
    .set_discloseaddress(true)
    .set_disclosetelephone(true)
    .set_disclosefax(false)
    .set_discloseemail(true)
    .set_disclosevat(true)
    .set_discloseident(true)
    .set_disclosenotifyemail(false)
    .set_logd_request_id(4)
    .exec(ctx);

    Fred::InfoContactOutput info_data_5 = Fred::InfoContact(test_contact_handle).exec(ctx);
    std::vector<Fred::InfoContactHistoryOutput> history_info_data_5 = Fred::InfoContactHistory(info_data_1.info_contact_data.roid, registrar_handle).exec(ctx);

    Fred::InfoContactOutput info_data_4_with_changes = info_data_4;

    //updated historyid
    BOOST_CHECK(info_data_4.info_contact_data.historyid !=info_data_5.info_contact_data.historyid);
    info_data_4_with_changes.info_contact_data.historyid = info_data_5.info_contact_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_contact_data.update_registrar_handle));
    info_data_4_with_changes.info_contact_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_contact_data.sponsoring_registrar_handle));
    info_data_4_with_changes.info_contact_data.sponsoring_registrar_handle = registrar_handle;

    //updated update_time
    info_data_4_with_changes.info_contact_data.update_time = info_data_5.info_contact_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_4.info_contact_data.authinfopw != info_data_5.info_contact_data.authinfopw);
    BOOST_CHECK(std::string("passw") == info_data_5.info_contact_data.authinfopw);
    info_data_4_with_changes.info_contact_data.authinfopw = std::string("passw");

    //empty string in street3 and fax
    info_data_4_with_changes.info_contact_data.street3 = std::string("");
    info_data_4_with_changes.info_contact_data.fax = std::string("");

    //check logd request_id
    BOOST_CHECK(4 == history_info_data_5.at(0).logd_request_id);

    //check changes made by last update
    BOOST_CHECK(info_data_4_with_changes == info_data_5);

    //check info contact history against info contact
    BOOST_CHECK(history_info_data_5.at(0) == info_data_5);
    BOOST_CHECK(history_info_data_5.at(1) == info_data_4);
    BOOST_CHECK(history_info_data_5.at(2) == info_data_3);
    BOOST_CHECK(history_info_data_5.at(3) == info_data_2);
    BOOST_CHECK(history_info_data_5.at(4) == info_data_1);

    //check info contact history against last info contact history
    BOOST_CHECK(history_info_data_5.at(1).info_contact_data == history_info_data_4.at(0).info_contact_data);

    //check historyid
    BOOST_CHECK(history_info_data_5.at(1).next_historyid == history_info_data_5.at(0).info_contact_data.historyid);
    BOOST_CHECK(history_info_data_5.at(0).info_contact_data.crhistoryid == info_data_5.info_contact_data.crhistoryid);

    ctx.commit_transaction();
}//update_contact

/**
 * test UpdateContact with wrong handle
 */

BOOST_FIXTURE_TEST_CASE(update_contact_wrong_handle, update_contact_fixture )
{
    std::string bad_test_contact_handle = std::string("bad")+test_contact_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContact(bad_test_contact_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_handle());
        BOOST_CHECK(static_cast<std::string>(ex.get_unknown_contact_handle()).compare(bad_test_contact_handle) == 0);
    }
}

/**
 * test UpdateContact with wrong registrar
 */
BOOST_FIXTURE_TEST_CASE(update_contact_wrong_registrar, update_contact_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_registrar_handle = registrar_handle+xmark;
    Fred::InfoContactOutput info_data_1 = Fred::InfoContact(test_contact_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContact(test_contact_handle, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoContactOutput info_data_2 = Fred::InfoContact(test_contact_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContact with wrong sponsoring registrar
 */
BOOST_FIXTURE_TEST_CASE(update_contact_wrong_sponsoring_registrar, update_contact_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_registrar_handle = registrar_handle+xmark;
    Fred::InfoContactOutput info_data_1 = Fred::InfoContact(test_contact_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContact(test_contact_handle, registrar_handle)
            .set_sponsoring_registrar(bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_sponsoring_registrar_handle());
        BOOST_CHECK(ex.get_unknown_sponsoring_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoContactOutput info_data_2 = Fred::InfoContact(test_contact_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}


/**
 * test UpdateContact with wrong ssntype
 */
BOOST_FIXTURE_TEST_CASE(update_contact_wrong_ssntype, update_contact_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput info_data_1 = Fred::InfoContact(test_contact_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContact(test_contact_handle, registrar_handle)
        .set_ssntype("bad-ssntype")
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_ssntype());
        BOOST_CHECK(ex.get_unknown_ssntype().compare("bad-ssntype") == 0);
    }

    Fred::InfoContactOutput info_data_2 = Fred::InfoContact(test_contact_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}

/**
 * test UpdateContact with wrong country
 */
BOOST_FIXTURE_TEST_CASE(update_contact_wrong_country, update_contact_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput info_data_1 = Fred::InfoContact(test_contact_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateContact(test_contact_handle, registrar_handle)
        .set_country("bad-country")
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_country());
        BOOST_CHECK(ex.get_unknown_country().compare("bad-country") == 0);
    }

    Fred::InfoContactOutput info_data_2 = Fred::InfoContact(test_contact_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_contact_data.delete_time.isnull());
}


BOOST_AUTO_TEST_SUITE_END();//TestUpdateContact
