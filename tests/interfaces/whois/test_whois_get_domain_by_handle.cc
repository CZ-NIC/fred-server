#include "tests/interfaces/whois/fixture_common.h"

#include "src/whois/zone_list.h"
#include "src/whois/domain_expiration_datetime.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/domain/transfer_domain.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/foreach.hpp>

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domain_by_handle)

struct plain_domain_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    Fred::InfoDomainData domain;
    const std::string test_fqdn;

    plain_domain_fixture()
    : test_fqdn("7.3.5.7.0.2.4.e164.arpa") //ENUM domain covers both enum and usual cases
    {
        Fred::OperationContextCreator ctx;
        domain = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(Test::registrar::make(ctx).handle,
                          Test::contact::make(ctx).handle,
                          test_fqdn)
                    .set_nsset(Test::nsset::make(ctx).handle)
                    .set_keyset(Test::keyset::make(ctx).handle)
                    .set_admin_contacts(
                        Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle))
                    .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2))
                    .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2)),
                ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(regular_case, plain_domain_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(domain.fqdn);

    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.validated_to.get_value() == domain.enum_domain_validation.get_value().validation_expiration);
    BOOST_CHECK(dom.fqdn                     == domain.fqdn);
    BOOST_CHECK(dom.registered               == now_utc);
    BOOST_CHECK(dom.registrant               == domain.registrant.handle);
    BOOST_CHECK(dom.sponsoring_registrar     == domain.sponsoring_registrar_handle);
    BOOST_CHECK(dom.expire                   == domain.expiration_date);
    BOOST_CHECK(dom.fqdn                     == domain.fqdn);
    BOOST_CHECK(dom.keyset                   == domain.keyset.get_value().handle);
    BOOST_CHECK(dom.nsset                    == domain.nsset.get_value().handle);

    BOOST_FOREACH(const Fred::ObjectIdHandlePair& it, domain.admin_contacts)
    {
        BOOST_CHECK(std::find(dom.admin_contacts.begin(), dom.admin_contacts.end(), it.handle) !=
                        dom.admin_contacts.end());
    }
    BOOST_CHECK(domain.admin_contacts.size() == dom.admin_contacts.size());

    Fred::OperationContextCreator ctx;
    const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(domain.id).exec(ctx);
    BOOST_FOREACH(const Fred::ObjectStateData& it, v_osd)
    {
        BOOST_CHECK(std::find(dom.statuses.begin(), dom.statuses.end(), it.state_name) !=
                        dom.statuses.end());
    }
    BOOST_CHECK(v_osd.size() == dom.statuses.size());

    BOOST_CHECK(dom.validated_to_time_estimate ==
            ::Whois::domain_validation_expiration_datetime_estimate(
                ctx, domain.enum_domain_validation.get_value_or_default().validation_expiration));
    BOOST_CHECK(dom.validated_to_time_actual.isnull());

    BOOST_CHECK(dom.expire_time_estimate == ::Whois::domain_expiration_datetime_estimate(ctx, domain.expiration_date));
    BOOST_CHECK(dom.expire_time_actual.isnull());
}


