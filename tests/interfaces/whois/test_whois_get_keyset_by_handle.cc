#include "tests/interfaces/whois/fixture_common.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_keyset_by_handle)

struct get_keyset_by_handle_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContext ctx;
    const Fred::InfoRegistrarData registrar;
    const Fred::InfoKeysetData keyset;
    const boost::posix_time::ptime now_utc;
    const boost::posix_time::ptime now_prague;

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
                      .exec("SELECT now()::timestamp")[0][0]))),
      now_prague(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                      .exec("SELECT now() AT TIME ZONE 'Europe/Prague'")[0][0])))
    {
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle, get_keyset_by_handle_fixture)
{
    Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(keyset.handle);

    BOOST_CHECK(ks.changed.get_value() == ptime(not_a_date_time));
    BOOST_CHECK(ks.created == now_utc);
    BOOST_CHECK(ks.dns_keys.at(0).alg == keyset.dns_keys.at(0).get_alg());
    BOOST_CHECK(ks.dns_keys.at(0).flags == keyset.dns_keys.at(0).get_flags());
    BOOST_CHECK(ks.dns_keys.at(0).protocol ==
            keyset.dns_keys.at(0).get_protocol());
    BOOST_CHECK(ks.dns_keys.at(0).public_key ==
            keyset.dns_keys.at(0).get_key());
    BOOST_CHECK(ks.handle == keyset.handle);
    BOOST_CHECK(ks.last_transfer.isnull());
    BOOST_CHECK(ks.registrar_handle == keyset.create_registrar_handle);
    BOOST_CHECK(ks.tech_contact_handles.at(0) ==
            keyset.tech_contacts.at(0).handle);
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_no_keyset,
                        get_keyset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySet ks =
            impl.get_keyset_by_handle("fine-keyset-handle");
        BOOST_ERROR("unreported dangling keyset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_wrong_nsset,
                        get_keyset_by_handle_fixture)
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
