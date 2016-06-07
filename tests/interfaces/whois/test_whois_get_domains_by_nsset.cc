#include "tests/interfaces/whois/fixture_common.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_nsset)

struct domains_by_nsset_fixture
: whois_impl_instance_fixture
{
    typedef Registry::WhoisImpl::Domain Domain;
    typedef Registry::WhoisImpl::DomainSeq DomainSeq;

    Fred::OperationContextCreator ctx;
    std::string test_nsset;
    unsigned int regular_domains;
    std::map<std::string, Fred::InfoDomainData> domain_info;
    const Fred::InfoRegistrarData registrar; 
    const Fred::InfoContactData contact, admin;
    const Fred::InfoNssetData nsset, other_nsset;
    const boost::posix_time::ptime now_utc;

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
                     .exec("SELECT now()::timestamp")[0][0])))
    {
        for(unsigned int i=0; i < regular_domains; ++i)
        {
            const Fred::InfoDomainData& idd = Test::exec(
                    Test::CreateX_factory<Fred::CreateDomain>()
                        .make(registrar.handle, contact.handle)
                        .set_nsset(test_nsset)
                        .set_keyset(Test::keyset::make(ctx).handle) 
                        .set_admin_contacts(Util::vector_of<std::string>(
                                Test::contact::make(ctx).handle))
                        .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                             boost::gregorian::date_duration(2)),
                    ctx);
            domain_info[idd.fqdn] = idd;
        }
        for(int i=0; i < 3; ++i)//3 different domains for another nsset
        {
            Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                           .make(registrar.handle, contact.handle)
                           .set_nsset(other_nsset.handle)
                           .set_admin_contacts(
                               Util::vector_of<std::string>(admin.handle)),
                       ctx);
        }
        //1 with no nsset
        Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                       .make(registrar.handle, contact.handle)
                       .set_admin_contacts(
                           Util::vector_of<std::string>(admin.handle)),
                   ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset, domains_by_nsset_fixture)
{
    DomainSeq domain_seq = impl.get_domains_by_nsset(test_nsset,
                                                     regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    BOOST_FOREACH(Domain it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_REQUIRE(it.fqdn     == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() == std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContextCreator ctx;
        const std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                    it.statuses.end());
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_limit_exceeded, domains_by_nsset_fixture)
{
    DomainSeq domain_seq = impl.get_domains_by_nsset(test_nsset,
                                                     regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains - 1);
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    BOOST_FOREACH(Domain it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_REQUIRE(it.fqdn     == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() == std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContextCreator ctx;
        const std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                    it.statuses.end());
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_absent_nsset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_nsset("absent-nsset", 1), Registry::WhoisImpl::ObjectNotExists);
//    try
//    {
//        typedef Registry::WhoisImpl::DomainSeq DomainSeq;
//        DomainSeq ds = impl.get_domains_by_nsset("absent-nsset", 1);
//        BOOST_ERROR("unreported dangling nsset");
//    }
//    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_no_nsset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_nsset("", 1), Registry::WhoisImpl::InvalidHandle);
//    try
//    {
//        typedef Registry::WhoisImpl::DomainSeq DomainSeq;
//        DomainSeq ds = impl.get_domains_by_nsset("", 1);
//        BOOST_ERROR("nsset handle rule is wrong");
//    }
//    catch(const Registry::WhoisImpl::InvalidHandle& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_nsset
BOOST_AUTO_TEST_SUITE_END();//TestWhois