struct cz_domain_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    Fred::InfoDomainData domain;
    const std::string test_fqdn;

    cz_domain_fixture()
    : test_fqdn("prvnipad-vodopad.cz")
    {
        Fred::OperationContextCreator ctx;
        domain = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(Test::registrar::make(ctx).handle,
                          Test::contact::make(ctx).handle,
                          test_fqdn)
                    .set_nsset(Test::nsset::make(ctx).handle)
                    .set_keyset(Test::keyset::make(ctx).handle)
                    .set_admin_contacts(
                        Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle)),
                ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(root_dot_query_case, cz_domain_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(domain.fqdn + ".");

    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.validated_to.isnull());
    BOOST_CHECK(dom.fqdn                     == domain.fqdn);
    BOOST_CHECK(dom.registered               == now_utc);
    BOOST_CHECK(dom.registrant               == domain.registrant.handle);
    BOOST_CHECK(dom.sponsoring_registrar     == domain.sponsoring_registrar_handle);
    BOOST_CHECK(dom.expire                   == domain.expiration_date);
    BOOST_CHECK(dom.fqdn                     == domain.fqdn);
    BOOST_CHECK(dom.keyset                   == domain.keyset.get_value().handle);
    BOOST_CHECK(dom.nsset                    == domain.nsset.get_value().handle);

    BOOST_FOREACH(const Fred::ObjectIdHandlePair& it, domain.admin_contacts)
    {
        BOOST_CHECK(std::find(dom.admin_contacts.begin(), dom.admin_contacts.end(), it.handle) !=
                        dom.admin_contacts.end());
    }
    BOOST_CHECK(domain.admin_contacts.size() == dom.admin_contacts.size());

    Fred::OperationContextCreator ctx;
    const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(domain.id).exec(ctx);
    BOOST_FOREACH(const Fred::ObjectStateData& it, v_osd)
    {
        BOOST_CHECK(std::find(dom.statuses.begin(), dom.statuses.end(), it.state_name) !=
                        dom.statuses.end());
    }
    BOOST_CHECK(v_osd.size() == dom.statuses.size());

    BOOST_CHECK(dom.validated_to_time_estimate.isnull());
    BOOST_CHECK(dom.validated_to_time_actual.isnull());

    BOOST_CHECK(dom.expire_time_estimate == ::Whois::domain_expiration_datetime_estimate(ctx, domain.expiration_date));
    BOOST_CHECK(dom.expire_time_actual.isnull());
}


struct update_domain_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    Fred::InfoDomainData domain;
    const std::string test_fqdn;
    std::string transfer_handle;

    update_domain_fixture()
    : test_fqdn("7.3.5.7.0.2.4.e164.arpa"), //ENUM domain covers both enum and usual cases
      transfer_handle("TR REG HANDLE")
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx),
                             transfer_registrar = Test::registrar::make(ctx, transfer_handle);
        domain = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(registrar.handle,
                          Test::contact::make(ctx).handle,
                          test_fqdn)
                    .set_nsset(Test::nsset::make(ctx).handle)
                    .set_expiration_date(boost::gregorian::day_clock::local_day() -
                                         boost::gregorian::date_duration(2))
                    .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() -
                                         boost::gregorian::date_duration(2)),
                ctx);
        Fred::UpdateDomain(test_fqdn, registrar.handle)
            .unset_nsset()
            .exec(ctx);
        Fred::TransferDomain(
            Fred::InfoDomainByFqdn(test_fqdn)
                .exec( ctx, "UTC" )
                .info_domain_data
                .id,
            transfer_handle,
            domain.authinfopw,
            0)
            .exec(ctx);
        Fred::InfoDomainOutput dom = Fred::InfoDomainByFqdn(test_fqdn).exec(ctx, "UTC");
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(update_case, update_domain_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(domain.fqdn);

    BOOST_CHECK(dom.changed == now_utc);
    BOOST_CHECK(dom.last_transfer == now_utc);
    BOOST_CHECK(dom.sponsoring_registrar == transfer_handle);

    Fred::OperationContextCreator ctx;
    BOOST_CHECK(dom.validated_to_time_actual.get_value() ==
            ::Whois::domain_validation_expiration_datetime_actual(ctx, domain.id).get_value());

    Optional<boost::posix_time::ptime> eta = ::Whois::domain_expiration_datetime_actual(ctx, domain.id);
    BOOST_CHECK(dom.expire_time_actual.get_value() == eta.get_value());
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
        BOOST_REQUIRE(list.end() == std::find(list.begin(), list.end(), std::string("aaa")));
    }
};

BOOST_FIXTURE_TEST_CASE(wrong_zone, wrong_zone_fixture)
{
    BOOST_CHECK_THROW(impl.get_domain_by_handle("aaa"), Registry::WhoisImpl::UnmanagedZone);
}

