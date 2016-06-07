#include "tests/interfaces/whois/fixture_common.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "src/whois/zone_list.h"
#include <boost/foreach.hpp>

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domain_by_handle)

struct test_domain_fixture
: whois_impl_instance_fixture
{
    Fred::InfoDomainData domain;
    boost::posix_time::ptime now_utc;

    test_domain_fixture()
    {
        Fred::OperationContextCreator ctx;
        domain = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(Test::registrar::make(ctx).handle,
                          Test::contact::make(ctx).handle)
                    .set_nsset(Test::nsset::make(ctx).handle) 
                    .set_keyset(Test::keyset::make(ctx).handle) 
                    .set_admin_contacts(Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle))
                    .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2)),
                ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(regular_case, test_domain_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(domain.fqdn);

    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.validated_to.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.registered == now_utc);
    BOOST_CHECK(dom.fqdn       == domain.fqdn);
    BOOST_CHECK(dom.registrant == domain.registrant.handle);
    BOOST_CHECK(dom.registrar  == domain.create_registrar_handle);
    BOOST_CHECK(dom.expire     == domain.expiration_date);
    BOOST_CHECK(dom.fqdn       == domain.fqdn);
    BOOST_CHECK(dom.keyset     == domain.keyset.get_value_or_default().handle);
    BOOST_CHECK(dom.nsset      == domain.nsset.get_value_or_default().handle);

    BOOST_FOREACH(const Fred::ObjectIdHandlePair& it, domain.admin_contacts)
    {
        BOOST_CHECK(std::find(dom.admin_contacts.begin(), dom.admin_contacts.end(), it.handle) !=
                        dom.admin_contacts.end());
    }
    BOOST_CHECK(domain.admin_contacts.size() == dom.admin_contacts.size());

    Fred::OperationContextCreator ctx;
    const std::vector<Fred::ObjectStateData> v_osd =
        Fred::GetObjectStates(domain.id).exec(ctx);
    BOOST_FOREACH(const Fred::ObjectStateData it, v_osd)
    {
        BOOST_CHECK(std::find(dom.statuses.begin(), dom.statuses.end(), it.state_name) !=
                dom.statuses.end());
    }
}

BOOST_FIXTURE_TEST_CASE(wrong_handle, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domain_by_handle(""), Registry::WhoisImpl::InvalidLabel);
}

struct wrong_zone_fixture
: whois_impl_instance_fixture
{
    wrong_zone_fixture()
    {
        std::vector<std::string> list = impl.get_managed_zone_list();
        BOOST_REQUIRE(list.end() ==
            std::find(list.begin(), list.end(), std::string("aaa")));
    }
};

