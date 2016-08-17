#include "tests/interfaces/whois/fixture_common.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_keysets_by_tech_c)

struct get_keysets_by_tech_c_fixture
: whois_impl_instance_fixture
{
    const unsigned long test_limit;
    boost::posix_time::ptime now_utc;
    std::map<std::string, Fred::InfoKeysetData> keyset_info;
    std::string contact_handle;

    get_keysets_by_tech_c_fixture()
    : test_limit(10)
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const Fred::InfoContactData contact = Test::contact::make(ctx);
        contact_handle = contact.handle;
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
        for(unsigned long i = 0; i < test_limit; ++i)
        {
            const Fred::InfoKeysetData& ikd = Test::exec(
                    Test::CreateX_factory<Fred::CreateKeyset>()
                        .make(registrar.handle)
                        .set_dns_keys(Util::vector_of<Fred::DnsKey>(
                            Fred::DnsKey(42, 777, 13, "any-key")))
                        .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
                    ctx);
            keyset_info[ikd.handle] = ikd;
        }
        for(unsigned long i = 0; i < 3; ++i)
        {
            Test::exec(
                Test::CreateX_factory<Fred::CreateKeyset>()
                    .make(registrar.handle)
                    .set_dns_keys(Util::vector_of<Fred::DnsKey>(
                        Fred::DnsKey(42, 777, 13, "any-key")))
                    .set_tech_contacts(Util::vector_of<std::string>(
                        Test::contact::make(ctx).handle)),
                ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c, get_keysets_by_tech_c_fixture)
{
    Registry::WhoisImpl::KeySetSeq ks_s = impl.get_keysets_by_tech_c(contact_handle, test_limit);
    BOOST_CHECK(!ks_s.limit_exceeded);
    BOOST_CHECK(ks_s.content.size() == test_limit);
    BOOST_FOREACH(const Registry::WhoisImpl::KeySet& it, ks_s.content)
    {
        std::map<std::string, Fred::InfoKeysetData>::const_iterator cit = keyset_info.find(it.handle);
        BOOST_REQUIRE(cit != keyset_info.end());
        const Fred::InfoKeysetData& found = cit->second;
        BOOST_REQUIRE(it.handle == found.handle);
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.handle == found.handle);
        BOOST_CHECK(it.sponsoring_registrar == found.sponsoring_registrar_handle);
        BOOST_FOREACH(const Fred::DnsKey& kit, found.dns_keys)
        {
            bool key_found = false;
            BOOST_FOREACH(const Registry::WhoisImpl::DNSKey& dit, it.dns_keys)
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
        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found.tech_contacts)
        {
            BOOST_CHECK(it.tech_contacts.end() !=
                    std::find(it.tech_contacts.begin(), it.tech_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.tech_contacts.size() == found.tech_contacts.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_limit_exceeded, get_keysets_by_tech_c_fixture)
{
    Registry::WhoisImpl::KeySetSeq ks_s = impl.get_keysets_by_tech_c(contact_handle, test_limit - 1);
    BOOST_CHECK(ks_s.limit_exceeded);
    BOOST_CHECK(ks_s.content.size() == test_limit - 1);
    std::map<std::string, Fred::InfoKeysetData>::const_iterator cit;
    BOOST_FOREACH(const Registry::WhoisImpl::KeySet& it, ks_s.content)
    {
        cit = keyset_info.find(it.handle);
        BOOST_REQUIRE(cit != keyset_info.end());
        const Fred::InfoKeysetData& found = cit->second;
        BOOST_REQUIRE(it.handle == found.handle);
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.handle == found.handle);
        BOOST_CHECK(it.sponsoring_registrar == found.sponsoring_registrar_handle);
        BOOST_FOREACH(const Fred::DnsKey& kit, found.dns_keys)
        {
            bool key_found = false;
            BOOST_FOREACH(const Registry::WhoisImpl::DNSKey& dit, it.dns_keys)
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
        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found.tech_contacts)
        {
            BOOST_CHECK(it.tech_contacts.end() !=
                    std::find(it.tech_contacts.begin(), it.tech_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.tech_contacts.size() == found.tech_contacts.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_no_contact, get_keysets_by_tech_c_fixture)
{
    BOOST_CHECK_THROW(impl.get_keysets_by_tech_c("fine-tech-c-handle", 1), Registry::WhoisImpl::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_wrong_contact, get_keysets_by_tech_c_fixture)
{
    BOOST_CHECK_THROW(impl.get_keysets_by_tech_c("", 1), Registry::WhoisImpl::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_keysets_by_tech_c
BOOST_AUTO_TEST_SUITE_END()//TestWhois
