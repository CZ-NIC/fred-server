#include "tests/interfaces/whois/fixture_common.h"
#include "util/util.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "src/fredlib/object_state/perform_object_state_request.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_admin_contact)

struct domains_by_admin_contact_fixture
: whois_impl_instance_fixture
{
    std::map<std::string, Fred::InfoDomainData> domain_info;
    boost::posix_time::ptime now_utc;
    const unsigned int regular_domains;
    const std::string delete_fqdn;
    std::string regular_handle;

    domains_by_admin_contact_fixture()
    : regular_domains(6),
      delete_fqdn("test-delete.cz")
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar  = Test::registrar::make(ctx);
        const Fred::InfoContactData regular_admin = Test::contact::make(ctx),
                                    system_admin = Test::contact::make(ctx),
                                    contact      = Test::contact::make(ctx);
        regular_handle = regular_admin.handle;
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(
                    ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));

        for(unsigned int i=0; i < regular_domains; ++i)
        {
            const Fred::InfoDomainData& idd = Test::exec(
                    Test::CreateX_factory<Fred::CreateDomain>()
                        .make(registrar.handle, contact.handle)
                        .set_admin_contacts(Util::vector_of<std::string>(regular_admin.handle))
                        .set_nsset(Test::nsset::make(ctx).handle) 
                        .set_keyset(Test::keyset::make(ctx).handle) 
                        .set_expiration_date(
                            boost::gregorian::day_clock::local_day() + boost::gregorian::date_duration(2)),
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
        //delete candidate
        const Fred::InfoDomainData& idd = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                .make(registrar.handle, contact.handle, delete_fqdn)
                .set_admin_contacts(Util::vector_of<std::string>(regular_admin.handle))
                .set_nsset(Test::nsset::make(ctx).handle) 
                .set_keyset(Test::keyset::make(ctx).handle) 
                .set_expiration_date(
                    boost::gregorian::day_clock::local_day() + boost::gregorian::date_duration(2)),
                ctx);

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

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact, domains_by_admin_contact_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_handle, regular_domains);
    BOOST_CHECK(! domain_seq.limit_exceeded);
    BOOST_CHECK(domain_seq.content.size() == regular_domains);

    std::map<std::string, Fred::InfoDomainData>::const_iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::Domain& it, domain_seq.content)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.creating_registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire == found->second.expiration_date);
        BOOST_CHECK(it.keyset == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset  == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() != std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContext ctx;
        const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_limit_exceeded, domains_by_admin_contact_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_handle, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);
    BOOST_CHECK(domain_seq.content.size() == regular_domains - 1);

    std::map<std::string, Fred::InfoDomainData>::const_iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::Domain& it, domain_seq.content)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.creating_registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire == found->second.expiration_date);
        BOOST_CHECK(it.keyset == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset  == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() == std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContext ctx;
        const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_no_contact, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_admin_contact("absent-contact", 1), Registry::WhoisImpl::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_wrong_contact, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_admin_contact("", 1), Registry::WhoisImpl::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_admin_contact
BOOST_AUTO_TEST_SUITE_END();//TestWhois
