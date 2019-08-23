/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/delete_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_diff.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/delete_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_diff.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "libfred/registrar/check_registrar.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_FIXTURE_TEST_SUITE(TestCheckHandle, Test::instantiate_db_template)

const std::string server_name = "test-check-handle";

struct check_handle_fixture : public Test::instantiate_db_template
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
    : xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , admin_contact_handle(std::string("TEST-ADMIN-C-") + xmark)
    , admin_contact_handle_rem(std::string("TEST-ADMIN-C-") + xmark + "-REM")
    , test_nsset_handle(std::string("TEST-NSSET-") + xmark)
    , test_nsset_handle_rem(std::string("TEST-NSSET-") + xmark + "-REM")
    , test_keyset_handle(std::string("TEST-KEYSET-") + xmark)
    , test_keyset_handle_rem(std::string("TEST-KEYSET-") + xmark + "-REM")
    {
        namespace ip = boost::asio::ip;

        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateContact(admin_contact_handle_rem,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAMEREM")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::DeleteContactByHandle(admin_contact_handle_rem).exec(ctx);

        ::LibFred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

        ::LibFred::CreateNsset(test_nsset_handle_rem, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

        ::LibFred::DeleteNssetByHandle(test_nsset_handle_rem).exec(ctx);

        ::LibFred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        ::LibFred::CreateKeyset(test_keyset_handle_rem, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        ::LibFred::DeleteKeysetByHandle(test_keyset_handle_rem).exec(ctx);

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
    ::LibFred::OperationContextCreator ctx;

    BOOST_CHECK(
        ::LibFred::Contact::get_handle_syntax_validity(ctx, admin_contact_handle + "@")
        ==
        ::LibFred::ContactHandleState::SyntaxValidity::invalid
    );

    BOOST_CHECK(
        ::LibFred::Contact::get_handle_registrability(ctx, admin_contact_handle)
        ==
        ::LibFred::ContactHandleState::Registrability::registered
    );

    BOOST_CHECK(
        ::LibFred::Contact::get_handle_registrability(ctx, admin_contact_handle_rem)
        ==
        ::LibFred::ContactHandleState::Registrability::in_protection_period
    );

    BOOST_CHECK(
        ::LibFred::Contact::get_handle_registrability(ctx, admin_contact_handle + xmark)
        ==
        ::LibFred::ContactHandleState::Registrability::available
    );
}

/**
 * test CheckContact handle - invalid cases
 */
BOOST_AUTO_TEST_CASE(check_contact_handle_validity_invalid)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "!") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "@") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "#") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "$") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "%") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "^") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "&") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "*") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "(") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, ")") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "[") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "]") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "/") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, ".") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, ",") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, ":") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "{") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "}") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "~") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "'") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "+") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "-") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "\"") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "\\") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "FOO--BAR") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "-FOOBAR") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "FOOBAR-") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "FOOBAR-") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "1234567890123456789012345678901") == ::LibFred::ContactHandleState::SyntaxValidity::invalid);
}

/**
 * test CheckContact handle - ok cases
 */
BOOST_AUTO_TEST_CASE(check_contact_handle_validity_ok)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "F") == ::LibFred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "FOOBAR") == ::LibFred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "FOO-BAR") == ::LibFred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "123-BAR") == ::LibFred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "FOO-BAR-BAZ") == ::LibFred::ContactHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Contact::get_handle_syntax_validity(ctx, "123456789012345678901234567890") == ::LibFred::ContactHandleState::SyntaxValidity::valid);
}

/**
 * test nsset handle ok cases
 */
BOOST_FIXTURE_TEST_CASE(check_nsset_handle_ok, check_handle_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::Nsset::get_handle_registrability(ctx, test_nsset_handle) == ::LibFred::NssetHandleState::Registrability::registered);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, test_nsset_handle+"@") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_registrability(ctx, test_nsset_handle_rem) == ::LibFred::NssetHandleState::Registrability::in_protection_period);
    BOOST_CHECK(::LibFred::Nsset::get_handle_registrability(ctx, test_nsset_handle+"1") == ::LibFred::NssetHandleState::Registrability::unregistered);

}