struct many_labels_fixture
: whois_impl_instance_fixture
{
    std::vector<std::string> domain_list;
    Fred::OperationContextCreator ctx;

    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
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
        std::string labeled_zone;
        unsigned int dots_exceeded = zone_data.dots_max + 2;
        labeled_zone.reserve(dots_exceeded * 2 + zone_data.name.size());
        for(unsigned int i=0; i < dots_exceeded; ++i) // XXX
        {
            labeled_zone += "1.";
        }
        labeled_zone += zone_data.name;
        return labeled_zone;
    }

    many_labels_fixture()
    {
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        BOOST_FOREACH(const std::string& it, zone_seq)
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
    }
}

BOOST_FIXTURE_TEST_CASE(no_handle, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domain_by_handle("fine-handle.cz"), Registry::WhoisImpl::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(invalid_handle, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domain_by_handle("a-.cz"), Registry::WhoisImpl::InvalidLabel);
}

struct invalid_unmanaged_fixture
: whois_impl_instance_fixture
{
    std::string invalid_unmanaged_fqdn;

    invalid_unmanaged_fixture()
    {
        std::string invalid_unmanaged_fqdn;
        invalid_unmanaged_fqdn = std::string(256, '1'); //invalid part XXX
        invalid_unmanaged_fqdn += ".aaa"; //unmanaged part XXX
    }
};

BOOST_FIXTURE_TEST_CASE(invalid_handle_unmanaged_zone, invalid_unmanaged_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(invalid_unmanaged_fqdn);
        BOOST_ERROR("domain must have invalid label and unmanaged zone");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
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
        unsigned int labels_exceeded = 20;// XXX
        unmanaged_toomany_fqdn.reserve(labels_exceeded * 2 + 3); //XXX

        for(unsigned int i=0; i < labels_exceeded; ++i)
        {
            unmanaged_toomany_fqdn += "1."; //toomany part
        }
        unmanaged_toomany_fqdn += "aaa";  //unmanaged zone part / XXX
    }
};

