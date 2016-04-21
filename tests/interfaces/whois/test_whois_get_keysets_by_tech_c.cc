#include "tests/interfaces/whois/fixture_common.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_keysets_by_tech_c)

struct get_keysets_by_tech_c_fixture
: whois_impl_instance_fixture
{
    typedef Registry::WhoisImpl::KeySetSeq KeySetSeq;
    typedef Registry::WhoisImpl::KeySet KeySet;

    Fred::OperationContext ctx;
    const Fred::InfoRegistrarData registrar;
    const Fred::InfoContactData contact;
    const boost::posix_time::ptime now_utc;
    const boost::posix_time::ptime now_prague;
    unsigned long test_limit;
    std::map<std::string, Fred::InfoKeysetData> keyset_info;

    get_keysets_by_tech_c_fixture()
    : registrar(Test::registrar::make(ctx)),
      contact(Test::contact::make(ctx)),
      now_utc(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                      .exec("SELECT now()::timestamp")[0][0]))),
      now_prague(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                      .exec("SELECT now() AT TIME ZONE 'Europe/Prague'")[0][0]))),
      test_limit(10)
    {
        for(unsigned long i = 0; i < test_limit; ++i)
        {
            const Fred::InfoKeysetData& ikd = Test::exec(
                    Test::CreateX_factory<Fred::CreateKeyset>()
                        .make(registrar.handle)
                        .set_dns_keys(Util::vector_of<Fred::DnsKey>(
                            Fred::DnsKey(42, 777, 13, "any-key")))//what key has to be here?
                        .set_tech_contacts(Util::vector_of<std::string>(
                            contact.handle)),
                    ctx);
            keyset_info[ikd.handle] = ikd;
        }
        for(unsigned long i = 0; i < 3; ++i)
        {
            Test::exec(
                    Test::CreateX_factory<Fred::CreateKeyset>()
                        .make(registrar.handle)
                        .set_dns_keys(Util::vector_of<Fred::DnsKey>(
                            Fred::DnsKey(42, 777, 13, "any-key")))//what key has to be here?
                        .set_tech_contacts(Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle)),//random contact
                    ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c, get_keysets_by_tech_c_fixture)
{
    KeySetSeq ks_s = impl.get_keysets_by_tech_c(contact.handle, test_limit);
    BOOST_CHECK(!ks_s.limit_exceeded);
    BOOST_CHECK(ks_s.content.size() == test_limit);
    for(std::vector<KeySet>::iterator it = ks_s.content.begin();
        it != ks_s.content.end();
        ++it)
    {
        Fred::InfoKeysetData& found = keyset_info[it->handle];
        BOOST_REQUIRE(it->handle == found.handle);
        BOOST_CHECK(it->changed.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->created == now_utc);
        BOOST_CHECK(it->dns_keys.at(0).alg == found.dns_keys.at(0).get_alg());
        BOOST_CHECK(it->dns_keys.at(0).flags ==
                found.dns_keys.at(0).get_flags());
        BOOST_CHECK(it->dns_keys.at(0).protocol ==
                found.dns_keys.at(0).get_protocol());
        BOOST_CHECK(it->dns_keys.at(0).public_key ==
                found.dns_keys.at(0).get_key());
        BOOST_CHECK(it->last_transfer.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->registrar_handle == found.create_registrar_handle);
        BOOST_CHECK(it->tech_contact_handles.at(0) ==
                found.tech_contacts.at(0).handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_limit_exceeded,
                        get_keysets_by_tech_c_fixture)
{
    KeySetSeq ks_s = impl.get_keysets_by_tech_c(contact.handle, test_limit - 1);
    BOOST_CHECK(ks_s.limit_exceeded);
    BOOST_CHECK(ks_s.content.size() == test_limit - 1);
    for(std::vector<KeySet>::iterator it = ks_s.content.begin();
        it != ks_s.content.end();
        ++it)
    {
        Fred::InfoKeysetData& found = keyset_info[it->handle];
        BOOST_REQUIRE(it->handle == found.handle);
        BOOST_CHECK(it->changed.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->created == now_utc);
        BOOST_CHECK(it->dns_keys.at(0).alg == found.dns_keys.at(0).get_alg());
        BOOST_CHECK(it->dns_keys.at(0).flags ==
                found.dns_keys.at(0).get_flags());
        BOOST_CHECK(it->dns_keys.at(0).protocol ==
                found.dns_keys.at(0).get_protocol());
        BOOST_CHECK(it->dns_keys.at(0).public_key ==
                found.dns_keys.at(0).get_key());
        BOOST_CHECK(it->last_transfer.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->registrar_handle == found.create_registrar_handle);
        BOOST_CHECK(it->tech_contact_handles.at(0) ==
                found.tech_contacts.at(0).handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_no_contact,
                        get_keysets_by_tech_c_fixture)
{
    try
    {
        KeySetSeq ks_s = impl.get_keysets_by_tech_c("fine-tech-c-handle",
                                                    test_limit);
        BOOST_ERROR("unreported dangling KeySets");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_wrong_contact,
                        get_keysets_by_tech_c_fixture)
{
    try
    {
        KeySetSeq ks_s = impl.get_keysets_by_tech_c("", test_limit);
        BOOST_ERROR("tech contact handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_keysets_by_tech_c
BOOST_AUTO_TEST_SUITE_END()//TestWhois