BOOST_FIXTURE_TEST_CASE(wrong_zone, wrong_zone_fixture)
{
    BOOST_CHECK_THROW(impl.get_domain_by_handle("aaa"), Registry::WhoisImpl::UnmanagedZone);
//    try
//    {
//        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle("aaa");
//        BOOST_ERROR("unreported managed zone");
//    }
//    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

struct many_labels_fixture
: whois_impl_instance_fixture
{
    std::vector<std::string> domain_list;
    Fred::OperationContextCreator ctx;

    std::string prepare_zone(Fred::OperationContextCreator& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::get_zone(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone;
        for(unsigned int i=0; i < zone_data.dots_max + 2; ++i) // !!!
        {
            labeled_zone << "1.";
        }
        labeled_zone << zone_data.name;
        return labeled_zone.str();
    }

    many_labels_fixture()
    {
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        BOOST_FOREACH(std::string it, zone_seq)
        {
            domain_list.push_back(prepare_zone(ctx, it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(too_many_labels, many_labels_fixture)
{
    BOOST_FOREACH(const std::string& it, domain_list)
    {
        BOOST_CHECK_THROW(impl.get_domain_by_handle(it), Registry::WhoisImpl::TooManyLabels);
//        try
//        {
//            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(it);
//            BOOST_ERROR("permitted label number is wrong");
//        }
//        catch(const Registry::WhoisImpl::TooManyLabels& ex)
//        {
//            BOOST_CHECK(true);
//            BOOST_MESSAGE(boost::diagnostic_information(ex));
//        }
    }
}

BOOST_FIXTURE_TEST_CASE(no_handle, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domain_by_handle("fine-handle.cz"), Registry::WhoisImpl::ObjectNotExists);
//    try
//    {
//        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle("fine-handle.cz");
//        BOOST_ERROR("unreported dangling domain");
//    }
//    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_FIXTURE_TEST_CASE(invalid_handle, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domain_by_handle("a-.cz"), Registry::WhoisImpl::InvalidLabel);
//    try
//    {
//        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle("a-.cz");
//        BOOST_ERROR("domain checker rule is wrong");
//    }
//    catch(const Registry::WhoisImpl::InvalidLabel& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

struct invalid_unmanaged_fixture
: whois_impl_instance_fixture
{
    std::string invalid_unmanaged_fqdn;

    invalid_unmanaged_fixture()
    {
        std::ostringstream prefix;
        for(unsigned int i=0; i < 256; ++i)//exceed the size of valid label
        {
            prefix << "1";//invalid part
        }
        prefix << '.' << "aaa";//unmanaged part / !!!
        invalid_unmanaged_fqdn = prefix.str();
    }
};

BOOST_FIXTURE_TEST_CASE(invalid_handle_unmanaged_zone, invalid_unmanaged_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom =
            impl.get_domain_by_handle(invalid_unmanaged_fqdn);
        BOOST_ERROR("domain must have invalid label and unmanaged zone");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_ERROR("domain must check name validity first");
    }
}

struct unmanaged_toomany_fixture
: whois_impl_instance_fixture
{
    std::string unmanaged_toomany_fqdn;

    unmanaged_toomany_fixture()
    {
        std::ostringstream prefix;
        for(unsigned int i=0; i < 20; ++i) // !!!
        {
            prefix << "1.";//toomany part
        }
        prefix << "aaa"; //unmanaged zone part / !!!
        unmanaged_toomany_fqdn = prefix.str();
    }
};

BOOST_FIXTURE_TEST_CASE(unmanaged_zone_too_many_labels, unmanaged_toomany_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom =
            impl.get_domain_by_handle(unmanaged_toomany_fqdn);
        BOOST_ERROR("domain must have unmanaged zone and exceeded number of labels");
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
    catch(const Registry::WhoisImpl::TooManyLabels& ex)
    {
        BOOST_ERROR("domain must check managed zone first");
    }
}

struct invalid_toomany_fixture
: whois_impl_instance_fixture
{
    std::vector<std::string> domain_list;

    std::string prepare_zone(Fred::OperationContextCreator& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::get_zone(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone, invalid_offset;
        for(unsigned int i=0; i < 256; ++i) // !!!
        {
            labeled_zone << "1.";// invalid + toomany part
        }
        labeled_zone << zone_data.name;
        return labeled_zone.str();
    }

    invalid_toomany_fixture()
    {
        Fred::OperationContextCreator ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        domain_list.reserve(zone_seq.size());
        BOOST_FOREACH(std::string it, zone_seq)
        {
            domain_list.push_back(prepare_zone(ctx, it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(invalid_handle_too_many_labels, invalid_toomany_fixture)
{
    BOOST_FOREACH(std::string it, domain_list)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(it);
            BOOST_ERROR("domain must have invalid handle and exceeded number of labels");
        }
        catch(const Registry::WhoisImpl::InvalidLabel& ex)
        {
            BOOST_CHECK(true);
            BOOST_MESSAGE(boost::diagnostic_information(ex));
        }
        catch(const Registry::WhoisImpl::TooManyLabels& ex)
        {
            BOOST_ERROR("domain must check name validity first");
        }
    }
}

struct invalid_unmanaged_toomany_fixture
: whois_impl_instance_fixture
{
    std::string invalid_unmanaged_toomany_fqdn;

    invalid_unmanaged_toomany_fixture()
    {
        std::ostringstream prefix;
        for(unsigned int i=0; i < 256; ++i)
        {
            prefix << "1."; // invalid + toomany part
        }
        prefix << "aaa"; //unmanaged zone part / !!!
        invalid_unmanaged_toomany_fqdn = prefix.str();
    }
};

BOOST_FIXTURE_TEST_CASE(invalid_unmanaged_toomany, invalid_unmanaged_toomany_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom =
            impl.get_domain_by_handle(invalid_unmanaged_toomany_fqdn);
        BOOST_ERROR("domain must have invalid handle, "
                    "unmanaged zone and exceeded number of labels");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_ERROR("domain must check name validity first");
    }
    catch(const Registry::WhoisImpl::TooManyLabels& ex)
    {
        BOOST_ERROR("domain must check name validity first, then managed zone");
    }
}

struct delete_candidate_fixture 
: whois_impl_instance_fixture
{
    std::string delete_fqdn;

    delete_candidate_fixture()
    : delete_fqdn("test-delete.cz")
    {
        Fred::OperationContextCreator ctx;

        Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                       .make(Test::registrar(ctx).info_data.handle,
                             Test::contact(ctx).info_data.handle,
                             delete_fqdn)
                       .set_admin_contacts(Util::vector_of<std::string>(
                           Test::contact::make(ctx).handle)),
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

        Fred::InfoDomainOutput dom =
            Fred::InfoDomainByHandle(delete_fqdn).exec(ctx, impl.output_timezone);

        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(delete_candidate, delete_candidate_fixture)
{
    Fred::OperationContextCreator ctx;
    Fred::InfoDomainData idd = Fred::InfoDomainByHandle(delete_fqdn)
        .exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_domain_data;
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(delete_fqdn);
    BOOST_CHECK(dom.fqdn == idd.fqdn);
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.statuses.end() !=
            std::find(dom.statuses.begin(), dom.statuses.end(), "deleteCandidate"));
}

BOOST_AUTO_TEST_SUITE_END()//get_domain_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
