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
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/keyset/info_keyset.h"
#include "fredlib/keyset/info_keyset_history.h"
#include "fredlib/keyset/info_keyset_compare.h"
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

BOOST_AUTO_TEST_SUITE(TestUpdateKeyset)

const std::string server_name = "test-update-keyset";


struct update_keyset_fixture
{
    Fred::OperationContext ctx;
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact4_handle;
    std::string admin_contact5_handle;
    std::string admin_contact6_handle;
    std::string test_keyset_handle;

    update_keyset_fixture()
    :registrar_handle (static_cast<std::string>(ctx.get_conn().exec("SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]))
    , xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact4_handle(std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark)
    , admin_contact5_handle(std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark)
    , admin_contact6_handle(std::string("TEST-ADMIN-CONTACT6-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    {
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact4_handle,registrar_handle)
            .set_name(admin_contact4_handle+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(admin_contact5_handle,registrar_handle)
            .set_name(admin_contact5_handle+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(admin_contact6_handle,registrar_handle)
            .set_name(admin_contact6_handle+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
                .set_dns_keys(Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8")))
                .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~update_keyset_fixture()
    {}
};

/**
 * test call InfoKeyset
*/
BOOST_FIXTURE_TEST_CASE(info_keyset, update_keyset_fixture )
{
    Fred::InfoKeysetOutput keyset_info1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    Fred::InfoKeysetOutput keyset_info2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).set_lock().exec(ctx);

    BOOST_CHECK(keyset_info1 == keyset_info2);
}

/**
 * test UpdateKeyset
 * test UpdateKeyset construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(update_keyset, update_keyset_fixture )
{
    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_1 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    //update_registrar_handle check
    BOOST_CHECK(info_data_1.info_keyset_data.update_registrar_handle.isnull());

    //update_time
    BOOST_CHECK(info_data_1.info_keyset_data.update_time.isnull());

    //history check
    BOOST_CHECK(history_info_data_1.at(0) == info_data_1);
    BOOST_CHECK(history_info_data_1.at(0).info_keyset_data.crhistoryid == info_data_1.info_keyset_data.historyid);

    //empty update
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_2 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_1_with_changes = info_data_1;

    //updated historyid
    BOOST_CHECK(info_data_1.info_keyset_data.historyid !=info_data_2.info_keyset_data.historyid);
    info_data_1_with_changes.info_keyset_data.historyid = info_data_2.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_2.info_keyset_data.update_registrar_handle));
    info_data_1_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_1_with_changes.info_keyset_data.update_time = info_data_2.info_keyset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_1_with_changes == info_data_2);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_2.at(0) == info_data_2);
    BOOST_CHECK(history_info_data_2.at(1) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_2.at(1).info_keyset_data == history_info_data_1.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_2.at(1).next_historyid == history_info_data_2.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_2.at(0).info_keyset_data.crhistoryid == info_data_2.info_keyset_data.crhistoryid);


    Fred::UpdateKeyset(test_keyset_handle//const std::string& handle
        , registrar_handle//const std::string& registrar
        , Optional<std::string>("testauthinfo")//const Optional<std::string>& authinfo
        , Util::vector_of<std::string>(admin_contact5_handle) //const std::vector<std::string>& add_tech_contact
        , Util::vector_of<std::string>(admin_contact6_handle)//const std::vector<std::string>& rem_tech_contact
        , Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "key"))//const std::vector<DnsKey>& add_dns_key
        , Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"))//const std::vector<DnsKey>& rem_dns_key
        , Optional<unsigned long long>(0)//const Optional<unsigned long long> logd_request_id
        ).exec(ctx);

    Fred::InfoKeysetOutput info_data_3 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_3 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_2_with_changes = info_data_2;

    //updated historyid
    BOOST_CHECK(info_data_2.info_keyset_data.historyid !=info_data_3.info_keyset_data.historyid);
    info_data_2_with_changes.info_keyset_data.historyid = info_data_3.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_3.info_keyset_data.update_registrar_handle));
    info_data_2_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_2_with_changes.info_keyset_data.update_time = info_data_3.info_keyset_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_2.info_keyset_data.authinfopw != info_data_3.info_keyset_data.authinfopw);
    BOOST_CHECK(std::string("testauthinfo") == info_data_3.info_keyset_data.authinfopw);
    info_data_2_with_changes.info_keyset_data.authinfopw = std::string("testauthinfo");

    //dnskeys
    info_data_2_with_changes.info_keyset_data.dns_keys = Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "key"));

    //tech contacts
    info_data_2_with_changes.info_keyset_data.tech_contacts = Util::vector_of<std::string>(admin_contact5_handle);

    //check changes made by last update
    BOOST_CHECK(info_data_2_with_changes == info_data_3);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_3.at(0) == info_data_3);
    BOOST_CHECK(history_info_data_3.at(1) == info_data_2);
    BOOST_CHECK(history_info_data_3.at(2) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_3.at(1).info_keyset_data == history_info_data_2.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_3.at(1).next_historyid == history_info_data_3.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_3.at(0).info_keyset_data.crhistoryid == info_data_3.info_keyset_data.crhistoryid);

    Fred::UpdateKeyset(test_keyset_handle//const std::string& handle
        , registrar_handle//const std::string& registrar
        , Optional<std::string>()//const Optional<std::string>& authinfo
        , std::vector<std::string>() //const std::vector<std::string>& add_tech_contact
        , std::vector<std::string>()//const std::vector<std::string>& rem_tech_contact
        , std::vector<Fred::DnsKey>()//const std::vector<DnsKey>& add_dns_key
        , std::vector<Fred::DnsKey>()//const std::vector<DnsKey>& rem_dns_key
        , Optional<unsigned long long>()//const Optional<unsigned long long> logd_request_id
        ).exec(ctx);

    Fred::InfoKeysetOutput info_data_4 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_4 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_3_with_changes = info_data_3;

    //updated historyid
    BOOST_CHECK(info_data_3.info_keyset_data.historyid !=info_data_4.info_keyset_data.historyid);
    info_data_3_with_changes.info_keyset_data.historyid = info_data_4.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_keyset_data.update_registrar_handle));
    info_data_3_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_3_with_changes.info_keyset_data.update_time = info_data_4.info_keyset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_3_with_changes == info_data_4);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_4.at(0) == info_data_4);
    BOOST_CHECK(history_info_data_4.at(1) == info_data_3);
    BOOST_CHECK(history_info_data_4.at(2) == info_data_2);
    BOOST_CHECK(history_info_data_4.at(3) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_4.at(1).info_keyset_data == history_info_data_3.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_4.at(1).next_historyid == history_info_data_4.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_4.at(0).info_keyset_data.crhistoryid == info_data_4.info_keyset_data.crhistoryid);

    //transfer password
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).set_authinfo("kukauthinfo").exec(ctx);

    Fred::InfoKeysetOutput info_data_5 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_5 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_4_with_changes = info_data_4;

    //updated historyid
    BOOST_CHECK(info_data_4.info_keyset_data.historyid !=info_data_5.info_keyset_data.historyid);
    info_data_4_with_changes.info_keyset_data.historyid = info_data_5.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_keyset_data.update_registrar_handle));
    info_data_4_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_4_with_changes.info_keyset_data.update_time = info_data_5.info_keyset_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_4.info_keyset_data.authinfopw != info_data_5.info_keyset_data.authinfopw);
    BOOST_CHECK(std::string("kukauthinfo") == info_data_5.info_keyset_data.authinfopw);
    info_data_4_with_changes.info_keyset_data.authinfopw = std::string("kukauthinfo");

    //check changes made by last update
    BOOST_CHECK(info_data_4_with_changes == info_data_5);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_5.at(0) == info_data_5);
    BOOST_CHECK(history_info_data_5.at(1) == info_data_4);
    BOOST_CHECK(history_info_data_5.at(2) == info_data_3);
    BOOST_CHECK(history_info_data_5.at(3) == info_data_2);
    BOOST_CHECK(history_info_data_5.at(4) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_5.at(1).info_keyset_data == history_info_data_4.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_5.at(1).next_historyid == history_info_data_5.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_5.at(0).info_keyset_data.crhistoryid == info_data_5.info_keyset_data.crhistoryid);

    //add tech contact
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).add_tech_contact(admin_contact4_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_6 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_6 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_5_with_changes = info_data_5;

    //updated historyid
    BOOST_CHECK(info_data_5.info_keyset_data.historyid !=info_data_6.info_keyset_data.historyid);
    info_data_5_with_changes.info_keyset_data.historyid = info_data_6.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_6.info_keyset_data.update_registrar_handle));
    info_data_5_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_5_with_changes.info_keyset_data.update_time = info_data_6.info_keyset_data.update_time;

    //add tech contact
    info_data_5_with_changes.info_keyset_data.tech_contacts.push_back(admin_contact4_handle);

    //check changes made by last update
    BOOST_CHECK(info_data_5_with_changes == info_data_6);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_6.at(0) == info_data_6);
    BOOST_CHECK(history_info_data_6.at(1) == info_data_5);
    BOOST_CHECK(history_info_data_6.at(2) == info_data_4);
    BOOST_CHECK(history_info_data_6.at(3) == info_data_3);
    BOOST_CHECK(history_info_data_6.at(4) == info_data_2);
    BOOST_CHECK(history_info_data_6.at(5) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_6.at(1).info_keyset_data == history_info_data_5.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_6.at(1).next_historyid == history_info_data_6.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_6.at(0).info_keyset_data.crhistoryid == info_data_6.info_keyset_data.crhistoryid);

    //remove tech contact
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).rem_tech_contact(admin_contact5_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_7 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_7 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_6_with_changes = info_data_6;

    //updated historyid
    BOOST_CHECK(info_data_6.info_keyset_data.historyid !=info_data_7.info_keyset_data.historyid);
    info_data_6_with_changes.info_keyset_data.historyid = info_data_7.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_7.info_keyset_data.update_registrar_handle));
    info_data_6_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_6_with_changes.info_keyset_data.update_time = info_data_7.info_keyset_data.update_time;

    //rem tech contact
    info_data_6_with_changes.info_keyset_data.tech_contacts.erase(
        std::remove(info_data_6_with_changes.info_keyset_data.tech_contacts.begin()
        , info_data_6_with_changes.info_keyset_data.tech_contacts.end(), admin_contact5_handle)
        , info_data_6_with_changes.info_keyset_data.tech_contacts.end());

    //check changes made by last update
    BOOST_CHECK(info_data_6_with_changes == info_data_7);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_7.at(0) == info_data_7);
    BOOST_CHECK(history_info_data_7.at(1) == info_data_6);
    BOOST_CHECK(history_info_data_7.at(2) == info_data_5);
    BOOST_CHECK(history_info_data_7.at(3) == info_data_4);
    BOOST_CHECK(history_info_data_7.at(4) == info_data_3);
    BOOST_CHECK(history_info_data_7.at(5) == info_data_2);
    BOOST_CHECK(history_info_data_7.at(6) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_7.at(1).info_keyset_data == history_info_data_6.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_7.at(1).next_historyid == history_info_data_7.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_7.at(0).info_keyset_data.crhistoryid == info_data_7.info_keyset_data.crhistoryid);

    //add dnskey
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).add_dns_key(Fred::DnsKey(257, 3, 5, "key2")).add_dns_key(Fred::DnsKey(257, 3, 5, "key3")).exec(ctx);

    Fred::InfoKeysetOutput info_data_8 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_8 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_7_with_changes = info_data_7;

    //updated historyid
    BOOST_CHECK(info_data_7.info_keyset_data.historyid !=info_data_8.info_keyset_data.historyid);
    info_data_7_with_changes.info_keyset_data.historyid = info_data_8.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_8.info_keyset_data.update_registrar_handle));
    info_data_7_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_7_with_changes.info_keyset_data.update_time = info_data_8.info_keyset_data.update_time;

    //add dnskey
    info_data_7_with_changes.info_keyset_data.dns_keys.push_back(Fred::DnsKey(257, 3, 5, "key2"));
    info_data_7_with_changes.info_keyset_data.dns_keys.push_back(Fred::DnsKey(257, 3, 5, "key3"));

    //check changes made by last update
    BOOST_CHECK(info_data_7_with_changes == info_data_8);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_8.at(0) == info_data_8);
    BOOST_CHECK(history_info_data_8.at(1) == info_data_7);
    BOOST_CHECK(history_info_data_8.at(2) == info_data_6);
    BOOST_CHECK(history_info_data_8.at(3) == info_data_5);
    BOOST_CHECK(history_info_data_8.at(4) == info_data_4);
    BOOST_CHECK(history_info_data_8.at(5) == info_data_3);
    BOOST_CHECK(history_info_data_8.at(6) == info_data_2);
    BOOST_CHECK(history_info_data_8.at(7) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_8.at(1).info_keyset_data == history_info_data_7.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_8.at(1).next_historyid == history_info_data_8.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_8.at(0).info_keyset_data.crhistoryid == info_data_8.info_keyset_data.crhistoryid);

    //remove dnskey
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).rem_dns_key(Fred::DnsKey(257, 3, 5, "key")).exec(ctx);

    Fred::InfoKeysetOutput info_data_9 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_9 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    Fred::InfoKeysetOutput info_data_8_with_changes = info_data_8;

    //updated historyid
    BOOST_CHECK(info_data_8.info_keyset_data.historyid !=info_data_9.info_keyset_data.historyid);
    info_data_8_with_changes.info_keyset_data.historyid = info_data_9.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_9.info_keyset_data.update_registrar_handle));
    info_data_8_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_8_with_changes.info_keyset_data.update_time = info_data_9.info_keyset_data.update_time;

    //rem dnskey
    info_data_8_with_changes.info_keyset_data.dns_keys.erase(
        std::remove(info_data_8_with_changes.info_keyset_data.dns_keys.begin()
        , info_data_8_with_changes.info_keyset_data.dns_keys.end(), Fred::DnsKey(257, 3, 5, "key"))
        , info_data_8_with_changes.info_keyset_data.dns_keys.end());

    //check changes made by last update
    BOOST_CHECK(info_data_8_with_changes == info_data_9);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_9.at(0) == info_data_9);
    BOOST_CHECK(history_info_data_9.at(1) == info_data_8);
    BOOST_CHECK(history_info_data_9.at(2) == info_data_7);
    BOOST_CHECK(history_info_data_9.at(3) == info_data_6);
    BOOST_CHECK(history_info_data_9.at(4) == info_data_5);
    BOOST_CHECK(history_info_data_9.at(5) == info_data_4);
    BOOST_CHECK(history_info_data_9.at(6) == info_data_3);
    BOOST_CHECK(history_info_data_9.at(7) == info_data_2);
    BOOST_CHECK(history_info_data_9.at(8) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_9.at(1).info_keyset_data == history_info_data_8.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_9.at(1).next_historyid == history_info_data_9.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_9.at(0).info_keyset_data.crhistoryid == info_data_9.info_keyset_data.crhistoryid);

    //request_id
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).set_logd_request_id(10).exec(ctx);

    Fred::InfoKeysetOutput info_data_10 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data_10 = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    //request id
    BOOST_CHECK(history_info_data_10.at(0).logd_request_id == 10);

    Fred::InfoKeysetOutput info_data_9_with_changes = info_data_9;

    //updated historyid
    BOOST_CHECK(info_data_9.info_keyset_data.historyid !=info_data_10.info_keyset_data.historyid);
    info_data_9_with_changes.info_keyset_data.historyid = info_data_10.info_keyset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_10.info_keyset_data.update_registrar_handle));
    info_data_9_with_changes.info_keyset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_9_with_changes.info_keyset_data.update_time = info_data_10.info_keyset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_9_with_changes == info_data_10);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_10.at(0) == info_data_10);
    BOOST_CHECK(history_info_data_10.at(1) == info_data_9);
    BOOST_CHECK(history_info_data_10.at(2) == info_data_8);
    BOOST_CHECK(history_info_data_10.at(3) == info_data_7);
    BOOST_CHECK(history_info_data_10.at(4) == info_data_6);
    BOOST_CHECK(history_info_data_10.at(5) == info_data_5);
    BOOST_CHECK(history_info_data_10.at(6) == info_data_4);
    BOOST_CHECK(history_info_data_10.at(7) == info_data_3);
    BOOST_CHECK(history_info_data_10.at(8) == info_data_2);
    BOOST_CHECK(history_info_data_10.at(9) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_10.at(1).info_keyset_data == history_info_data_9.at(0).info_keyset_data);

    //check historyid
    BOOST_CHECK(history_info_data_10.at(1).next_historyid == history_info_data_10.at(0).info_keyset_data.historyid);
    BOOST_CHECK(history_info_data_10.at(0).info_keyset_data.crhistoryid == info_data_10.info_keyset_data.crhistoryid);



}//update_keyset

/**
 * test UpdateKeyset with wrong handle
 */

BOOST_FIXTURE_TEST_CASE(update_keyset_wrong_handle, update_keyset_fixture )
{

    std::string bad_test_keyset_handle = std::string("bad")+test_keyset_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(bad_test_keyset_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_keyset_handle());
        BOOST_CHECK(static_cast<std::string>(ex.get_unknown_keyset_handle()).compare(bad_test_keyset_handle) == 0);
    }
}

/**
 * test UpdateKeyset with wrong registrar
 */
BOOST_FIXTURE_TEST_CASE(update_keyset_wrong_registrar, update_keyset_fixture)
{
    std::string bad_registrar_handle = registrar_handle+xmark;

    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());

}

/**
 * test UpdateKeyset add non-existing tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_keyset_add_wrong_tech_contact, update_keyset_fixture)
{
    std::string bad_tech_contact_handle = admin_contact5_handle+xmark;

    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .add_tech_contact(bad_tech_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_technical_contact_handle());
        BOOST_CHECK(ex.get_unknown_technical_contact_handle().compare(bad_tech_contact_handle) == 0);
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}

/**
 * test UpdateKeyset add already added tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_keyset_add_already_added_tech_contact, update_keyset_fixture)
{
    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .add_tech_contact(admin_contact6_handle)//already added in fixture
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_already_set_technical_contact_handle());
        BOOST_CHECK(ex.get_already_set_technical_contact_handle().compare(admin_contact6_handle) == 0);
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}


/**
 * test UpdateKeyset remove non-existing tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_keyset_rem_wrong_tech_contact, update_keyset_fixture)
{
    std::string bad_tech_contact_handle = admin_contact6_handle+xmark;

    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .rem_tech_contact(bad_tech_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_technical_contact_handle());
        BOOST_CHECK(ex.get_unknown_technical_contact_handle().compare(bad_tech_contact_handle) == 0);
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}

/**
 * test UpdateKeyset remove existing unassigned tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_keyset_rem_unassigned_tech_contact, update_keyset_fixture)
{
    std::string bad_tech_contact_handle = admin_contact4_handle;

    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .rem_tech_contact(bad_tech_contact_handle)
        .exec(ctx);

        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unassigned_technical_contact_handle());
        BOOST_CHECK(ex.get_unassigned_technical_contact_handle().compare(bad_tech_contact_handle) == 0);
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}


/**
 * test UpdateKeyset add already added dnskey
 */
BOOST_FIXTURE_TEST_CASE(update_keyset_add_already_added_dnskey, update_keyset_fixture)
{
    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .add_dns_key(Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_already_set_dns_key());
        BOOST_CHECK(ex.get_already_set_dns_key() == Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"));
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}

/**
 * test UpdateKeyset remove unassigned dnskey
 */
BOOST_FIXTURE_TEST_CASE(update_keyset_unassigned_dnskey, update_keyset_fixture)
{
    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .rem_dns_key(Fred::DnsKey(257, 3, 5, "unassignedkey"))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unassigned_dns_key());
        BOOST_CHECK(ex.get_unassigned_dns_key() == Fred::DnsKey(257, 3, 5, "unassignedkey"));
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_keyset_data.delete_time.isnull());
}


/**
 * test InfoKeysetHistory
 * create and update test keyset
 * compare successive states from info keyset with states from info keyset history
 * check initial and next historyid in info keyset history
 * check valid_from and valid_to in info keyset history
 */
BOOST_FIXTURE_TEST_CASE(info_keyset_history_test, update_keyset_fixture)
{
    Fred::InfoKeysetOutput info_data_1 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);
    //call update
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }

    Fred::InfoKeysetOutput info_data_2 = Fred::InfoKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    std::vector<Fred::InfoKeysetHistoryOutput> history_info_data = Fred::InfoKeysetHistory(info_data_1.info_keyset_data.roid, registrar_handle).exec(ctx);

    BOOST_CHECK(history_info_data.at(0) == info_data_2);
    BOOST_CHECK(history_info_data.at(1) == info_data_1);

    BOOST_CHECK(history_info_data.at(1).next_historyid == history_info_data.at(0).info_keyset_data.historyid);

    BOOST_CHECK(history_info_data.at(1).history_valid_from < history_info_data.at(1).history_valid_to);
    BOOST_CHECK(history_info_data.at(1).history_valid_to <= history_info_data.at(0).history_valid_from);
    BOOST_CHECK(history_info_data.at(0).history_valid_to.isnull());

    BOOST_CHECK(history_info_data.at(1).info_keyset_data.crhistoryid == history_info_data.at(1).info_keyset_data.historyid);
}



BOOST_AUTO_TEST_SUITE_END();//TestUpdateKeyset
