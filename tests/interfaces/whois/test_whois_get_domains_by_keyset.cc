#include "tests/interfaces/whois/fixture_common.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_keyset)

struct domains_by_keyset_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContextCreator ctx;
    std::string test_keyset;
    unsigned int regular_domains;
    std::map<std::string, Fred::InfoDomainData> domain_info;
    const Fred::InfoRegistrarData registrar; 
    const Fred::InfoContactData contact, admin;
    const Fred::InfoKeysetData keyset, other_keyset;
    const boost::posix_time::ptime now_utc;

    domains_by_keyset_fixture()
    : test_keyset("test-keyset"),
      regular_domains(6),
      registrar(Test::registrar::make(ctx)),
      contact(Test::contact::make(ctx)),
      admin(Test::contact::make(ctx)),
      keyset(Test::keyset::make(ctx, test_keyset)),
      other_keyset(Test::keyset::make(ctx)),
      now_utc(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                      .exec("SELECT now()::timestamp")[0][0])))
    {
        for(unsigned int i=0; i < regular_domains; ++i)
        {
            const Fred::InfoDomainData& idd = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(registrar.handle, contact.handle)
                    .set_nsset(Test::nsset::make(ctx).handle) 
                    .set_keyset(test_keyset)
                    .set_admin_contacts(Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle))
                    .set_admin_contacts(Util::vector_of<std::string>(admin.handle))
                    .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2)),
                ctx);
            domain_info[idd.fqdn] = idd;
        }
        for(unsigned int i=0; i < 3; ++i)//3 different domains for another keyset
        {
            Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                           .make(registrar.handle, contact.handle)
                           .set_keyset(other_keyset.handle)
                           .set_admin_contacts(Util::vector_of<std::string>(admin.handle)),
                       ctx);
        }
        //1 with no keyset
        Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                       .make(registrar.handle, contact.handle)
                       .set_admin_contacts(Util::vector_of<std::string>(admin.handle)),
                   ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset, domains_by_keyset_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq =
        impl.get_domains_by_keyset(test_keyset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    BOOST_FOREACH(Registry::WhoisImpl::Domain it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());//?
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered        == now_utc);
        BOOST_CHECK(it.fqdn              == found->second.fqdn);
        BOOST_CHECK(it.registrant_handle == found->second.registrant.handle);
        BOOST_CHECK(it.registrar_handle  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire            == found->second.expiration_date);
        BOOST_CHECK(it.fqdn              == found->second.fqdn);
        BOOST_CHECK(it.keyset_handle     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset_handle      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair oit, found->second.admin_contacts)
        {
            bool found = (it.admin_contact_handles.end() == std::find(it.admin_contact_handles.begin(),
                        it.admin_contact_handles.end(), oit.handle));
            BOOST_CHECK(!found);//dirty, wasn't working with BOOST_ERROR ;(
        }

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

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_limit_exceeded, domains_by_keyset_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq =
        impl.get_domains_by_keyset(test_keyset, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains - 1);
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    BOOST_FOREACH(Registry::WhoisImpl::Domain it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());//?
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered        == now_utc);
        BOOST_CHECK(it.fqdn              == found->second.fqdn);
        BOOST_CHECK(it.registrant_handle == found->second.registrant.handle);
        BOOST_CHECK(it.registrar_handle  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire            == found->second.expiration_date);
        BOOST_CHECK(it.fqdn              == found->second.fqdn);
        BOOST_CHECK(it.keyset_handle     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset_handle      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair oit, found->second.admin_contacts)
        {
            bool found = (it.admin_contact_handles.end() == std::find(it.admin_contact_handles.begin(),
                        it.admin_contact_handles.end(), oit.handle));
            BOOST_CHECK(!found);//dirty, wasn't working with BOOST_ERROR ;(
        }

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

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_no_keyset, whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds =
            impl.get_domains_by_keyset("absent-nsset", 0);
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_wrong_keyset, whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_keyset("", 0);
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
