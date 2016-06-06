#include "tests/interfaces/whois/fixture_common.h"
#include "util/util.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_admin_contact)

struct domains_by_admin_contact_fixture
: whois_impl_instance_fixture
{
    std::map<std::string, Fred::InfoDomainData> domain_info;
    Fred::InfoContactData regular_admin;
    boost::posix_time::ptime now_utc;
    const unsigned int regular_domains;
    const std::string delete_fqdn;

    domains_by_admin_contact_fixture()
    : regular_domains(6),
      delete_fqdn("test-delete.cz")
    {
        Fred::OperationContextCreator ctx;
        regular_admin                      = Test::contact::make(ctx);
        Fred::InfoRegistrarData registrar  = Test::registrar::make(ctx);
        Fred::InfoContactData system_admin = Test::contact::make(ctx),
                              contact      = Test::contact::make(ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(
                    ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));

        std::string tmp_handle;
        for(unsigned int i=0; i < regular_domains; ++i)
        {
            const Fred::InfoDomainData& idd = Test::exec(
                    Test::CreateX_factory<Fred::CreateDomain>()
                        .make(registrar.handle, contact.handle)
                        .set_admin_contacts(Util::vector_of<std::string>(regular_admin.handle))
                        .set_nsset(Test::nsset::make(ctx).handle) 
                        .set_keyset(Test::keyset::make(ctx).handle) 
                        .set_expiration_date(boost::gregorian::day_clock::local_day() + boost::gregorian::date_duration(2)),
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
//        domain_info[idd.fqdn] = idd;

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
        Fred::InfoDomainOutput dom = Fred::InfoDomainByHandle(delete_fqdn).exec(ctx, impl.output_timezone);
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact, domains_by_admin_contact_fixture)
{
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_admin.handle, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains);
    std::map<std::string, Fred::InfoDomainData>::const_iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::Domain& it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire == found->second.expiration_date);
        BOOST_CHECK(it.keyset == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset  == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() != std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContextCreator ctx;
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
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_admin_contact(regular_admin.handle, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == static_cast<unsigned>(regular_domains - 1));
    std::map<std::string, Fred::InfoDomainData>::iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::Domain& it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.validated_to.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.registrar  == found->second.create_registrar_handle);
        BOOST_CHECK(it.expire == found->second.expiration_date);
        BOOST_CHECK(it.keyset == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset  == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const Fred::ObjectIdHandlePair& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() == std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        Fred::OperationContextCreator ctx;
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
//    try
//    {
//        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_admin_contact("absent-contact", 1);
//        BOOST_ERROR("unreported dangling registrant");
//    }
//    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_admin_contact_wrong_contact, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_admin_contact("", 1), Registry::WhoisImpl::InvalidHandle);
//    try
//    {
//        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_admin_contact("", 1);
//        BOOST_ERROR("registrant handle rule is wrong");
//    }
//    catch(const Registry::WhoisImpl::InvalidHandle& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_admin_contact
BOOST_AUTO_TEST_SUITE_END();//TestWhois
