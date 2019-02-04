#include "test/backend/whois/fixture_common.hh"

#include "src/backend/whois/domain_expiration_datetime.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/domain/transfer_domain.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_keyset)

struct domains_by_keyset_fixture
: whois_impl_instance_fixture
{
    const std::string test_keyset;
    unsigned int regular_domains;
    std::map<std::string, ::LibFred::InfoDomainData> domain_info;
    boost::posix_time::ptime now_utc;
    const std::string delete_fqdn;

    domains_by_keyset_fixture()
    : test_keyset("test-keyset"),
      regular_domains(6), //XXX
      delete_fqdn("test-delete.cz")
    {
        ::LibFred::OperationContextCreator ctx;
        const ::LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const ::LibFred::InfoContactData contact     = Test::contact::make(ctx),
                                    admin       = Test::contact::make(ctx);
        const ::LibFred::InfoKeysetData keyset       = Test::keyset::make(ctx, test_keyset),
                                   other_keyset = Test::keyset::make(ctx);
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
        for(unsigned int i=0; i < regular_domains; ++i)
        {
            const ::LibFred::InfoDomainData& idd = Test::exec(   //lifetime extension of temporary
                    Test::CreateX_factory<::LibFred::CreateDomain>()
                        .make(registrar.handle, contact.handle)
                        .set_nsset(Test::nsset::make(ctx).handle)
                        .set_keyset(test_keyset)
                        .set_admin_contacts(
                            Util::vector_of<std::string>(admin.handle))
                        .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                             boost::gregorian::date_duration(2)),
                    ctx);
            domain_info[idd.fqdn] = idd;
        }
        //enum domain
        regular_domains++;
        const ::LibFred::InfoDomainData& enum_domain = Test::exec(
                Test::CreateX_factory<::LibFred::CreateDomain>()
                    .make(registrar.handle, contact.handle, "7.3.5.7.0.2.4.e164.arpa")
                    .set_admin_contacts(Util::vector_of<std::string>(admin.handle))
                    .set_nsset(Test::nsset::make(ctx).handle)
                    .set_keyset(test_keyset)
                    .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2))
                    .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() +
                                                    boost::gregorian::date_duration(2)),
                ctx);
        domain_info[enum_domain.fqdn] = enum_domain;
        //3 different domains for another keyset
        for(unsigned int i=0; i < 3; ++i)
        {
            Test::exec(
                Test::CreateX_factory<::LibFred::CreateDomain>()
                    .make(registrar.handle, contact.handle)
                    .set_keyset(other_keyset.handle)
                    .set_admin_contacts(Util::vector_of<std::string>(admin.handle)),
                ctx);
        }
        //1 with no keyset
        Test::exec(
            Test::CreateX_factory<::LibFred::CreateDomain>()
                .make(registrar.handle, contact.handle)
                .set_admin_contacts(
                    Util::vector_of<std::string>(admin.handle)),
            ctx);

        //delete candidate
        const ::LibFred::InfoDomainData& idd = Test::exec(
                Test::CreateX_factory<::LibFred::CreateDomain>()
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
        ::LibFred::InfoDomainOutput dom = ::LibFred::InfoDomainByFqdn(delete_fqdn).exec(ctx, "UTC");
        ::LibFred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset, domains_by_keyset_fixture)
{
    Fred::Backend::Whois::DomainSeq domain_seq = impl.get_domains_by_keyset(test_keyset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Fred::Backend::Whois::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::map<std::string, ::LibFred::InfoDomainData>::iterator found;
    BOOST_FOREACH(const Fred::Backend::Whois::Domain& it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.sponsoring_registrar  == found->second.sponsoring_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const ::LibFred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() != std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        ::LibFred::OperationContextCreator ctx;
        const std::vector<::LibFred::ObjectStateData> v_osd =
            ::LibFred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const ::LibFred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                    it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
        if(! found->second.enum_domain_validation.isnull())
        {
            BOOST_CHECK(it.validated_to.get_value() == found->second.enum_domain_validation.get_value().validation_expiration);
            BOOST_CHECK(it.validated_to_time_estimate ==
                    Fred::Backend::Whois::domain_validation_expiration_datetime_estimate(
                        ctx, found->second.enum_domain_validation.get_value_or_default().validation_expiration));
        }
        else
        {
            BOOST_CHECK(it.validated_to.isnull());
            BOOST_CHECK(it.validated_to_time_estimate.isnull());
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_limit_exceeded, domains_by_keyset_fixture)
{
    Fred::Backend::Whois::DomainSeq domain_seq =
        impl.get_domains_by_keyset(test_keyset, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Fred::Backend::Whois::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains - 1);
    std::map<std::string, ::LibFred::InfoDomainData>::iterator found;
    BOOST_FOREACH(const Fred::Backend::Whois::Domain& it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.sponsoring_registrar  == found->second.sponsoring_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const ::LibFred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(std::find(it.admin_contacts.begin(), it.admin_contacts.end(), oit.handle) !=
                    it.admin_contacts.end());
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        ::LibFred::OperationContextCreator ctx;
        const std::vector<::LibFred::ObjectStateData> v_osd =
            ::LibFred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const ::LibFred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                    it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
        if(! found->second.enum_domain_validation.isnull())
        {
            BOOST_CHECK(it.validated_to.get_value() == found->second.enum_domain_validation.get_value().validation_expiration);
            BOOST_CHECK(it.validated_to_time_estimate ==
                    Fred::Backend::Whois::domain_validation_expiration_datetime_estimate(
                        ctx, found->second.enum_domain_validation.get_value_or_default().validation_expiration));
        }
        else
        {
            BOOST_CHECK(it.validated_to.isnull());
            BOOST_CHECK(it.validated_to_time_estimate.isnull());
        }
    }
}

struct update_domains_by_keyset_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    ::LibFred::InfoDomainData domain;
    const std::string test_fqdn;
    const std::string test_keyset;
    std::string transfer_handle;

    update_domains_by_keyset_fixture()
    : test_fqdn("7.3.5.7.0.2.4.e164.arpa"), //ENUM domain covers both enum and usual cases
      test_keyset("test-keyset"),
      transfer_handle("TR REG HANDLE")
    {
        ::LibFred::OperationContextCreator ctx;
        const ::LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx),
                             transfer_registrar = Test::registrar::make(ctx, transfer_handle);
        const ::LibFred::InfoContactData contact     = Test::contact::make(ctx);
        const ::LibFred::InfoKeysetData keyset       = Test::keyset::make(ctx, test_keyset);
        domain = Test::exec(
            Test::CreateX_factory<::LibFred::CreateDomain>()
                .make(registrar.handle,
                      contact.handle,
                      test_fqdn)
                .set_keyset(test_keyset)
                .set_nsset(Test::nsset::make(ctx).handle)
                .set_expiration_date(boost::gregorian::day_clock::local_day() -
                    boost::gregorian::date_duration(2))
                .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() -
                    boost::gregorian::date_duration(2)),
            ctx);
        ::LibFred::UpdateDomain(test_fqdn, registrar.handle)
            .unset_nsset()
            .exec(ctx);
        ::LibFred::TransferDomain(
            ::LibFred::InfoDomainByFqdn(test_fqdn)
                .exec( ctx, "UTC" )
                .info_domain_data
                .id,
            transfer_handle,
            domain.authinfopw,
            0)
            .exec(ctx);
        ::LibFred::InfoDomainOutput dom = ::LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, "UTC");
        ::LibFred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(update_domains_by_keyset, update_domains_by_keyset_fixture)
{
    Fred::Backend::Whois::Domain dom = impl.get_domains_by_keyset(test_keyset, 1).content.at(0);
    BOOST_CHECK(dom.changed == now_utc);
    BOOST_CHECK(dom.last_transfer == now_utc);
    BOOST_CHECK(dom.sponsoring_registrar == transfer_handle);

    ::LibFred::OperationContextCreator ctx;
    BOOST_CHECK(dom.validated_to_time_actual.get_value() ==
            Fred::Backend::Whois::domain_validation_expiration_datetime_actual(ctx, domain.id).get_value());

    Optional<boost::posix_time::ptime> eta = Fred::Backend::Whois::domain_expiration_datetime_actual(ctx, domain.id);
    BOOST_CHECK(dom.expire_time_actual.get_value() == eta.get_value());
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_no_keyset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_keyset("absent-nsset", 1), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_wrong_keyset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_keyset("", 1), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_keyset
BOOST_AUTO_TEST_SUITE_END();//TestWhois
