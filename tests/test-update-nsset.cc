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
#include "fredlib/nsset/info_nsset.h"
#include "fredlib/nsset/info_nsset_history.h"
#include "fredlib/nsset/info_nsset_compare.h"
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

BOOST_AUTO_TEST_SUITE(TestUpdateNsset)

const std::string server_name = "test-update-nsset";

/**
 * test call OldInfoNsset
*/

BOOST_AUTO_TEST_CASE(info_nsset)
{
    Fred::OperationContext ctx;

    std::string registrar_handle = static_cast<std::string>(
            ctx.get_conn().exec("SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
    BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string admin_contact2_handle = std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark;
    Fred::CreateContact(admin_contact2_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact3_handle = std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark;
    Fred::CreateContact(admin_contact3_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string test_nsset_handle = std::string("TEST-NSSET-HANDLE")+xmark;
    Fred::CreateNsset(test_nsset_handle, registrar_handle)
        .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
            )
            .exec(ctx);

    //ctx.commit_transaction();

    Fred::InfoNssetOut nsset_info1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    Fred::InfoNssetOut nsset_info2 = Fred::OldInfoNsset(test_nsset_handle).set_lock().exec(ctx);
}

struct update_nsset_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string admin_contact3_handle;
    std::string test_nsset_handle;

    update_nsset_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark)
    , admin_contact3_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , test_nsset_handle ( std::string("TEST-NSSET-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(admin_contact3_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle))
                .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~update_nsset_fixture()
    {}
};


/**
 * test UpdateNsset
 * test UpdateNsset construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(update_nsset, update_nsset_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_1 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    //update_registrar_handle check
    BOOST_CHECK(info_data_1.info_nsset_data.update_registrar_handle.isnull());

    //update_time
    BOOST_CHECK(info_data_1.info_nsset_data.update_time.isnull());

    //history check
    BOOST_CHECK(history_info_data_1.at(0) == info_data_1);
    BOOST_CHECK(history_info_data_1.at(0).info_nsset_data.crhistoryid == info_data_1.info_nsset_data.historyid);

    //empty update
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).exec(ctx);

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_2 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_1_with_changes = info_data_1;

    //updated historyid
    BOOST_CHECK(info_data_1.info_nsset_data.historyid !=info_data_2.info_nsset_data.historyid);
    info_data_1_with_changes.info_nsset_data.historyid = info_data_2.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_2.info_nsset_data.update_registrar_handle));
    info_data_1_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_1_with_changes.info_nsset_data.update_time = info_data_2.info_nsset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_1_with_changes == info_data_2);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_2.at(0) == info_data_2);
    BOOST_CHECK(history_info_data_2.at(1) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_2.at(1).info_nsset_data == history_info_data_1.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_2.at(1).next_historyid == history_info_data_2.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_2.at(0).info_nsset_data.crhistoryid == info_data_2.info_nsset_data.crhistoryid);

    Fred::UpdateNsset(test_nsset_handle//handle
            , registrar_handle//registrar
            , Optional<std::string>()//sponsoring registrar
            , Optional<std::string>()//authinfo
            , std::vector<Fred::DnsHost>() //add_dns
            , std::vector<std::string>() //rem_dns
            , std::vector<std::string>() //add_tech_contact
            , std::vector<std::string>() //rem_tech_contact
            , Optional<short>() //tech_check_level
            , Optional<unsigned long long>() //logd_request_id
            ).exec(ctx);

    Fred::InfoNssetOut info_data_3 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_3 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_2_with_changes = info_data_2;

    //updated historyid
    BOOST_CHECK(info_data_2.info_nsset_data.historyid !=info_data_3.info_nsset_data.historyid);
    info_data_2_with_changes.info_nsset_data.historyid = info_data_3.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_3.info_nsset_data.update_registrar_handle));
    info_data_2_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_2_with_changes.info_nsset_data.update_time = info_data_3.info_nsset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_2_with_changes == info_data_3);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_3.at(0) == info_data_3);
    BOOST_CHECK(history_info_data_3.at(1) == info_data_2);
    BOOST_CHECK(history_info_data_3.at(2) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_3.at(1).info_nsset_data == history_info_data_2.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_3.at(1).next_historyid == history_info_data_3.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_3.at(0).info_nsset_data.crhistoryid == info_data_3.info_nsset_data.crhistoryid);

    Fred::UpdateNsset(test_nsset_handle//handle
            , registrar_handle//registrar
                , Optional<std::string>(registrar_handle)//sponsoring registrar
                , Optional<std::string>("passwd")//authinfo
                , Util::vector_of<Fred::DnsHost>
                    (Fred::DnsHost("host",  Util::vector_of<std::string>("127.0.0.1")("127.1.1.1"))) //add_dns
                    (Fred::DnsHost("host1", Util::vector_of<std::string>("127.0.0.2")("127.1.1.2"))) //add_dns
                    (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")))//add_dns
                , Util::vector_of<std::string>("a.ns.nic.cz")("b.ns.nic.cz") //rem_dns
                , Util::vector_of<std::string>(admin_contact2_handle) //std::vector<std::string>() //add_tech_contact
                , Util::vector_of<std::string>(admin_contact2_handle) //std::vector<std::string>() //rem_tech_contact
                , Optional<short>(0) //tech_check_level
                , Optional<unsigned long long>(0) //logd_request_id
                ).exec(ctx);

    Fred::InfoNssetOut info_data_4 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_4 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_3_with_changes = info_data_3;

    //updated historyid
    BOOST_CHECK(info_data_3.info_nsset_data.historyid !=info_data_4.info_nsset_data.historyid);
    info_data_3_with_changes.info_nsset_data.historyid = info_data_4.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_nsset_data.update_registrar_handle));
    info_data_3_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_4.info_nsset_data.sponsoring_registrar_handle));
    info_data_3_with_changes.info_nsset_data.sponsoring_registrar_handle = registrar_handle;


    //updated update_time
    info_data_3_with_changes.info_nsset_data.update_time = info_data_4.info_nsset_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_3.info_nsset_data.authinfopw != info_data_4.info_nsset_data.authinfopw);
    BOOST_CHECK(std::string("passwd") == info_data_4.info_nsset_data.authinfopw);
    info_data_3_with_changes.info_nsset_data.authinfopw = std::string("passwd");

    //update dns_hosts
    info_data_3_with_changes.info_nsset_data.dns_hosts = Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")))
        (Fred::DnsHost("host",  Util::vector_of<std::string>("127.0.0.1")("127.1.1.1")))
        (Fred::DnsHost("host1", Util::vector_of<std::string>("127.0.0.2")("127.1.1.2")));

    //check changes made by last update
    BOOST_CHECK(info_data_3_with_changes == info_data_4);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_4.at(0) == info_data_4);
    BOOST_CHECK(history_info_data_4.at(1) == info_data_3);
    BOOST_CHECK(history_info_data_4.at(2) == info_data_2);
    BOOST_CHECK(history_info_data_4.at(3) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_4.at(1).info_nsset_data == history_info_data_3.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_4.at(1).next_historyid == history_info_data_4.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_4.at(0).info_nsset_data.crhistoryid == info_data_4.info_nsset_data.crhistoryid);

    Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .set_sponsoring_registrar(registrar_handle)
        .add_dns(Fred::DnsHost("host2",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3")))
        .rem_dns("b.ns.nic.cz")
        .add_tech_contact(admin_contact2_handle)
        .rem_tech_contact(admin_contact2_handle)
        .set_authinfo("passw")
        .set_logd_request_id(4)
        .set_tech_check_level(3)
    .exec(ctx);

    Fred::InfoNssetOut info_data_5 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_5 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_4_with_changes = info_data_4;

    //updated historyid
    BOOST_CHECK(info_data_4.info_nsset_data.historyid !=info_data_5.info_nsset_data.historyid);
    info_data_4_with_changes.info_nsset_data.historyid = info_data_5.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_nsset_data.update_registrar_handle));
    info_data_4_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated sponsoring_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_5.info_nsset_data.sponsoring_registrar_handle));
    info_data_4_with_changes.info_nsset_data.sponsoring_registrar_handle = registrar_handle;

    //updated update_time
    info_data_4_with_changes.info_nsset_data.update_time = info_data_5.info_nsset_data.update_time;

    //updated authinfopw
    BOOST_CHECK(info_data_4.info_nsset_data.authinfopw != info_data_5.info_nsset_data.authinfopw);
    BOOST_CHECK(std::string("passw") == info_data_5.info_nsset_data.authinfopw);
    info_data_4_with_changes.info_nsset_data.authinfopw = std::string("passw");

    //update dns_hosts
    info_data_4_with_changes.info_nsset_data.dns_hosts = Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("host2",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3")))
        (Fred::DnsHost("host",  Util::vector_of<std::string>("127.0.0.1")("127.1.1.1")))
        (Fred::DnsHost("host1", Util::vector_of<std::string>("127.0.0.2")("127.1.1.2")));

    //updated tech_check_level
    BOOST_CHECK(3 == info_data_5.info_nsset_data.tech_check_level);
    info_data_4_with_changes.info_nsset_data.tech_check_level = 3;

    //check logd request_id
    BOOST_CHECK(4 == history_info_data_5.at(0).logd_request_id);


    //check changes made by last update
    BOOST_CHECK(info_data_4_with_changes == info_data_5);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_5.at(0) == info_data_5);
    BOOST_CHECK(history_info_data_5.at(1) == info_data_4);
    BOOST_CHECK(history_info_data_5.at(2) == info_data_3);
    BOOST_CHECK(history_info_data_5.at(3) == info_data_2);
    BOOST_CHECK(history_info_data_5.at(4) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_5.at(1).info_nsset_data == history_info_data_4.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_5.at(1).next_historyid == history_info_data_5.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_5.at(0).info_nsset_data.crhistoryid == info_data_5.info_nsset_data.crhistoryid);

    //add dns host
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).add_dns(Fred::DnsHost("host3",  Util::vector_of<std::string>("127.0.0.5")("127.1.1.5"))).exec(ctx);

    Fred::InfoNssetOut info_data_6 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_6 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_5_with_changes = info_data_5;

    //updated historyid
    BOOST_CHECK(info_data_5.info_nsset_data.historyid !=info_data_6.info_nsset_data.historyid);
    info_data_5_with_changes.info_nsset_data.historyid = info_data_6.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_6.info_nsset_data.update_registrar_handle));
    info_data_5_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_5_with_changes.info_nsset_data.update_time = info_data_6.info_nsset_data.update_time;

    //update dns_hosts
    info_data_5_with_changes.info_nsset_data.dns_hosts = Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("host2",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3")))
        (Fred::DnsHost("host",  Util::vector_of<std::string>("127.0.0.1")("127.1.1.1")))
        (Fred::DnsHost("host1", Util::vector_of<std::string>("127.0.0.2")("127.1.1.2")))
        (Fred::DnsHost("host3",  Util::vector_of<std::string>("127.0.0.5")("127.1.1.5")));

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
    BOOST_CHECK(history_info_data_6.at(1).info_nsset_data == history_info_data_5.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_6.at(1).next_historyid == history_info_data_6.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_6.at(0).info_nsset_data.crhistoryid == info_data_6.info_nsset_data.crhistoryid);

    //rem dns host
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).rem_dns("host2").exec(ctx);

    Fred::InfoNssetOut info_data_7 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_7 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_6_with_changes = info_data_6;

    //updated historyid
    BOOST_CHECK(info_data_6.info_nsset_data.historyid !=info_data_7.info_nsset_data.historyid);
    info_data_6_with_changes.info_nsset_data.historyid = info_data_7.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_7.info_nsset_data.update_registrar_handle));
    info_data_6_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_6_with_changes.info_nsset_data.update_time = info_data_7.info_nsset_data.update_time;

    //update dns_hosts
    info_data_6_with_changes.info_nsset_data.dns_hosts = Util::vector_of<Fred::DnsHost>
        (Fred::DnsHost("host",  Util::vector_of<std::string>("127.0.0.1")("127.1.1.1")))
        (Fred::DnsHost("host1", Util::vector_of<std::string>("127.0.0.2")("127.1.1.2")))
        (Fred::DnsHost("host3",  Util::vector_of<std::string>("127.0.0.5")("127.1.1.5")));

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
    BOOST_CHECK(history_info_data_7.at(1).info_nsset_data == history_info_data_6.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_7.at(1).next_historyid == history_info_data_7.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_7.at(0).info_nsset_data.crhistoryid == info_data_7.info_nsset_data.crhistoryid);

    //rem tech contact
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).rem_tech_contact(admin_contact3_handle).exec(ctx);

    Fred::InfoNssetOut info_data_8 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_8 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_7_with_changes = info_data_7;

    //updated historyid
    BOOST_CHECK(info_data_7.info_nsset_data.historyid !=info_data_8.info_nsset_data.historyid);
    info_data_7_with_changes.info_nsset_data.historyid = info_data_8.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_8.info_nsset_data.update_registrar_handle));
    info_data_7_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_7_with_changes.info_nsset_data.update_time = info_data_8.info_nsset_data.update_time;

    //rem tech contact
    info_data_7_with_changes.info_nsset_data.tech_contacts.erase(std::remove(
            info_data_7_with_changes.info_nsset_data.tech_contacts.begin()
            , info_data_7_with_changes.info_nsset_data.tech_contacts.end()
            , admin_contact3_handle));

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
    BOOST_CHECK(history_info_data_8.at(1).info_nsset_data == history_info_data_7.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_8.at(1).next_historyid == history_info_data_8.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_8.at(0).info_nsset_data.crhistoryid == info_data_8.info_nsset_data.crhistoryid);

    //add tech contact
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).add_tech_contact(admin_contact3_handle).exec(ctx);

    Fred::InfoNssetOut info_data_9 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_9 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_8_with_changes = info_data_8;

    //updated historyid
    BOOST_CHECK(info_data_8.info_nsset_data.historyid !=info_data_9.info_nsset_data.historyid);
    info_data_8_with_changes.info_nsset_data.historyid = info_data_9.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_9.info_nsset_data.update_registrar_handle));
    info_data_8_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_8_with_changes.info_nsset_data.update_time = info_data_9.info_nsset_data.update_time;

    //add tech contact
    info_data_8_with_changes.info_nsset_data.tech_contacts.push_back(admin_contact3_handle);

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
    BOOST_CHECK(history_info_data_9.at(1).info_nsset_data == history_info_data_8.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_9.at(1).next_historyid == history_info_data_9.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_9.at(0).info_nsset_data.crhistoryid == info_data_9.info_nsset_data.crhistoryid);

    //set authinfopw
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).set_authinfo("passw").exec(ctx);

    Fred::InfoNssetOut info_data_10 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_10 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_9_with_changes = info_data_9;

    //updated historyid
    BOOST_CHECK(info_data_9.info_nsset_data.historyid !=info_data_10.info_nsset_data.historyid);
    info_data_9_with_changes.info_nsset_data.historyid = info_data_10.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_10.info_nsset_data.update_registrar_handle));
    info_data_9_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_9_with_changes.info_nsset_data.update_time = info_data_10.info_nsset_data.update_time;

    //set authinfopw
    info_data_9_with_changes.info_nsset_data.authinfopw = "passw";

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
    BOOST_CHECK(history_info_data_10.at(1).info_nsset_data == history_info_data_9.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_10.at(1).next_historyid == history_info_data_10.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_10.at(0).info_nsset_data.crhistoryid == info_data_10.info_nsset_data.crhistoryid);

    //set logd request_id
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).set_logd_request_id(1).exec(ctx);

    Fred::InfoNssetOut info_data_11 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_11 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_10_with_changes = info_data_10;

    //updated historyid
    BOOST_CHECK(info_data_10.info_nsset_data.historyid !=info_data_11.info_nsset_data.historyid);
    info_data_10_with_changes.info_nsset_data.historyid = info_data_11.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_11.info_nsset_data.update_registrar_handle));
    info_data_10_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_10_with_changes.info_nsset_data.update_time = info_data_11.info_nsset_data.update_time;

    //check changes made by last update
    BOOST_CHECK(info_data_10_with_changes == info_data_11);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_11.at(0) == info_data_11);
    BOOST_CHECK(history_info_data_11.at(1) == info_data_10);
    BOOST_CHECK(history_info_data_11.at(2) == info_data_9);
    BOOST_CHECK(history_info_data_11.at(3) == info_data_8);
    BOOST_CHECK(history_info_data_11.at(4) == info_data_7);
    BOOST_CHECK(history_info_data_11.at(5) == info_data_6);
    BOOST_CHECK(history_info_data_11.at(6) == info_data_5);
    BOOST_CHECK(history_info_data_11.at(7) == info_data_4);
    BOOST_CHECK(history_info_data_11.at(8) == info_data_3);
    BOOST_CHECK(history_info_data_11.at(9) == info_data_2);
    BOOST_CHECK(history_info_data_11.at(10) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_11.at(1).info_nsset_data == history_info_data_10.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_11.at(1).next_historyid == history_info_data_11.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_11.at(0).info_nsset_data.crhistoryid == info_data_11.info_nsset_data.crhistoryid);

    //check logd request_id
    BOOST_CHECK(1 == history_info_data_11.at(0).logd_request_id);

    //set tech check level
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).set_tech_check_level(2).exec(ctx);

    Fred::InfoNssetOut info_data_12 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    std::vector<Fred::InfoNssetOutput> history_info_data_12 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    Fred::InfoNssetOut info_data_11_with_changes = info_data_11;

    //updated historyid
    BOOST_CHECK(info_data_11.info_nsset_data.historyid !=info_data_12.info_nsset_data.historyid);
    info_data_11_with_changes.info_nsset_data.historyid = info_data_12.info_nsset_data.historyid;

    //updated update_registrar_handle
    BOOST_CHECK(registrar_handle == std::string(info_data_12.info_nsset_data.update_registrar_handle));
    info_data_11_with_changes.info_nsset_data.update_registrar_handle = registrar_handle;

    //updated update_time
    info_data_11_with_changes.info_nsset_data.update_time = info_data_12.info_nsset_data.update_time;

    //updated tech_check_level
    BOOST_CHECK(2 == info_data_12.info_nsset_data.tech_check_level);
    info_data_11_with_changes.info_nsset_data.tech_check_level = 2;


    //check changes made by last update
    BOOST_CHECK(info_data_11_with_changes == info_data_12);

    //check info domain history against info domain
    BOOST_CHECK(history_info_data_12.at(0) == info_data_12);
    BOOST_CHECK(history_info_data_12.at(1) == info_data_11);
    BOOST_CHECK(history_info_data_12.at(2) == info_data_10);
    BOOST_CHECK(history_info_data_12.at(3) == info_data_9);
    BOOST_CHECK(history_info_data_12.at(4) == info_data_8);
    BOOST_CHECK(history_info_data_12.at(5) == info_data_7);
    BOOST_CHECK(history_info_data_12.at(6) == info_data_6);
    BOOST_CHECK(history_info_data_12.at(7) == info_data_5);
    BOOST_CHECK(history_info_data_12.at(8) == info_data_4);
    BOOST_CHECK(history_info_data_12.at(9) == info_data_3);
    BOOST_CHECK(history_info_data_12.at(10) == info_data_2);
    BOOST_CHECK(history_info_data_12.at(11) == info_data_1);

    //check info domain history against last info domain history
    BOOST_CHECK(history_info_data_12.at(1).info_nsset_data == history_info_data_11.at(0).info_nsset_data);

    //check historyid
    BOOST_CHECK(history_info_data_12.at(1).next_historyid == history_info_data_12.at(0).info_nsset_data.historyid);
    BOOST_CHECK(history_info_data_12.at(0).info_nsset_data.crhistoryid == info_data_12.info_nsset_data.crhistoryid);

    ctx.commit_transaction();
}//update_nsset



/**
 * test UpdateNsset with wrong handle
 */

BOOST_FIXTURE_TEST_CASE(update_nsset_wrong_handle, update_nsset_fixture )
{
    std::string bad_test_nsset_handle = std::string("bad")+test_nsset_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(bad_test_nsset_handle, registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_nsset_handle());
        BOOST_CHECK(static_cast<std::string>(ex.get_unknown_nsset_handle()).compare(bad_test_nsset_handle) == 0);
    }
}

/**
 * test UpdateNsset with wrong registrar
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_wrong_registrar, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_registrar_handle = registrar_handle+xmark;
    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        BOOST_CHECK(ex.get_unknown_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());

}

/**
 * test UpdateNsset with wrong sponsoring registrar
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_wrong_sponsoring_registrar, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_registrar_handle = registrar_handle+xmark;
    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
            .set_sponsoring_registrar(bad_registrar_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_sponsoring_registrar_handle());
        BOOST_CHECK(ex.get_unknown_sponsoring_registrar_handle().compare(bad_registrar_handle) == 0);
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());

}


/**
 * test UpdateNsset add non-existing tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_add_wrong_tech_contact, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_tech_contact_handle = admin_contact2_handle+xmark;

    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .add_tech_contact(bad_tech_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unknown_technical_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unknown_technical_contact_handle().at(0).compare(bad_tech_contact_handle) == 0);
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());
}

/**
 * test UpdateNsset add already added tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_add_already_added_tech_contact, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .add_tech_contact(admin_contact3_handle)//already added in fixture
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_already_set_technical_contact_handle());
        BOOST_CHECK(ex.get_vector_of_already_set_technical_contact_handle().at(0).compare(admin_contact3_handle) == 0);
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());
}

/**
 * test UpdateNsset remove non-existing tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_rem_wrong_tech_contact, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_tech_contact_handle = admin_contact2_handle+xmark;

    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .rem_tech_contact(bad_tech_contact_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unknown_technical_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unknown_technical_contact_handle().at(0).compare(bad_tech_contact_handle) == 0);
    }


    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());
}

/**
 * test UpdateNsset remove existing unassigned tech contact
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_rem_unassigned_tech_contact, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_tech_contact_handle = admin_contact2_handle;

    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .rem_tech_contact(bad_tech_contact_handle)
        .exec(ctx);

        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unassigned_technical_contact_handle());
        BOOST_CHECK(ex.get_vector_of_unassigned_technical_contact_handle().at(0).compare(bad_tech_contact_handle) == 0);
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());
}


/**
 * test UpdateNsset add already added dnshost
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_add_already_added_dnshost, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .add_dns(Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3")))//already added in fixture
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_already_set_dns_host());
        BOOST_CHECK(ex.get_vector_of_already_set_dns_host().at(0).compare("a.ns.nic.cz") == 0);
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());
}

/**
 * test UpdateNsset remove unassigned dnshost
 */
BOOST_FIXTURE_TEST_CASE(update_nsset_remove_unassigned_dnshost, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .rem_dns("c.ns.nic.cz")
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::UpdateNsset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_vector_of_unassigned_dns_host());
        BOOST_CHECK(ex.get_vector_of_unassigned_dns_host().at(0).compare("c.ns.nic.cz") == 0);
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    BOOST_CHECK(info_data_2.info_nsset_data.delete_time.isnull());
}


/**
 * test InfoNssetHistory
 * create and update test nsset
 * compare successive states from info nsset with states from info nsset history
 * check initial and next historyid in info nsset history
 * check valid_from and valid_to in info nsset history
 */
BOOST_FIXTURE_TEST_CASE(info_nsset_history_test, update_nsset_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOut info_data_1 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);
    //call update
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .exec(ctx);
        ctx.commit_transaction();
    }

    Fred::InfoNssetOut info_data_2 = Fred::OldInfoNsset(test_nsset_handle).exec(ctx);

    std::vector<Fred::InfoNssetOutput> history_info_data = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx);

    BOOST_CHECK(history_info_data.at(0) == info_data_2);
    BOOST_CHECK(history_info_data.at(1) == info_data_1);

    BOOST_CHECK(history_info_data.at(1).next_historyid == history_info_data.at(0).info_nsset_data.historyid);

    BOOST_CHECK(history_info_data.at(1).history_valid_from < history_info_data.at(1).history_valid_to);
    BOOST_CHECK(history_info_data.at(1).history_valid_to <= history_info_data.at(0).history_valid_from);
    BOOST_CHECK(history_info_data.at(0).history_valid_to.isnull());

    BOOST_CHECK(history_info_data.at(1).info_nsset_data.crhistoryid == history_info_data.at(1).info_nsset_data.historyid);

}

BOOST_AUTO_TEST_SUITE_END();//TestUpdateNsset
