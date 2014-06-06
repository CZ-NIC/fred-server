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
#include "src/fredlib/registrar.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/info_nsset_diff.h"
#include "src/fredlib/nsset/info_nsset_impl.h"
#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/opexception.h"
#include "util/util.h"

#include "src/fredlib/nsset/info_nsset_diff.h"

#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/object_states.h"
#include "src/contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "src/fredlib/db_settings.h"

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

BOOST_AUTO_TEST_SUITE(TestInfoNsset)

const std::string server_name = "test-info-domain";


struct info_nsset_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string admin_contact3_handle;
    std::string test_nsset_handle;

    info_nsset_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark)
    , admin_contact3_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , test_nsset_handle ( std::string("TEST-NSSET-HANDLE")+xmark)
    {
        using boost::asio::ip::address;

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
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<address>(address::from_string("127.0.0.3"))(address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<address>(address::from_string("127.0.0.4"))(address::from_string("127.1.1.4")))) //add_dns
                )
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact3_handle))
                .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~info_nsset_fixture()
    {}
};

/**
 * test call InfoNsset
*/
BOOST_FIXTURE_TEST_CASE(info_nsset, info_nsset_fixture )
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoNssetOutput> nsset_res;

    BOOST_MESSAGE(Fred::InfoNsset()
                .set_handle(test_nsset_handle)
                .set_history_query(false)
                .explain_analyze(ctx,nsset_res,"Europe/Prague")
                );

    Fred::InfoNssetOutput info_data_1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    Fred::InfoNssetOutput info_data_2 = Fred::InfoNssetByHandle(test_nsset_handle).set_lock().exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);

    Fred::InfoNssetOutput info_data_3 = Fred::InfoNssetById(info_data_1.info_nsset_data.id).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_3);
    Fred::InfoNssetOutput info_data_4 = Fred::InfoNssetHistory(info_data_1.info_nsset_data.roid).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_4);
    Fred::InfoNssetOutput info_data_5 = Fred::InfoNssetHistoryById(info_data_1.info_nsset_data.id).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_5);
    Fred::InfoNssetOutput info_data_6 = Fred::InfoNssetHistoryByHistoryid(info_data_1.info_nsset_data.historyid).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_6);

    //impl
    for( int j = 0; j < (1 << 7); ++j)
    {
        Fred::InfoNsset i;
        if(j & (1 << 0)) i.set_handle(info_data_1.info_nsset_data.handle);
        if(j & (1 << 1)) i.set_roid(info_data_1.info_nsset_data.roid);
        if(j & (1 << 2)) i.set_id(info_data_1.info_nsset_data.id);
        if(j & (1 << 3)) i.set_historyid(info_data_1.info_nsset_data.historyid);
        if(j & (1 << 4)) i.set_lock();
        if(j & (1 << 5)) i.set_history_timestamp(info_data_1.info_nsset_data.creation_time);
        if(j & (1 << 6)) i.set_history_query(true);

        std::vector<Fred::InfoNssetOutput> output;
        BOOST_MESSAGE(i.explain_analyze(ctx,output));
        if((j & (1 << 0)) || (j & (1 << 1)) || (j & (1 << 2)) || (j & (1 << 3)))//check if selective
        {
            if((info_data_1 != output.at(0)))
            {
                BOOST_MESSAGE(Fred::diff_nsset_data(info_data_1.info_nsset_data
                        , output.at(0).info_nsset_data).to_string());
            }
            BOOST_CHECK(output.at(0) == info_data_1);
        }
    }


    ctx.commit_transaction();
}

/**
 * test call InfoNssetDiff
*/
BOOST_FIXTURE_TEST_CASE(info_nsset_diff, info_nsset_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoNssetOutput nsset_info1 = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx);
    Fred::InfoNssetOutput nsset_info2 = Fred::InfoNssetByHandle(test_nsset_handle).set_lock().exec(ctx);

    Fred::InfoNssetDiff test_diff, test_empty_diff;

    //differing data
    test_diff.crhistoryid = std::make_pair(1ull,2ull);
    test_diff.historyid = std::make_pair(1ull,2ull);
    test_diff.id = std::make_pair(1ull,2ull);
    test_diff.delete_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.handle = std::make_pair(std::string("test1"),std::string("test2"));
    test_diff.roid = std::make_pair(std::string("testroid1"),std::string("testroid2"));
    test_diff.sponsoring_registrar_handle = std::make_pair(std::string("testspreg1"),std::string("testspreg2"));
    test_diff.create_registrar_handle = std::make_pair(std::string("testcrreg1"),std::string("testcrreg2"));
    test_diff.update_registrar_handle = std::make_pair(Nullable<std::string>("testcrreg1"),Nullable<std::string>());
    test_diff.creation_time = std::make_pair(boost::posix_time::ptime(),boost::posix_time::second_clock::local_time());
    test_diff.update_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.transfer_time = std::make_pair(Nullable<boost::posix_time::ptime>()
                ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.authinfopw = std::make_pair(std::string("testpass1"),std::string("testpass2"));

    BOOST_MESSAGE(test_diff.to_string());
    BOOST_MESSAGE(test_empty_diff.to_string());

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_MESSAGE(Fred::diff_nsset_data(nsset_info1.info_nsset_data,nsset_info2.info_nsset_data).to_string());

    //because of changes to Nullable::operator<<
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Database::QPNull))[0][0].isnull());
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Nullable<std::string>()))[0][0].isnull());
}


BOOST_AUTO_TEST_SUITE_END();//TestInfoNsset
