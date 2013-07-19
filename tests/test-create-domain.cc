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
#include "fredlib/registrar.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/keyset/info_keyset.h"
#include "fredlib/keyset/info_keyset_history.h"
#include "fredlib/keyset/info_keyset_compare.h"
#include "fredlib/nsset/info_nsset.h"
#include "fredlib/nsset/info_nsset_history.h"
#include "fredlib/nsset/info_nsset_compare.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"
#include "fredlib/domain/info_domain_compare.h"
#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/info_contact_history.h"
#include "fredlib/contact/info_contact_compare.h"


#include "util/util.h"

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

static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

BOOST_AUTO_TEST_SUITE(TestCreateDomain)

const std::string server_name = "test-create-domain";

struct create_domain_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_handle;

    create_domain_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_handle ( std::string("fred")+xmark+".cz")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
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
    }
    ~create_domain_fixture()
    {}
};

/**
 * test CreateDomain with wrong registrar
 */
BOOST_FIXTURE_TEST_CASE(create_domain_wrong_registrar, create_domain_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_registrar_handle = registrar_handle+xmark;
    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreateDomain(
                test_domain_handle //const std::string& fqdn
                , bad_registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        throw;
    }
    , std::exception
    , check_std_exception);
}

/**
 * test CreateDomain with wrong fqdn syntax
 */
BOOST_FIXTURE_TEST_CASE(create_domain_wrong_fqdn_syntax, create_domain_fixture)
{
    Fred::OperationContext ctx;
    std::string bad_test_domain_handle = test_domain_handle+".2bad..";
    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreateDomain(
                bad_test_domain_handle //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::CreateDomain::Exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_invalid_fqdn_syntax());
        throw;
    }
    , std::exception
    , check_std_exception);
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateContact

