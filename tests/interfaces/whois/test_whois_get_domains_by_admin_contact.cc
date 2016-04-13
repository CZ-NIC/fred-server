#include "tests/interfaces/whois/fixture_common.h"
#include "util/util.h"
#include "tests/setup/fixtures_utils.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_admin_contact)

struct domains_by_admin_contact_fixture
: whois_impl_instance_fixture
{
    std::map<std::string, Fred::InfoDomainData> domain_info;
    Fred::OperationContext ctx;
    const Fred::InfoRegistrarData registrar; // rly need?
    const Fred::InfoContactData system_admin, regular_admin, contact;
    int regular_domains;
    const boost::posix_time::ptime now_utc;
    const boost::posix_time::ptime now_prague;

    domains_by_admin_contact_fixture()
    : registrar(Test::registrar::make(ctx)),
      system_admin(Test::contact::make(ctx)),
      regular_admin(Test::contact::make(ctx)),
      contact(Test::contact::make(ctx)),
      regular_domains(6),
      now_utc(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn().exec("SELECT now()::timestamp")[0][0]))),
      now_prague(boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec("SELECT now() AT TIME ZONE 'Europe/Prague'")[0][0])))
    {
        std::string tmp_handle;
        for(int i=0; i < regular_domains; ++i)
        {
            const Fred::InfoDomainData& idd = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>().make(registrar.handle, contact.handle)
                    .set_admin_contacts(Util::vector_of<std::string>(regular_admin.handle)),
                ctx);
            domain_info[idd.fqdn] = idd;
        }
        for(int i=0; i < 3; ++i)//3 different domains for another contact
        {
            Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                       .make(registrar.handle, contact.handle)
                       .set_admin_contacts(
                           Util::vector_of<std::string>(system_admin.handle)),
                       ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact, domains_by_admin_contact_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_admin.handle, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == static_cast<unsigned>(regular_domains));
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        found = domain_info.find(it->fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->second.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->last_transfer.get_value() == ptime(not_a_date_time));
        BOOST_CHECK_EQUAL(it->registered, now_utc);
        BOOST_CHECK(it->registrant_handle == found->second.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->second.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_limit_exceeded, domains_by_admin_contact_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_admin.handle, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == static_cast<unsigned>(regular_domains - 1));
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        found = domain_info.find(it->fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->second.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->last_transfer.get_value() == ptime(not_a_date_time));
        BOOST_CHECK_EQUAL(it->registered, now_utc);
        BOOST_CHECK(it->registrant_handle == found->second.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->second.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_no_contact, whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_admin_contact("absent-contact", 0);
        BOOST_ERROR("unreported dangling registrant");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_wrong_contact, whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_admin_contact("", 0);
        BOOST_ERROR("registrant handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_admin_contact
BOOST_AUTO_TEST_SUITE_END();//TestWhois
