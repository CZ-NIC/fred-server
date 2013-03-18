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

BOOST_AUTO_TEST_SUITE(TestUpdateKeyset)

const std::string server_name = "test-update-keyset";

/**
 * test UpdateKeyset
 * test UpdateKeyset construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(update_keyset)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string admin_contact4_handle = std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark;
    Fred::CreateContact(admin_contact4_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT4 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact5_handle = std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark;
    Fred::CreateContact(admin_contact5_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT5 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact6_handle = std::string("TEST-ADMIN-CONTACT6-HANDLE")+xmark;
    Fred::CreateContact(admin_contact6_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT6 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string test_keyset_handle = std::string("TEST-KEYSET-HANDLE")+xmark;
    Fred::CreateKeyset(test_keyset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact6_handle))
            .exec(ctx);

    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).exec(ctx);

    Fred::UpdateKeyset(test_keyset_handle, registrar_handle)
        .add_dns_key(Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"))
        .exec(ctx);

    Fred::UpdateKeyset(test_keyset_handle//const std::string& handle
                , registrar_handle//const std::string& registrar
                , Optional<std::string>("testauthinfo")//const Optional<std::string>& authinfo
                , Util::vector_of<std::string>(admin_contact5_handle) //const std::vector<std::string>& add_tech_contact
                , Util::vector_of<std::string>(admin_contact6_handle)//const std::vector<std::string>& rem_tech_contact
                , Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "key"))//const std::vector<DnsKey>& add_dns_key
                , Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"))//const std::vector<DnsKey>& rem_dns_key
                , Optional<unsigned long long>(0)//const Optional<unsigned long long> logd_request_id
                ).exec(ctx);

    Fred::UpdateKeyset(test_keyset_handle//const std::string& handle
                , registrar_handle//const std::string& registrar
                , Optional<std::string>()//const Optional<std::string>& authinfo
                , std::vector<std::string>() //const std::vector<std::string>& add_tech_contact
                , std::vector<std::string>()//const std::vector<std::string>& rem_tech_contact
                , std::vector<Fred::DnsKey>()//const std::vector<DnsKey>& add_dns_key
                , std::vector<Fred::DnsKey>()//const std::vector<DnsKey>& rem_dns_key
                , Optional<unsigned long long>()//const Optional<unsigned long long> logd_request_id
                ).exec(ctx);

    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).set_authinfo("kukauthinfo").exec(ctx);
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).add_tech_contact(admin_contact4_handle).exec(ctx);
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).rem_tech_contact(admin_contact5_handle).exec(ctx);
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).add_dns_key(Fred::DnsKey(257, 3, 5, "key2")).add_dns_key(Fred::DnsKey(257, 3, 5, "key3")).exec(ctx);
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).rem_dns_key(Fred::DnsKey(257, 3, 5, "key")).exec(ctx);
    Fred::UpdateKeyset(test_keyset_handle, registrar_handle).set_logd_request_id(0).exec(ctx);

}//update_keyset

BOOST_AUTO_TEST_SUITE_END();//TestUpdateKeyset
