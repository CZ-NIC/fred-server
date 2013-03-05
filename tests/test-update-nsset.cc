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

BOOST_AUTO_TEST_SUITE(TestUtils)

const std::string server_name = "test-update-nsset";

BOOST_AUTO_TEST_CASE(update_nsset)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
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

    Fred::UpdateNsset(test_nsset_handle, registrar_handle).exec(ctx);

    Fred::UpdateNsset(test_nsset_handle//handle
            , registrar_handle//registrar
            , Optional<std::string>()//authinfo
            , std::vector<Fred::DnsHost>() //add_dns
            , std::vector<std::string>() //rem_dns
            , std::vector<std::string>() //add_tech_contact
            , std::vector<std::string>() //rem_tech_contact
            , Optional<short>() //tech_check_level
            , Optional<unsigned long long>() //logd_request_id
            ).exec(ctx);

    Fred::UpdateNsset(test_nsset_handle//handle
            , registrar_handle//registrar
                , Optional<std::string>("passwd")//authinfo
                , Util::vector_of<Fred::DnsHost>
                    (Fred::DnsHost("host",  Util::vector_of<std::string>("127.0.0.1")("127.1.1.1"))) //add_dns
                    (Fred::DnsHost("host1", Util::vector_of<std::string>("127.0.0.2")("127.1.1.2"))) //add_dns
                , Util::vector_of<std::string>("a.ns.nic.cz") //rem_dns
                , Util::vector_of<std::string>(admin_contact3_handle) //std::vector<std::string>() //add_tech_contact
                , Util::vector_of<std::string>(admin_contact3_handle) //std::vector<std::string>() //rem_tech_contact
                , Optional<short>(0) //tech_check_level
                , Optional<unsigned long long>(0) //logd_request_id
                ).exec(ctx);

    Fred::UpdateNsset(test_nsset_handle, registrar_handle)
        .add_dns(Fred::DnsHost("host2",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3")))
        .rem_dns("b.ns.nic.cz")
        .add_tech_contact(admin_contact3_handle)
        .rem_tech_contact(admin_contact3_handle)
        .set_authinfo("passw")
        .set_logd_request_id(0)
        .set_tech_check_level(0)
    .exec(ctx);

    Fred::UpdateNsset(test_nsset_handle, registrar_handle).add_dns(Fred::DnsHost("host3",  Util::vector_of<std::string>("127.0.0.5")("127.1.1.5"))).exec(ctx);
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).rem_dns("host2").exec(ctx);
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).add_tech_contact(admin_contact3_handle).exec(ctx);
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).rem_tech_contact(admin_contact3_handle).exec(ctx);
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).set_authinfo("passw").exec(ctx);
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).set_logd_request_id(0).exec(ctx);
    Fred::UpdateNsset(test_nsset_handle, registrar_handle).set_tech_check_level(0).exec(ctx);

    //ctx.commit_transaction();
}//update_nsset

BOOST_AUTO_TEST_SUITE_END();//TestUpdateNsset
