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
#include <fredlib/registrar.h>

#include "src/fredlib/contact/check_contact.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

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
    , admin_contact_handle(std::string("TEST-ADMIN-C-") + xmark)
    , admin_contact_handle_rem(std::string("TEST-ADMIN-C-") + xmark + "-REM")
    , test_nsset_handle(std::string("TEST-NSSET-") + xmark)
    , test_nsset_handle_rem(std::string("TEST-NSSET-") + xmark + "-REM")
    , test_keyset_handle(std::string("TEST-KEYSET-") + xmark)
    , test_keyset_handle_rem(std::string("TEST-KEYSET-") + xmark + "-REM")
    {
        namespace ip = boost::asio::ip;

        Fred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(admin_contact_handle_rem,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAMEREM")+xmark)
            .set_disclosename(true)
            .set_place(place)
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
    BOOST_CHECK(
        Fred::Contact::is_handle_valid(admin_contact_handle + "@")
        ==
        Fred::ContactHandleState::SyntaxValidity::invalid
    );

    Fred::OperationContextCreator ctx;

    BOOST_CHECK(
        Fred::Contact::is_handle_in_registry(ctx, admin_contact_handle)
        ==
        Fred::ContactHandleState::InRegistry::registered
    );

    BOOST_CHECK(
        Fred::Contact::is_handle_in_registry(ctx, admin_contact_handle_rem)
        ==
        Fred::ContactHandleState::InRegistry::in_protection_period
    );

    BOOST_CHECK(
        Fred::Contact::is_handle_in_registry(ctx, admin_contact_handle + xmark)
        ==
        Fred::ContactHandleState::InRegistry::unregistered
    );
}

/**
 * test CheckContact handle - invalid cases
 */
BOOST_AUTO_TEST_CASE(check_contact_handle_validity_invalid)
{
    BOOST_CHECK(Fred::Contact::is_handle_valid("") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("!") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("@") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("#") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("$") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("%") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("^") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("&") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("*") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("(") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid(")") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("[") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("]") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("/") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid(".") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid(",") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid(":") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("{") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("}") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("~") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("'") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("+") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("-") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("\"") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("\\") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("FOO--BAR") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("-FOOBAR") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("FOOBAR-") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("FOOBAR-") == Fred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("1234567890123456789012345678901") == Fred::ContactHandleState::SyntaxValidity::invalid);
}

/**
 * test CheckContact handle - ok cases
 */
BOOST_AUTO_TEST_CASE(check_contact_handle_validity_ok)
{
    BOOST_CHECK(Fred::Contact::is_handle_valid("F") == Fred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("FOOBAR") == Fred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("FOO-BAR") == Fred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("123-BAR") == Fred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("FOO-BAR-BAZ") == Fred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(Fred::Contact::is_handle_valid("123456789012345678901234567890") == Fred::ContactHandleState::SyntaxValidity::valid);
}

/**
 * test CheckNsset true returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_nsset_handle_true, check_handle_fixture)
{
    Fred::OperationContextCreator ctx;
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
    Fred::OperationContextCreator ctx;
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
    Fred::OperationContextCreator ctx;
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
    Fred::OperationContextCreator ctx;
    std::string conflicting_handle;
    BOOST_CHECK(!Fred::CheckKeyset(test_keyset_handle+xmark).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(conflicting_handle.empty());
    BOOST_CHECK(!Fred::CheckKeyset(test_keyset_handle).is_protected(ctx));
    BOOST_CHECK(!Fred::CheckKeyset(test_keyset_handle).is_free(ctx));
}

/**
 * test CheckRegistrar - invalid cases
 */
BOOST_AUTO_TEST_CASE(check_registrar_handle_invalid)
{
    BOOST_CHECK(Fred::CheckRegistrar("").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("1").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("12").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("12345678901234567").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123!").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123@").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123#").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123$").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123%").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123^").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123&").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123*").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123(").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123)").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123[").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123]").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123/").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123.").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123,").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123:").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123{").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123}").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123~").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123'").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123+").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123-").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123\"").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("123\\").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("-FOOBAR").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("FOOBAR-").is_invalid_handle());
    BOOST_CHECK(Fred::CheckRegistrar("FOO--BAR").is_invalid_handle());
}

/**
 * test CheckRegistrar - ok cases
 */
BOOST_AUTO_TEST_CASE(check_registrar_handle_ok)
{
    BOOST_CHECK(!Fred::CheckRegistrar("FOOBAR").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("FOO-BAR").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("MO-O").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("foobar-42").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("REG-FOOBAR").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("REG-FOO-BAR").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("REG-FOO-BAR-42").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("123").is_invalid_handle());
    BOOST_CHECK(!Fred::CheckRegistrar("1234567890123456").is_invalid_handle());
}


BOOST_AUTO_TEST_SUITE_END();//TestCheckHandle

