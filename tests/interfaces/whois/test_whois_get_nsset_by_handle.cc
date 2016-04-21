#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_nsset_by_handle)

struct get_nsset_by_handle_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContextCreator ctx;
    std::string test_nsset_handle;
    const Fred::InfoRegistrarData registrar;
    const Fred::InfoContactData contact;
    const Fred::InfoNssetData nsset;
    const boost::posix_time::ptime now_utc;
    const boost::posix_time::ptime now_prague;

    get_nsset_by_handle_fixture()
    : test_nsset_handle("TEST-NSSET"),
      registrar(Test::registrar::make(ctx)),     
      contact(Test::contact::make(ctx)),
      nsset(
          Test::exec(
              Test::CreateX_factory<Fred::CreateNsset>()
                  .make(registrar.handle, test_nsset_handle)
                  .set_dns_hosts(Util::vector_of<Fred::DnsHost>(Fred::DnsHost("some-nameserver", Util::vector_of<boost::asio::ip::address>(boost::asio::ip::address()))))
                  .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
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

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle, get_nsset_by_handle_fixture)
{
    Registry::WhoisImpl::NSSet nss =
        impl.get_nsset_by_handle(test_nsset_handle);

    BOOST_CHECK(nss.changed.isnull());
    BOOST_CHECK(nss.last_transfer.isnull());
    BOOST_CHECK(nss.created == now_utc);
    BOOST_CHECK(nss.handle == nsset.handle);
    BOOST_CHECK(nss.nservers.at(0).fqdn == nsset.dns_hosts.at(0).get_fqdn());
    BOOST_CHECK(nss.nservers.at(0).ip_addresses.at(0) ==
            nsset.dns_hosts.at(0).get_inet_addr().at(0));
    BOOST_CHECK(nss.registrar_handle == nsset.create_registrar_handle);
    BOOST_CHECK(nss.tech_contact_handles.at(0) ==
            nsset.tech_contacts.at(0).handle);
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_no_nsset,
                        whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss =
            impl.get_nsset_by_handle("fine-nsset-handle");
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_wrong_nsset,
                        whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle("");
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END()//get_nsset_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
