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

#include <boost/test/unit_test.hpp>
#include <string>

#include "src/fredlib/opcontext.h"
#include <fredlib/contact.h>
#include <fredlib/keyset.h>
#include <fredlib/nsset.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"
#include "tests/fredlib/util.h"

BOOST_FIXTURE_TEST_SUITE(TestCheckHandle, Test::Fixture::instantiate_db_template)

const std::string server_name = "test-check-handle";

struct check_handle_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact_handle;
    std::string admin_contact_handle_rem;
    std::string test_nsset_handle;
    std::string test_nsset_handle_rem;
    std::string test_keyset_handle;
    std::string test_keyset_handle_rem;

    check_handle_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , admin_contact_handle_rem(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark+"-REM")
    , test_nsset_handle ( std::string("TEST-NSSET-")+xmark+"-HANDLE")
    , test_nsset_handle_rem ( std::string("TEST-NSSET-")+xmark+"-HANDLE-REM")
    , test_keyset_handle ( std::string("TEST-KEYSET-")+xmark+"-HANDLE")
    , test_keyset_handle_rem ( std::string("TEST-KEYSET-")+xmark+"-HANDLE-REM")
    {
        namespace ip = boost::asio::ip;

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

        Fred::CreateContact(admin_contact_handle_rem,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAMEREM")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::DeleteContactByHandle(admin_contact_handle_rem).exec(ctx);

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

        Fred::CreateNsset(test_nsset_handle_rem, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

        Fred::DeleteNssetByHandle(test_nsset_handle_rem).exec(ctx);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        Fred::CreateKeyset(test_keyset_handle_rem, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        Fred::DeleteKeysetByHandle(test_keyset_handle_rem).exec(ctx);

        ctx.commit_transaction();
    }
    ~check_handle_fixture()
    {}
};

/**
 * test CheckContact true returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_contact_handle_true, check_handle_fixture)
{
    Fred::OperationContext ctx;
    std::string conflicting_handle;
    BOOST_CHECK(Fred::CheckContact(admin_contact_handle).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(admin_contact_handle.compare(conflicting_handle) == 0);
    BOOST_CHECK(Fred::CheckContact(admin_contact_handle).is_registered(ctx));
    BOOST_CHECK(Fred::CheckContact(admin_contact_handle+"@").is_invalid_handle());
    BOOST_CHECK(Fred::CheckContact(admin_contact_handle_rem).is_protected(ctx));
    BOOST_CHECK(Fred::CheckContact(admin_contact_handle+xmark).is_free(ctx));
}

/**
 * test CheckContact false returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_contact_handle_false, check_handle_fixture)
{
    BOOST_CHECK(!Fred::CheckContact(admin_contact_handle).is_invalid_handle());
    Fred::OperationContext ctx;
    std::string conflicting_handle;
    BOOST_CHECK(!Fred::CheckContact(admin_contact_handle+xmark).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(conflicting_handle.empty());
    BOOST_CHECK(!Fred::CheckContact(admin_contact_handle).is_protected(ctx));
    BOOST_CHECK(!Fred::CheckContact(admin_contact_handle).is_free(ctx));
}

/**
 * test CheckNsset true returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_nsset_handle_true, check_handle_fixture)
{
    Fred::OperationContext ctx;
    std::string conflicting_handle;
    BOOST_CHECK(Fred::CheckNsset(test_nsset_handle).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(test_nsset_handle.compare(conflicting_handle) == 0);
    BOOST_CHECK(Fred::CheckNsset(test_nsset_handle).is_registered(ctx));
    BOOST_CHECK(Fred::CheckNsset(test_nsset_handle+"@").is_invalid_handle());
    BOOST_CHECK(Fred::CheckNsset(test_nsset_handle_rem).is_protected(ctx));
    BOOST_CHECK(Fred::CheckNsset(test_nsset_handle+xmark).is_free(ctx));
}

/**
 * test CheckNsset false returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_nsset_handle_false, check_handle_fixture)
{
    BOOST_CHECK(!Fred::CheckNsset(test_nsset_handle).is_invalid_handle());
    Fred::OperationContext ctx;
    std::string conflicting_handle;
    BOOST_CHECK(!Fred::CheckNsset(test_nsset_handle+xmark).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(conflicting_handle.empty());
    BOOST_CHECK(!Fred::CheckNsset(test_nsset_handle).is_protected(ctx));
    BOOST_CHECK(!Fred::CheckNsset(test_nsset_handle).is_free(ctx));
}


/**
 * test CheckKeyset true returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_keyset_handle_true, check_handle_fixture)
{
    Fred::OperationContext ctx;
    std::string conflicting_handle;
    BOOST_CHECK(Fred::CheckKeyset(test_keyset_handle).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(test_keyset_handle.compare(conflicting_handle) == 0);
    BOOST_CHECK(Fred::CheckKeyset(test_keyset_handle).is_registered(ctx));
    BOOST_CHECK(Fred::CheckKeyset(test_keyset_handle+"@").is_invalid_handle());
    BOOST_CHECK(Fred::CheckKeyset(test_keyset_handle_rem).is_protected(ctx));
    BOOST_CHECK(Fred::CheckKeyset(test_keyset_handle+xmark).is_free(ctx));
}

/**
 * test CheckKeyset false returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_keyset_handle_false, check_handle_fixture)
{
    BOOST_CHECK(!Fred::CheckKeyset(test_keyset_handle).is_invalid_handle());
    Fred::OperationContext ctx;
    std::string conflicting_handle;
    BOOST_CHECK(!Fred::CheckKeyset(test_keyset_handle+xmark).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(conflicting_handle.empty());
    BOOST_CHECK(!Fred::CheckKeyset(test_keyset_handle).is_protected(ctx));
    BOOST_CHECK(!Fred::CheckKeyset(test_keyset_handle).is_free(ctx));
}



BOOST_AUTO_TEST_SUITE_END();//TestCheckHandle

