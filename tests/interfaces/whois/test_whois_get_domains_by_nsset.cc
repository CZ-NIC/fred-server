#include "tests/interfaces/whois/fixture_common.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "tests/setup/fixtures_utils.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_nsset)

struct domains_by_nsset_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContext ctx;
    std::string test_nsset;
    unsigned int regular_domains;
    std::map<std::string, Fred::InfoDomainData> domain_info;
    const Fred::InfoRegistrarData registrar; 
    const Fred::InfoContactData contact, admin;
    const Fred::InfoNssetData nsset, other_nsset;
    const boost::posix_time::ptime now_utc;
    const boost::posix_time::ptime now_prague;

    domains_by_nsset_fixture()
    : test_nsset("test-nsset"),
      regular_domains(6),
      registrar(Test::registrar::make(ctx)),
      contact(Test::contact::make(ctx)),
      admin(Test::contact::make(ctx)),
      nsset(Test::nsset::make(ctx, test_nsset)),
      other_nsset(Test::nsset::make(ctx)),
      now_utc(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                  .exec("SELECT now()::timestamp")[0][0]))),
      now_prague(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                  .exec("SELECT now() AT TIME ZONE 'Europe/Prague'")[0][0])))
    {
        for(int i=0; i < regular_domains; ++i)
        {
            const Fred::InfoDomainData& idd = Test::exec(
                    Test::CreateX_factory<Fred::CreateDomain>()
                    .make(registrar.handle, contact.handle)
                    .set_nsset(test_nsset)
                    .set_admin_contacts(Util::vector_of<std::string>(admin.handle)),
                    ctx);
            domain_info[idd.fqdn] = idd;

//            Fred::CreateDomain(test_fqdn + boost::lexical_cast<std::string>(i) + ".cz", test_registrar_handle, test_registrant_handle)
//                .set_nsset(test_nsset)
//                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
//                .exec(ctx);
        }
        for(int i=0; i < 3; ++i)//3 different domains for another nsset
        {
            Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                           .make(registrar.handle, contact.handle)
                           .set_nsset(other_nsset.handle)
                           .set_admin_contacts(
                               Util::vector_of<std::string>(admin.handle)),
                       ctx);

//            Fred::CreateDomain(test_fqdn + boost::lexical_cast<std::string>(i) + ".cz", test_registrar_handle, test_registrant_handle)
//                .set_nsset(std::string("different-nsset"))
//                .set_admin_contacts(Util::vector_of<std::string>("different admin"))
//                .exec(ctx);
        }
        //1 with no nsset
        Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                       .make(registrar.handle, contact.handle)
                       .set_admin_contacts(Util::vector_of<std::string>(admin.handle)),
                   ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset, domains_by_nsset_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq =
        impl.get_domains_by_nsset(test_nsset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin();
        it < domain_vec.end();
        ++it)
    {
        found = domain_info.find(it->fqdn);
        BOOST_REQUIRE(it->fqdn == found->second.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it->admin_contact_handles.at(0) ==
            found->second.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->last_transfer.get_value() == ptime(not_a_date_time));
        BOOST_CHECK_EQUAL(it->registered, now_utc);
        BOOST_CHECK(it->registrant_handle == found->second.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->second.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_limit_exceeded, domains_by_nsset_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq =
        impl.get_domains_by_nsset(test_nsset, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains - 1);
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin();
        it < domain_vec.end();
        ++it)
    {
        found = domain_info.find(it->fqdn);
        BOOST_REQUIRE(it->fqdn == found->second.fqdn);
        BOOST_REQUIRE(it->fqdn == found->second.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) ==
            found->second.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.get_value() == ptime(not_a_date_time));
        BOOST_CHECK(it->last_transfer.get_value() == ptime(not_a_date_time));
        BOOST_CHECK_EQUAL(it->registered, now_utc);
        BOOST_CHECK(it->registrant_handle == found->second.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->second.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_absent_nsset, domains_by_nsset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds =
            impl.get_domains_by_nsset("absent-nsset", 0);
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_no_nsset, domains_by_nsset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_nsset("", 0);
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_nsset
BOOST_AUTO_TEST_SUITE_END();//TestWhois
