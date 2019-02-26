/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/whois/fixture_common.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_keysets_by_tech_c)

struct get_keysets_by_tech_c_fixture
: whois_impl_instance_fixture
{
    const unsigned long test_limit;
    boost::posix_time::ptime now_utc;
    std::map<std::string, LibFred::InfoKeysetData> keyset_info;
    std::string contact_handle;

    get_keysets_by_tech_c_fixture()
    : test_limit(10)
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const LibFred::InfoContactData contact = Test::contact::make(ctx);
        contact_handle = contact.handle;
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
        for(unsigned long i = 0; i < test_limit; ++i)
        {
            const LibFred::InfoKeysetData& ikd = Test::exec(
                    Test::CreateX_factory<::LibFred::CreateKeyset>()
                        .make(registrar.handle)
                        .set_dns_keys(Util::vector_of<::LibFred::DnsKey>(
                            LibFred::DnsKey(42, 777, 13, "any-key")))
                        .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
                    ctx);
            keyset_info[ikd.handle] = ikd;
        }
        for(unsigned long i = 0; i < 3; ++i)
        {
            Test::exec(
                Test::CreateX_factory<::LibFred::CreateKeyset>()
                    .make(registrar.handle)
                    .set_dns_keys(Util::vector_of<::LibFred::DnsKey>(
                        LibFred::DnsKey(42, 777, 13, "any-key")))
                    .set_tech_contacts(Util::vector_of<std::string>(
                        Test::contact::make(ctx).handle)),
                ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c, get_keysets_by_tech_c_fixture)
{
    Fred::Backend::Whois::KeySetSeq ks_s = impl.get_keysets_by_tech_c(contact_handle, test_limit);
    BOOST_CHECK(!ks_s.limit_exceeded);
    BOOST_CHECK(ks_s.content.size() == test_limit);
    BOOST_FOREACH(const Fred::Backend::Whois::KeySet& it, ks_s.content)
    {
        std::map<std::string, LibFred::InfoKeysetData>::const_iterator cit = keyset_info.find(it.handle);
        BOOST_REQUIRE(cit != keyset_info.end());
        const LibFred::InfoKeysetData& found = cit->second;
        BOOST_REQUIRE(it.handle == found.handle);
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.handle == found.handle);
        BOOST_CHECK(it.sponsoring_registrar == found.sponsoring_registrar_handle);
        BOOST_FOREACH(const LibFred::DnsKey& kit, found.dns_keys)
        {
            bool key_found = false;
            BOOST_FOREACH(const Fred::Backend::Whois::DNSKey& dit, it.dns_keys)
            {
                if(kit.get_key() == dit.public_key)
                {
                    key_found = true;
                    BOOST_CHECK(dit.alg == kit.get_alg());
                    BOOST_CHECK(dit.flags == kit.get_flags());
                    BOOST_CHECK(dit.protocol == kit.get_protocol());
                }
            }
            BOOST_CHECK(key_found);
        }
        BOOST_FOREACH(const LibFred::RegistrableObject::Contact::ContactReference& oit, found.tech_contacts)
        {
            BOOST_CHECK(it.tech_contacts.end() !=
                    std::find(it.tech_contacts.begin(), it.tech_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.tech_contacts.size() == found.tech_contacts.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_limit_exceeded, get_keysets_by_tech_c_fixture)
{
    Fred::Backend::Whois::KeySetSeq ks_s = impl.get_keysets_by_tech_c(contact_handle, test_limit - 1);
    BOOST_CHECK(ks_s.limit_exceeded);
    BOOST_CHECK(ks_s.content.size() == test_limit - 1);
    std::map<std::string, LibFred::InfoKeysetData>::const_iterator cit;
    BOOST_FOREACH(const Fred::Backend::Whois::KeySet& it, ks_s.content)
    {
        cit = keyset_info.find(it.handle);
        BOOST_REQUIRE(cit != keyset_info.end());
        const LibFred::InfoKeysetData& found = cit->second;
        BOOST_REQUIRE(it.handle == found.handle);
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.handle == found.handle);
        BOOST_CHECK(it.sponsoring_registrar == found.sponsoring_registrar_handle);
        BOOST_FOREACH(const LibFred::DnsKey& kit, found.dns_keys)
        {
            bool key_found = false;
            BOOST_FOREACH(const Fred::Backend::Whois::DNSKey& dit, it.dns_keys)
            {
                if (kit.get_key() == dit.public_key)
                {
                    key_found = true;
                    BOOST_CHECK(dit.alg == kit.get_alg());
                    BOOST_CHECK(dit.flags == kit.get_flags());
                    BOOST_CHECK(dit.protocol == kit.get_protocol());
                }
            }
            BOOST_CHECK(key_found);
        }
        BOOST_FOREACH(const auto& tech_contact, found.tech_contacts)
        {
            BOOST_CHECK(it.tech_contacts.end() !=
                    std::find(it.tech_contacts.begin(), it.tech_contacts.end(), tech_contact.handle));
        }
        BOOST_CHECK(it.tech_contacts.size() == found.tech_contacts.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_no_contact, get_keysets_by_tech_c_fixture)
{
    BOOST_CHECK_THROW(impl.get_keysets_by_tech_c("fine-tech-c-handle", 1), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_wrong_contact, get_keysets_by_tech_c_fixture)
{
    BOOST_CHECK_THROW(impl.get_keysets_by_tech_c("", 1), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_keysets_by_tech_c
BOOST_AUTO_TEST_SUITE_END()//TestWhois
