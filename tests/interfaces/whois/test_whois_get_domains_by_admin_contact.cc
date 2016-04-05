//many registrars!
//many registrants!
//many contacts!
#include "tests/interfaces/whois/fixture_common.h"
#include "util/util.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_admin_contact)

struct domains_by_admin_contact_fixture
: whois_impl_instance_fixture
{
    std::map<std::string, Fred::InfoDomainData> domain_info;
    Fred::OperationContext ctx;
    empty_contact_fixture system_admin, regular_admin, ecf;
    empty_registrar_fixture erf;
    int regular_domains;

    domains_by_admin_contact_fixture()
    : system_admin("system-admin"),
      regular_admin("regular-admin"),
      regular_domains(6)
    {
        std::string tmp_handle;
        for(int i=0; i < regular_domains; ++i)
        {
            tmp_handle = std::string("test") + boost::lexical_cast<std::string>(i) + ".cz";
            domain_info[tmp_handle] = 
                Test::exec(
                    Fred::CreateDomain(tmp_handle, erf.registrar.handle, ecf.contact.handle)
                      .set_admin_contacts(Util::vector_of<std::string>(regular_admin.contact.handle)),
                    ctx);
        }
        for(int i=0; i < 3; ++i)//3 different domains for another contact
        {
            tmp_handle = std::string("test-other") + boost::lexical_cast<std::string>(i) + ".cz";
            domain_info[tmp_handle] = 
                Test::exec(
                    Fred::CreateDomain(tmp_handle, erf.registrar.handle, ecf.contact.handle)
                      .set_admin_contacts(Util::vector_of<std::string>(system_admin.contact.handle)),
                    ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact, domains_by_admin_contact_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_admin.contact.handle, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == static_cast<unsigned>(regular_domains));
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        found = domain_info.find(it->fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->second.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->second.creation_time);
        BOOST_CHECK(it->registrant_handle == found->second.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->second.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_limit_exceeded, domains_by_admin_contact_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_admin.contact.handle, regular_domains);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() != static_cast<unsigned>(regular_domains));
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        found = domain_info.find(it->fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->second.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->second.creation_time);
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
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
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
