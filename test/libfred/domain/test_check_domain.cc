/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
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
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/asio/ip/address.hpp>
#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-check-domain";

struct check_domain_fixture : public Test::instantiate_db_template
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
    : xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , admin_contact_handle(std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark)
    , test_nsset_handle ( std::string("TEST-NSSET-")+xmark+"-HANDLE")
    , test_keyset_handle ( std::string("TEST-KEYSET-")+xmark+"-HANDLE")
    , test_domain_name ( std::string("testfred")+xmark+".cz")
    , test_domain_name_rem ( std::string("testremfred")+xmark+".cz")
    , blacklisted_domain_name("fredblack"+xmark+".cz")
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

        ::LibFred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<::LibFred::DnsHost>
                (::LibFred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
                (::LibFred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
                ).exec(ctx);

        ::LibFred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        ::LibFred::CreateDomain(test_domain_name, registrar_handle, registrant_contact_handle)
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_keyset(test_keyset_handle).set_nsset(test_nsset_handle).exec(ctx);

        ::LibFred::CreateDomain(test_domain_name_rem, registrar_handle, registrant_contact_handle)
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_keyset(test_keyset_handle).set_nsset(test_nsset_handle).exec(ctx);

        ::LibFred::DeleteDomainByFqdn(test_domain_name_rem).exec(ctx);

        //blacklist
        ctx.get_conn().exec_params("INSERT into domain_blacklist (regexp,valid_from,valid_to,reason) "
            " VALUES ($1::text,current_timestamp,current_timestamp + interval '1 month','test' )"
            , Database::query_param_list(std::string("^")+blacklisted_domain_name+"$"));

        ctx.commit_transaction();
    }
    ~check_domain_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCheckDomain, check_domain_fixture)

/**
 * test CheckDomain true returning cases
 */
BOOST_AUTO_TEST_CASE(check_domain_name_true)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(::LibFred::CheckDomain(std::string("-")+test_domain_name).is_invalid_syntax(ctx));
    BOOST_CHECK(::LibFred::CheckDomain(std::string("testfred")+xmark+".czz").is_bad_zone(ctx));
    BOOST_CHECK(::LibFred::CheckDomain(std::string("testfred")+xmark+".czz.cz").is_bad_length(ctx));
    BOOST_CHECK(::LibFred::CheckDomain(blacklisted_domain_name).is_blacklisted(ctx));
    std::string conflicting_handle;
    BOOST_CHECK(::LibFred::CheckDomain(test_domain_name).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(test_domain_name.compare(conflicting_handle) == 0);
    BOOST_CHECK(::LibFred::CheckDomain(test_domain_name).is_registered(ctx));
    BOOST_CHECK(::LibFred::CheckDomain(std::string("testfred")+xmark+"available.cz").is_available(ctx));
}

/**
 * test CheckDomain false returning cases
 */

BOOST_AUTO_TEST_CASE(check_domain_name_false)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(!LibFred::CheckDomain(test_domain_name).is_invalid_syntax(ctx));
    BOOST_CHECK(!LibFred::CheckDomain(test_domain_name).is_bad_zone(ctx));
    BOOST_CHECK(!LibFred::CheckDomain(test_domain_name).is_bad_length(ctx));
    BOOST_CHECK(!LibFred::CheckDomain(test_domain_name).is_blacklisted(ctx));
    std::string conflicting_handle;
    BOOST_CHECK(!LibFred::CheckDomain(blacklisted_domain_name).is_registered(ctx, conflicting_handle));
    BOOST_CHECK(conflicting_handle.empty());
    BOOST_CHECK(!LibFred::CheckDomain(blacklisted_domain_name).is_registered(ctx));
    BOOST_CHECK(!LibFred::CheckDomain(test_domain_name).is_available(ctx));
}

BOOST_AUTO_TEST_SUITE_END();//TestCheckDomain

