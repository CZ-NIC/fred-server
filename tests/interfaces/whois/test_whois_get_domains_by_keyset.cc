#include "tests/interfaces/whois/fixture_common.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_keyset)

struct domains_by_keyset_fixture
: whois_impl_instance_fixture
{
    const std::string test_keyset;
    const unsigned int regular_domains;
    std::map<std::string, Fred::InfoDomainData> domain_info;
    boost::posix_time::ptime now_utc;
    const std::string delete_fqdn;

    domains_by_keyset_fixture()
    : test_keyset("test-keyset"),
      regular_domains(6), //XXX
      delete_fqdn("test-delete.cz")
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const Fred::InfoContactData contact     = Test::contact::make(ctx),
                                    admin       = Test::contact::make(ctx);
        const Fred::InfoKeysetData keyset       = Test::keyset::make(ctx, test_keyset),
                                   other_keyset = Test::keyset::make(ctx);
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
        for(unsigned int i=0; i < regular_domains; ++i)
        {
            const Fred::InfoDomainData& idd = Test::exec(   //lifetime extension of temporary
                    Test::CreateX_factory<Fred::CreateDomain>()
                        .make(registrar.handle, contact.handle)
                        .set_nsset(Test::nsset::make(ctx).handle) 
                        .set_keyset(test_keyset)
                        .set_admin_contacts(
                            Util::vector_of<std::string>(
                                Test::contact::make(ctx).handle))
                        .set_admin_contacts(
                            Util::vector_of<std::string>(admin.handle))
                        .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                             boost::gregorian::date_duration(2)),
                    ctx);
            domain_info[idd.fqdn] = idd;
        }
        for(unsigned int i=0; i < 3; ++i) //3 different domains for another keyset
        {
            Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(registrar.handle, contact.handle)
                    .set_keyset(other_keyset.handle)
                    .set_admin_contacts(Util::vector_of<std::string>(admin.handle)),
                ctx);
        }
        //1 with no keyset
        Test::exec(
            Test::CreateX_factory<Fred::CreateDomain>()
                .make(registrar.handle, contact.handle)
                .set_admin_contacts(
                    Util::vector_of<std::string>(admin.handle)),
            ctx);
        
        //delete candidate
        const Fred::InfoDomainData& idd = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(registrar.handle, contact.handle, delete_fqdn)
                    .set_admin_contacts(Util::vector_of<std::string>(Util::vector_of<std::string>(admin.handle)))
                    .set_nsset(Test::nsset::make(ctx).handle)
                    .set_keyset(test_keyset)
                    .set_expiration_date(
                        boost::gregorian::day_clock::local_day() + boost::gregorian::date_duration(2)),
                ctx);
        domain_info[idd.fqdn] = idd;
        ctx.get_conn().exec_params(
                "UPDATE domain_history "
                "SET exdate = now() - "
                "(SELECT val::int * '1 day'::interval "
                "FROM enum_parameters "
                "WHERE name = 'expiration_registration_protection_period') "
                "WHERE id = "
                "(SELECT id "
                "FROM object_registry "
                "WHERE name = $1::text)",
                Database::query_param_list(delete_fqdn));
        ctx.get_conn().exec_params(
                "UPDATE domain "
                "SET exdate = now() - "
                "(SELECT val::int * '1 day'::interval "
                "FROM enum_parameters "
                "WHERE name = 'expiration_registration_protection_period') "
                "WHERE id = "
                "(SELECT id "
                "FROM object_registry "
                "WHERE name = $1::text)",
                Database::query_param_list(delete_fqdn));
        Fred::InfoDomainOutput dom = Fred::InfoDomainByHandle(delete_fqdn).exec(ctx, "UTC");
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset, domains_by_keyset_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_keyset(test_keyset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::Domain& it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        if (it.fqdn == delete_fqdn)
        {
            BOOST_CHECK(it.statuses.size()    == 1);
            BOOST_CHECK(it.statuses.at(0)     == "deleteCandidate");
            BOOST_CHECK(it.registered         == boost::posix_time::ptime(not_a_date_time));
            BOOST_CHECK(it.registrant         == "");
            BOOST_CHECK(it.creating_registrar == "");
            BOOST_CHECK(it.expire             == boost::gregorian::date(not_a_date_time));
            BOOST_CHECK(it.keyset             == "");
            BOOST_CHECK(it.nsset              == "");
            BOOST_CHECK(it.admin_contacts.empty());
            continue;
        }
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.creating_registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() != std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContextCreator ctx;
        const std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                    it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
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
    BOOST_FOREACH(const Registry::WhoisImpl::Domain& it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        if (it.fqdn == delete_fqdn)
        {
            BOOST_CHECK(it.statuses.size()    == 1);
            BOOST_CHECK(it.statuses.at(0)     == "deleteCandidate");
            BOOST_CHECK(it.registered         == boost::posix_time::ptime(not_a_date_time));
            BOOST_CHECK(it.registrant         == "");
            BOOST_CHECK(it.creating_registrar == "");
            BOOST_CHECK(it.expire             == boost::gregorian::date(not_a_date_time));
            BOOST_CHECK(it.keyset             == "");
            BOOST_CHECK(it.nsset              == "");
            BOOST_CHECK(it.admin_contacts.empty());
            continue;
        }
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.creating_registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(std::find(it.admin_contacts.begin(), it.admin_contacts.end(), oit.handle) != 
                    it.admin_contacts.end());
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContextCreator ctx;
        const std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                    it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_no_keyset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_keyset("absent-nsset", 1), Registry::WhoisImpl::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_wrong_keyset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_keyset("", 1), Registry::WhoisImpl::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_nsset
BOOST_AUTO_TEST_SUITE_END();//TestWhois
