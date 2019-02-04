#include "test/backend/whois/fixture_common.hh"

#include "libfred/registrable_object/keyset/keyset_dns_key.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_keyset_by_handle)

struct get_keyset_by_handle_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    LibFred::InfoKeysetData keyset;

    get_keyset_by_handle_fixture()
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        keyset = Test::exec(
                Test::CreateX_factory<::LibFred::CreateKeyset>().make(registrar.handle)
                    .set_dns_keys(
                        Util::vector_of<::LibFred::DnsKey>(
                            LibFred::DnsKey(42, 777, 13, "any-key")))
                    .set_tech_contacts(
                        Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle)),
                ctx);
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
                ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle, get_keyset_by_handle_fixture)
{
    Fred::Backend::Whois::KeySet ks = impl.get_keyset_by_handle(keyset.handle);

    BOOST_CHECK(ks.created == now_utc);
    BOOST_CHECK(ks.changed.isnull());
    BOOST_CHECK(ks.last_transfer.isnull());
    BOOST_CHECK(ks.handle == keyset.handle);
    BOOST_CHECK(ks.sponsoring_registrar == keyset.sponsoring_registrar_handle);
    BOOST_FOREACH(const LibFred::DnsKey& it, keyset.dns_keys)
    {
        bool key_found = false;
        BOOST_FOREACH(const Fred::Backend::Whois::DNSKey& dit, ks.dns_keys)
        {
            if(it.get_key() == dit.public_key)
            {
                key_found = true;
                BOOST_CHECK(dit.alg == it.get_alg());
                BOOST_CHECK(dit.flags == it.get_flags());
                BOOST_CHECK(dit.protocol == it.get_protocol());
            }
        }
        BOOST_CHECK(key_found);
    }
    BOOST_FOREACH(const LibFred::ObjectIdHandlePair& it, keyset.tech_contacts)
    {
        BOOST_CHECK(ks.tech_contacts.end() !=
                std::find(ks.tech_contacts.begin(), ks.tech_contacts.end(), it.handle));
    }
    BOOST_CHECK(ks.tech_contacts.size() == keyset.tech_contacts.size());
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_no_keyset, get_keyset_by_handle_fixture)
{
    BOOST_CHECK_THROW(impl.get_keyset_by_handle("fine-keyset-handle"), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_wrong_nsset, get_keyset_by_handle_fixture)
{
    BOOST_CHECK_THROW(impl.get_keyset_by_handle(""), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_keyset_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