BOOST_FIXTURE_TEST_CASE(unmanaged_zone_too_many_labels, unmanaged_toomany_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(unmanaged_toomany_fqdn);
        BOOST_ERROR("domain must have unmanaged zone and exceeded number of labels");
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
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

    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
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
        std::string labeled_zone;
        const unsigned int labels_dots_exceeded = 256;
        labeled_zone.reserve(labels_dots_exceeded * 2 + zone_data.name.size());
        for(unsigned int i=0; i < labels_dots_exceeded; ++i) // XXX
        {
            labeled_zone += "1."; // invalid + toomany part
        }
        labeled_zone += zone_data.name;
        return labeled_zone;
    }

    invalid_toomany_fixture()
    {
        Fred::OperationContextCreator ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        domain_list.reserve(zone_seq.size());
        BOOST_FOREACH(const std::string& it, zone_seq)
        {
            domain_list.push_back(prepare_zone(ctx, it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(invalid_handle_too_many_labels, invalid_toomany_fixture)
{
    BOOST_FOREACH(const std::string& it, domain_list)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(it);
            BOOST_ERROR("domain must have invalid handle and exceeded number of labels");
        }
        catch(const Registry::WhoisImpl::InvalidLabel& ex)
        {
            BOOST_CHECK(true);
            BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
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
        const unsigned int labels_dots_exceeded = 256;
        invalid_unmanaged_toomany_fqdn.reserve(labels_dots_exceeded * 2 + 3);
        for(unsigned int i=0; i < labels_dots_exceeded; ++i)
        {
            invalid_unmanaged_toomany_fqdn += "1."; // invalid + toomany part
        }
        invalid_unmanaged_toomany_fqdn += "aaa"; //unmanaged zone part / XXX
    }
};

BOOST_FIXTURE_TEST_CASE(invalid_unmanaged_toomany, invalid_unmanaged_toomany_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(invalid_unmanaged_toomany_fqdn);
        BOOST_ERROR("domain must have invalid handle, "
                    "unmanaged zone and exceeded number of labels");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
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

        Test::exec(
            Test::CreateX_factory<Fred::CreateDomain>()
                .make(Test::registrar(ctx).info_data.handle,
                      Test::contact(ctx).info_data.handle,
                      delete_fqdn)
                .set_nsset(Test::nsset::make(ctx).handle)
                .set_keyset(Test::keyset::make(ctx).handle)
                .set_admin_contacts(
                    Util::vector_of<std::string>(
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

        Fred::InfoDomainOutput dom = Fred::InfoDomainByFqdn(delete_fqdn).exec(ctx, "UTC");
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(delete_candidate, delete_candidate_fixture)
{
    Fred::OperationContextCreator ctx;
    Fred::InfoDomainData idd = Fred::InfoDomainByFqdn(delete_fqdn).exec(ctx, "UTC").info_domain_data;
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(delete_fqdn);
    BOOST_CHECK(dom.fqdn == idd.fqdn);
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.validated_to.isnull());
    BOOST_CHECK(dom.statuses.size()      == 1);
    BOOST_CHECK(dom.statuses.at(0)       == "deleteCandidate");
    BOOST_CHECK(dom.registered           == boost::posix_time::ptime(not_a_date_time));
    BOOST_CHECK(dom.registrant           == "");
    BOOST_CHECK(dom.sponsoring_registrar == "");
    BOOST_CHECK(dom.expire               == boost::gregorian::date(not_a_date_time));
    BOOST_CHECK(dom.keyset               == "");
    BOOST_CHECK(dom.nsset                == "");
    BOOST_CHECK(dom.admin_contacts.empty());

}

struct child_parent_fixture
: whois_impl_instance_fixture
{
    const std::string child, parent;
    Fred::InfoDomainData parent_obj;
    boost::posix_time::ptime now_utc;

    child_parent_fixture()
    : child("1.1.1.1.1.7.0.2.4.e164.arpa"),
      parent("7.0.2.4.e164.arpa")
    {
        Fred::OperationContextCreator ctx;
        parent_obj = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(Test::registrar::make(ctx).handle,
                          Test::contact::make(ctx).handle,
                          parent)
                    .set_nsset(Test::nsset::make(ctx).handle)
                    .set_keyset(Test::keyset::make(ctx).handle)
                    .set_admin_contacts(
                        Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle))
                    .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2))
                    .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2)),
                ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(child_parent, child_parent_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(child);

    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.validated_to.get_value() == parent_obj.enum_domain_validation.get_value().validation_expiration);
    BOOST_CHECK(dom.fqdn                     == parent_obj.fqdn);
    BOOST_CHECK(dom.registered               == now_utc);
    BOOST_CHECK(dom.registrant               == parent_obj.registrant.handle);
    BOOST_CHECK(dom.sponsoring_registrar     == parent_obj.sponsoring_registrar_handle);
    BOOST_CHECK(dom.expire                   == parent_obj.expiration_date);
    BOOST_CHECK(dom.fqdn                     == parent_obj.fqdn);
    BOOST_CHECK(dom.keyset                   == parent_obj.keyset.get_value().handle);
    BOOST_CHECK(dom.nsset                    == parent_obj.nsset.get_value().handle);

    BOOST_FOREACH(const Fred::ObjectIdHandlePair& it, parent_obj.admin_contacts)
    {
        BOOST_CHECK(std::find(dom.admin_contacts.begin(), dom.admin_contacts.end(), it.handle) !=
                        dom.admin_contacts.end());
    }
    BOOST_CHECK(parent_obj.admin_contacts.size() == dom.admin_contacts.size());

    Fred::OperationContextCreator ctx;
    const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(parent_obj.id).exec(ctx);
    BOOST_FOREACH(const Fred::ObjectStateData& it, v_osd)
    {
        BOOST_CHECK(std::find(dom.statuses.begin(), dom.statuses.end(), it.state_name) !=
                        dom.statuses.end());
    }
    BOOST_CHECK(v_osd.size() == dom.statuses.size());

    BOOST_CHECK(dom.validated_to_time_estimate ==
            ::Whois::domain_validation_expiration_datetime_estimate(
                ctx, parent_obj.enum_domain_validation.get_value_or_default().validation_expiration));
    BOOST_CHECK(dom.validated_to_time_actual.isnull());

    BOOST_CHECK(dom.expire_time_estimate == ::Whois::domain_expiration_datetime_estimate(ctx, parent_obj.expiration_date));
    BOOST_CHECK(dom.expire_time_actual.isnull());
}

struct parent_child_fixture
: whois_impl_instance_fixture
{
    const std::string child, parent;
    Fred::InfoDomainData child_obj;
    boost::posix_time::ptime now_utc;

    parent_child_fixture()
    : child("1.1.1.1.1.7.0.2.4.e164.arpa"),
      parent("7.0.2.4.e164.arpa")
    {
        Fred::OperationContextCreator ctx;
        child_obj = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(Test::registrar::make(ctx).handle,
                          Test::contact::make(ctx).handle,
                          child)
                    .set_nsset(Test::nsset::make(ctx).handle)
                    .set_keyset(Test::keyset::make(ctx).handle)
                    .set_admin_contacts(
                        Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle))
                    .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2))
                    .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2)),
                ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(parent_child, parent_child_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(parent);

    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.validated_to.get_value() == child_obj.enum_domain_validation.get_value().validation_expiration);
    BOOST_CHECK(dom.fqdn                     == child_obj.fqdn);
    BOOST_CHECK(dom.registered               == now_utc);
    BOOST_CHECK(dom.registrant               == child_obj.registrant.handle);
    BOOST_CHECK(dom.sponsoring_registrar     == child_obj.sponsoring_registrar_handle);
    BOOST_CHECK(dom.expire                   == child_obj.expiration_date);
    BOOST_CHECK(dom.fqdn                     == child_obj.fqdn);
    BOOST_CHECK(dom.keyset                   == child_obj.keyset.get_value().handle);
    BOOST_CHECK(dom.nsset                    == child_obj.nsset.get_value().handle);

    BOOST_FOREACH(const Fred::ObjectIdHandlePair& it, child_obj.admin_contacts)
    {
        BOOST_CHECK(std::find(dom.admin_contacts.begin(), dom.admin_contacts.end(), it.handle) !=
                        dom.admin_contacts.end());
    }
    BOOST_CHECK(child_obj.admin_contacts.size() == dom.admin_contacts.size());

    Fred::OperationContextCreator ctx;
    const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(child_obj.id).exec(ctx);
    BOOST_FOREACH(const Fred::ObjectStateData& it, v_osd)
    {
        BOOST_CHECK(std::find(dom.statuses.begin(), dom.statuses.end(), it.state_name) !=
                        dom.statuses.end());
    }
    BOOST_CHECK(v_osd.size() == dom.statuses.size());

    BOOST_CHECK(dom.validated_to_time_estimate ==
            ::Whois::domain_validation_expiration_datetime_estimate(
                ctx, child_obj.enum_domain_validation.get_value_or_default().validation_expiration));
    BOOST_CHECK(dom.validated_to_time_actual.isnull());

    BOOST_CHECK(dom.expire_time_estimate == ::Whois::domain_expiration_datetime_estimate(ctx, child_obj.expiration_date));
    BOOST_CHECK(dom.expire_time_actual.isnull());
}

