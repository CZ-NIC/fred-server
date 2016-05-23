#include "tests/interfaces/whois/fixture_common.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "src/fredlib/keyset/keyset_dns_key.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_keyset_by_handle)

struct get_keyset_by_handle_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContextCreator ctx;
    const Fred::InfoRegistrarData registrar;
    const Fred::InfoKeysetData keyset;
    const boost::posix_time::ptime now_utc;

    get_keyset_by_handle_fixture()
    : registrar(Test::registrar::make(ctx)),
      keyset(
          Test::exec(
              Test::CreateX_factory<Fred::CreateKeyset>().make(registrar.handle)
                  .set_dns_keys(Util::vector_of<Fred::DnsKey>(
                      Fred::DnsKey(42, 777, 13, "any-key")))//what key has to be here?
                  .set_tech_contacts(Util::vector_of<std::string>(
                      Test::contact::make(ctx).handle)),
              ctx)),
      now_utc(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                      .exec("SELECT now()::timestamp")[0][0])))
    {
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle, get_keyset_by_handle_fixture)
{
    Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(keyset.handle);

    BOOST_CHECK(ks.created == now_utc);
    BOOST_CHECK(ks.changed.isnull());
    BOOST_CHECK(ks.last_transfer.isnull());
    BOOST_CHECK(ks.handle == keyset.handle);
    BOOST_CHECK(ks.creating_registrar == keyset.create_registrar_handle);
    BOOST_FOREACH(Fred::DnsKey it, keyset.dns_keys)
    {
        bool key_found = false;
        BOOST_FOREACH(Registry::WhoisImpl::DNSKey dit, ks.dns_keys)
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
    BOOST_FOREACH(Fred::ObjectIdHandlePair it, keyset.tech_contacts)
    {
        BOOST_CHECK(ks.tech_contact_handles.end() !=
                std::find(ks.tech_contact_handles.begin(), ks.tech_contact_handles.end(), it.handle));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_no_keyset, get_keyset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle("fine-keyset-handle");
        BOOST_ERROR("unreported dangling keyset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_wrong_nsset, get_keyset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle("");
        BOOST_ERROR("keyset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_keyset_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
