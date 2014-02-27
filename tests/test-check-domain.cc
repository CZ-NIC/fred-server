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
#include <boost/regex.hpp>


//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/delete_domain.h"
#include "src/fredlib/domain/info_domain.h"

#include "util/util.h"

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

BOOST_AUTO_TEST_SUITE(TestCheckDomain)

const std::string server_name = "test-check-domain";

struct check_domain_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact_handle;
    std::string registrant_contact_handle;
    std::string test_nsset_handle;
    std::string test_keyset_handle;
    std::string test_domain_name;
    std::string test_domain_name_rem;
    std::string blacklisted_domain_name;

    check_domain_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact_handle(std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark)
    , test_nsset_handle ( std::string("TEST-NSSET-")+xmark+"-HANDLE")
    , test_keyset_handle ( std::string("TEST-KEYSET-")+xmark+"-HANDLE")
    , test_domain_name ( std::string("testfred")+xmark+".cz")
    , test_domain_name_rem ( std::string("testremfred")+xmark+".cz")
    , blacklisted_domain_name("fredblack"+xmark+".cz")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
                ).exec(ctx);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        Fred::CreateDomain(test_domain_name, registrar_handle, registrant_contact_handle)
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_keyset(test_keyset_handle).set_nsset(test_nsset_handle).exec(ctx);

        Fred::CreateDomain(test_domain_name_rem, registrar_handle, registrant_contact_handle)
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_keyset(test_keyset_handle).set_nsset(test_nsset_handle).exec(ctx);

        Fred::DeleteDomainByHandle(test_domain_name_rem).exec(ctx);

        //blacklist
        ctx.get_conn().exec_params("INSERT into domain_blacklist (regexp,valid_from,valid_to,reason) "
            " VALUES ($1::text,current_timestamp,current_timestamp + interval '1 month','test' )"
            , Database::query_param_list(std::string("^")+blacklisted_domain_name+"$"));

        ctx.commit_transaction();
    }
    ~check_domain_fixture()
    {}
};

/**
 * test CheckDomain true returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_contact_handle_true, check_domain_fixture)
{
    Fred::OperationContext ctx;
    BOOST_CHECK(Fred::CheckDomain(std::string("-")+test_domain_name).is_invalid_handle(ctx));
    BOOST_CHECK(Fred::CheckDomain(std::string("testfred")+xmark+".czz").is_bad_zone(ctx));
    BOOST_CHECK(Fred::CheckDomain(std::string("testfred")+xmark+".czz.cz").is_bad_length(ctx));
    BOOST_CHECK(Fred::CheckDomain(blacklisted_domain_name).is_blacklisted(ctx));
    std::string conflicting_handle;
    BOOST_CHECK(Fred::CheckDomain(test_domain_name).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(test_domain_name.compare(conflicting_handle) == 0);
    BOOST_CHECK(Fred::CheckDomain(test_domain_name).is_registered(ctx));
    BOOST_CHECK(Fred::CheckDomain(std::string("testfred")+xmark+"available.cz").is_available(ctx));
}

/**
 * test CheckDomain false returning cases
 */

BOOST_FIXTURE_TEST_CASE(check_contact_handle_false, check_domain_fixture)
{
    Fred::OperationContext ctx;
    BOOST_CHECK(!Fred::CheckDomain(test_domain_name).is_invalid_handle(ctx));
    BOOST_CHECK(!Fred::CheckDomain(test_domain_name).is_bad_zone(ctx));
    BOOST_CHECK(!Fred::CheckDomain(test_domain_name).is_bad_length(ctx));
    BOOST_CHECK(!Fred::CheckDomain(test_domain_name).is_blacklisted(ctx));
    std::string conflicting_handle;
    BOOST_CHECK(!Fred::CheckDomain(blacklisted_domain_name).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(conflicting_handle.empty());
    BOOST_CHECK(!Fred::CheckDomain(blacklisted_domain_name).is_registered(ctx));
    BOOST_CHECK(!Fred::CheckDomain(test_domain_name).is_available(ctx));
}

BOOST_AUTO_TEST_SUITE_END();//TestCheckDomain