struct del_can_child_parent_fixture
: whois_impl_instance_fixture
{
    std::string child, parent;

    del_can_child_parent_fixture()
    : child("1.1.1.1.1.7.0.2.4.e164.arpa"),
      parent("7.0.2.4.e164.arpa")
    {
        Fred::OperationContextCreator ctx;

        Test::exec(
            Test::CreateX_factory<Fred::CreateDomain>()
                .make(Test::registrar(ctx).info_data.handle,
                      Test::contact(ctx).info_data.handle,
                      parent)
                .set_nsset(Test::nsset::make(ctx).handle)
                .set_keyset(Test::keyset::make(ctx).handle)
                .set_admin_contacts(
                    Util::vector_of<std::string>(
                        Test::contact::make(ctx).handle))
                .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                     boost::gregorian::date_duration(2))
                .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() +
                                     boost::gregorian::date_duration(2)),
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
            Database::query_param_list(parent));
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
            Database::query_param_list(parent));

        Fred::InfoDomainOutput dom = Fred::InfoDomainByFqdn(parent).exec(ctx, "UTC");
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(del_can_child_parent, del_can_child_parent_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(child);

    BOOST_CHECK(dom.fqdn == parent);
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.validated_to.isnull());
    BOOST_CHECK(dom.admin_contacts.empty());
    BOOST_CHECK(dom.statuses.size() == 1);
    BOOST_CHECK(dom.statuses.at(0)  == "deleteCandidate");
    BOOST_CHECK(dom.registered      == boost::posix_time::ptime(not_a_date_time));
    BOOST_CHECK(dom.registrant      == "");
    BOOST_CHECK(dom.expire          == boost::gregorian::date(not_a_date_time));
    BOOST_CHECK(dom.keyset          == "");
    BOOST_CHECK(dom.nsset           == "");
    BOOST_CHECK(dom.sponsoring_registrar == "");
    BOOST_CHECK(dom.expire_time_estimate == boost::posix_time::ptime(not_a_date_time));
    BOOST_CHECK(dom.expire_time_actual.isnull());
    BOOST_CHECK(dom.validated_to_time_estimate.isnull());
    BOOST_CHECK(dom.validated_to_time_actual.isnull());
}