BOOST_AUTO_TEST_CASE(check_nsset_handle_validity_invalid)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "!") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "@") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "#") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "$") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "%") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "^") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "&") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "*") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "(") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, ")") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "[") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "]") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "/") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, ".") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, ",") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, ":") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "{") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "}") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "~") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "'") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "+") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "-") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "\"") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "\\") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "FOO--BAR") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "-FOOBAR") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "FOOBAR-") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "FOOBAR-") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "1234567890123456789012345678901") == ::LibFred::NssetHandleState::SyntaxValidity::invalid);
}

/**
 * test nsset handle handle - ok cases
 */
BOOST_AUTO_TEST_CASE(check_nsset_handle_validity_ok)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "F") == ::LibFred::NssetHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "FOOBAR") == ::LibFred::NssetHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "FOO-BAR") == ::LibFred::NssetHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "123-BAR") == ::LibFred::NssetHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "FOO-BAR-BAZ") == ::LibFred::NssetHandleState::SyntaxValidity::valid);
    BOOST_CHECK(::LibFred::Nsset::get_handle_syntax_validity(ctx, "123456789012345678901234567890") == ::LibFred::NssetHandleState::SyntaxValidity::valid);
}



/**
 * test CheckKeyset true returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_keyset_handle_true, check_handle_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::Keyset::get_handle_registrability(ctx, test_keyset_handle) == ::LibFred::Keyset::HandleState::registered);
    BOOST_CHECK(::LibFred::Keyset::get_handle_syntax_validity(ctx, test_keyset_handle+"@") == ::LibFred::Keyset::HandleState::invalid);
    BOOST_CHECK(::LibFred::Keyset::get_handle_registrability(ctx, test_keyset_handle_rem) == ::LibFred::Keyset::HandleState::in_protection_period);
    BOOST_CHECK(::LibFred::Keyset::get_handle_registrability(ctx, test_keyset_handle + xmark) == ::LibFred::Keyset::HandleState::available);
}

/**
 * test CheckKeyset false returning cases
 */
BOOST_FIXTURE_TEST_CASE(check_keyset_handle_false, check_handle_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::Keyset::get_handle_syntax_validity(ctx, test_keyset_handle) == ::LibFred::Keyset::HandleState::valid);
    BOOST_CHECK(::LibFred::Keyset::get_handle_registrability(ctx, test_keyset_handle + xmark) != ::LibFred::Keyset::HandleState::registered);
    BOOST_CHECK(::LibFred::Keyset::get_handle_registrability(ctx, test_keyset_handle) != ::LibFred::Keyset::HandleState::in_protection_period);
    BOOST_CHECK(::LibFred::Keyset::get_handle_registrability(ctx, test_keyset_handle) != ::LibFred::Keyset::HandleState::available);
}

/**
 * test CheckRegistrar - invalid cases
 */
BOOST_AUTO_TEST_CASE(check_registrar_handle_invalid)
{
    BOOST_CHECK(::LibFred::CheckRegistrar("").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("1").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("12").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("12345678901234567").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123!").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123@").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123#").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123$").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123%").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123^").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123&").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123*").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123(").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123)").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123[").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123]").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123/").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123.").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123,").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123:").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123{").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123}").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123~").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123'").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123+").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123-").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123\"").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("123\\").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("-FOOBAR").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("FOOBAR-").is_invalid_handle());
    BOOST_CHECK(::LibFred::CheckRegistrar("FOO--BAR").is_invalid_handle());
}

/**
 * test CheckRegistrar - ok cases
 */
BOOST_AUTO_TEST_CASE(check_registrar_handle_ok)
{
    BOOST_CHECK(!LibFred::CheckRegistrar("FOOBAR").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("FOO-BAR").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("MO-O").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("foobar-42").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("REG-FOOBAR").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("REG-FOO-BAR").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("REG-FOO-BAR-42").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("123").is_invalid_handle());
    BOOST_CHECK(!LibFred::CheckRegistrar("1234567890123456").is_invalid_handle());
}


BOOST_AUTO_TEST_SUITE_END();//TestCheckHandle

