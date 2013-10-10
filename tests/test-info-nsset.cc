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
#include "fredlib/nsset/info_nsset.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/domain/create_domain.h"
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
                .set_lock(false)
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
    Fred::InfoNssetOutput info_data_5 = Fred::HistoryInfoNssetById(info_data_1.info_nsset_data.id).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_5);
    Fred::InfoNssetOutput info_data_6 = Fred::HistoryInfoNssetByHistoryid(info_data_1.info_nsset_data.historyid).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_6);

    //impl
    for( int j = 0; j < (1 << 7); ++j)
    {
        Fred::InfoNsset i;
        if(j & (1 << 0)) i.set_handle(info_data_1.info_nsset_data.handle);
        if(j & (1 << 1)) i.set_roid(info_data_1.info_nsset_data.roid);
        if(j & (1 << 2)) i.set_id(info_data_1.info_nsset_data.id);
        if(j & (1 << 3)) i.set_historyid(info_data_1.info_nsset_data.historyid);
        if(j & (1 << 4)) i.set_lock(true);
        if(j & (1 << 5)) i.set_history_timestamp(info_data_1.info_nsset_data.creation_time);
        if(j & (1 << 6)) i.set_history_query(true);

        std::vector<Fred::InfoNssetOutput> output;
        BOOST_MESSAGE(i.explain_analyze(ctx,output));
        if((j & (1 << 0)) || (j & (1 << 1)) || (j & (1 << 2)) || (j & (1 << 3)))//check if selective
        {
            if((info_data_1 != output.at(0)))
                output.at(0).info_nsset_data.set_diff_print();
            BOOST_CHECK(output.at(0) == info_data_1);
        }
    }


    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END();//TestInfoNsset