struct del_can_parent_child_fixture
: whois_impl_instance_fixture
{
    std::string child, parent;

    del_can_parent_child_fixture()
    : child("1.1.1.1.1.7.0.2.4.e164.arpa"),
      parent("7.0.2.4.e164.arpa")
    {
        Fred::OperationContextCreator ctx;

        Test::exec(
            Test::CreateX_factory<Fred::CreateDomain>()
                .make(Test::registrar(ctx).info_data.handle,
                      Test::contact(ctx).info_data.handle,
                      child)
                .set_nsset(Test::nsset::make(ctx).handle)
                .set_keyset(Test::keyset::make(ctx).handle)
                .set_admin_contacts(
                    Util::vector_of<std::string>(
                        Test::contact::make(ctx).handle))
                .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                     boost::gregorian::date_duration(2))
                .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() +
                                     boost::gregorian::date_duration(2)),
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
            Database::query_param_list(child));
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
            Database::query_param_list(child));

        Fred::InfoDomainOutput dom = Fred::InfoDomainByFqdn(child).exec(ctx, "UTC");
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(del_can_parent_child, del_can_parent_child_fixture)
{
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(parent);

    BOOST_CHECK(dom.fqdn == child);
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.validated_to.isnull());
    BOOST_CHECK(dom.admin_contacts.empty());
    BOOST_CHECK(dom.statuses.size() == 1);
    BOOST_CHECK(dom.statuses.at(0)  == "deleteCandidate");
    BOOST_CHECK(dom.registered      == boost::posix_time::ptime(not_a_date_time));
    BOOST_CHECK(dom.registrant      == "");
    BOOST_CHECK(dom.expire          == boost::gregorian::date(not_a_date_time));
    BOOST_CHECK(dom.keyset          == "");
    BOOST_CHECK(dom.nsset           == "");
    BOOST_CHECK(dom.sponsoring_registrar == "");
    BOOST_CHECK(dom.expire_time_estimate == boost::posix_time::ptime(not_a_date_time));
    BOOST_CHECK(dom.expire_time_actual.isnull());
    BOOST_CHECK(dom.validated_to_time_estimate.isnull());
    BOOST_CHECK(dom.validated_to_time_actual.isnull());
}

BOOST_AUTO_TEST_SUITE_END()//get_domain_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
