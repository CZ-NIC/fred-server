/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "test/backend/whois/fixture_common.hh"

#include "src/backend/whois/domain_expiration_datetime.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/domain/transfer_domain.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_domains_by_nsset)

struct domains_by_nsset_fixture
: whois_impl_instance_fixture
{
    const std::string test_nsset;
    unsigned int regular_domains;
    std::map<std::string, LibFred::InfoDomainData> domain_info;
    boost::posix_time::ptime now_utc;
    const std::string delete_fqdn;

    domains_by_nsset_fixture()
    : test_nsset("test-nsset"),
      regular_domains(6), //XXX
      delete_fqdn("test-delete.cz")
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const LibFred::InfoContactData contact     = Test::contact::make(ctx),
                                    admin       = Test::contact::make(ctx);
        const LibFred::InfoNssetData nsset         = Test::nsset::make(ctx, test_nsset),
                                  other_nsset   = Test::nsset::make(ctx);
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
        for(unsigned int i=0; i < regular_domains; ++i)
        {
            const LibFred::InfoDomainData& idd = Test::exec(   //lifetime extension of temporary
                    Test::CreateX_factory<::LibFred::CreateDomain>()
                        .make(registrar.handle, contact.handle)
                        .set_nsset(test_nsset)
                        .set_keyset(Test::keyset::make(ctx).handle)
                        .set_admin_contacts(
                            Util::vector_of<std::string>(
                                Test::contact::make(ctx).handle))
                        .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                             boost::gregorian::date_duration(2)),
                    ctx);
            domain_info[idd.fqdn] = idd;
        }
        //enum domain
        regular_domains++;
        const LibFred::InfoDomainData& enum_domain = Test::exec(
                Test::CreateX_factory<::LibFred::CreateDomain>()
                    .make(registrar.handle, contact.handle, "7.3.5.7.0.2.4.e164.arpa")
                    .set_admin_contacts(Util::vector_of<std::string>(admin.handle))
                    .set_nsset(test_nsset)
                    .set_keyset(Test::keyset::make(ctx).handle)
                    .set_expiration_date(boost::gregorian::day_clock::local_day() +
                                         boost::gregorian::date_duration(2))
                    .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() +
                                                    boost::gregorian::date_duration(2)),
                ctx);
        domain_info[enum_domain.fqdn] = enum_domain;
        for(int i=0; i < 3; ++i)//3 different domains for another nsset
        {
            Test::exec(
                    Test::CreateX_factory<::LibFred::CreateDomain>()
                        .make(registrar.handle, contact.handle)
                        .set_nsset(other_nsset.handle)
                        .set_admin_contacts(
                            Util::vector_of<std::string>(admin.handle)),
                       ctx);
        }
        //1 with no nsset
        Test::exec(Test::CreateX_factory<::LibFred::CreateDomain>()
                       .make(registrar.handle, contact.handle)
                       .set_admin_contacts(
                           Util::vector_of<std::string>(admin.handle)),
                   ctx);

        //delete candidate
        const LibFred::InfoDomainData& idd = Test::exec(
                Test::CreateX_factory<::LibFred::CreateDomain>()
                    .make(registrar.handle, contact.handle, delete_fqdn)
                    .set_admin_contacts(Util::vector_of<std::string>(Test::contact::make(ctx).handle))
                    .set_nsset(test_nsset)
                    .set_keyset(Test::keyset::make(ctx).handle)
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
        LibFred::InfoDomainOutput dom = LibFred::InfoDomainByFqdn(delete_fqdn).exec(ctx, "UTC");
        LibFred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset, domains_by_nsset_fixture)
{
    Fred::Backend::Whois::DomainSeq domain_seq = impl.get_domains_by_nsset(test_nsset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);
    BOOST_CHECK(domain_seq.content.size() == regular_domains);

    std::map<std::string, LibFred::InfoDomainData>::iterator found;
    BOOST_FOREACH(const Fred::Backend::Whois::Domain& it, domain_seq.content)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_REQUIRE(it.fqdn     == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.sponsoring_registrar  == found->second.sponsoring_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const LibFred::RegistrableObject::Contact::ContactReference& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() != std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        LibFred::OperationContextCreator ctx;
        const std::vector<::LibFred::ObjectStateData> v_osd =
            LibFred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const LibFred::ObjectStateData& oit, v_osd)
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

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_limit_exceeded, domains_by_nsset_fixture)
{
    Fred::Backend::Whois::DomainSeq domain_seq = impl.get_domains_by_nsset(test_nsset, regular_domains - 1);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Fred::Backend::Whois::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == regular_domains - 1);
    std::map<std::string, LibFred::InfoDomainData>::iterator found;
    BOOST_FOREACH(const Fred::Backend::Whois::Domain& it, domain_vec)
    {
        found = domain_info.find(it.fqdn);
        BOOST_REQUIRE(found != domain_info.end());
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.registered == now_utc);
        BOOST_REQUIRE(it.fqdn     == found->second.fqdn);
        BOOST_CHECK(it.registrant == found->second.registrant.handle);
        BOOST_CHECK(it.sponsoring_registrar  == found->second.sponsoring_registrar_handle);
        BOOST_CHECK(it.expire     == found->second.expiration_date);
        BOOST_CHECK(it.fqdn       == found->second.fqdn);
        BOOST_CHECK(it.keyset     == found->second.keyset.get_value_or_default().handle);
        BOOST_CHECK(it.nsset      == found->second.nsset.get_value_or_default().handle);

        BOOST_FOREACH(const LibFred::RegistrableObject::Contact::ContactReference& oit, found->second.admin_contacts)
        {
            BOOST_CHECK(it.admin_contacts.end() != std::find(it.admin_contacts.begin(),
                        it.admin_contacts.end(), oit.handle));
        }
        BOOST_CHECK(it.admin_contacts.size() == found->second.admin_contacts.size());

        LibFred::OperationContextCreator ctx;
        const std::vector<::LibFred::ObjectStateData> v_osd =
            LibFred::GetObjectStates(found->second.id).exec(ctx);
        BOOST_FOREACH(const LibFred::ObjectStateData& oit, v_osd)
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

struct update_domains_by_nsset_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    LibFred::InfoDomainData domain;
    const std::string test_fqdn;
    const std::string test_nsset;
    std::string transfer_handle;

    update_domains_by_nsset_fixture()
    : test_fqdn("7.3.5.7.0.2.4.e164.arpa"), //ENUM domain covers both enum and usual cases
      test_nsset("test-nsset"),
      transfer_handle("TR REG HANDLE")
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx),
                             transfer_registrar = Test::registrar::make(ctx, transfer_handle);
        const LibFred::InfoContactData contact     = Test::contact::make(ctx);
        const LibFred::InfoNssetData nsset         = Test::nsset::make(ctx, test_nsset);
        domain = Test::exec(
            Test::CreateX_factory<::LibFred::CreateDomain>()
                .make(registrar.handle,
                      contact.handle,
                      test_fqdn)
                .set_nsset(test_nsset)
                .set_keyset(Test::keyset::make(ctx).handle)
                .set_expiration_date(boost::gregorian::day_clock::local_day() -
                    boost::gregorian::date_duration(2))
                .set_enum_validation_expiration(boost::gregorian::day_clock::local_day() -
                    boost::gregorian::date_duration(2)),
            ctx);
        LibFred::UpdateDomain(test_fqdn, registrar.handle)
            .unset_keyset()
            .exec(ctx);
        LibFred::TransferDomain(
            LibFred::InfoDomainByFqdn(test_fqdn)
                .exec( ctx, "UTC" )
                .info_domain_data
                .id,
            transfer_handle,
            domain.authinfopw,
            0)
            .exec(ctx);
        LibFred::InfoDomainOutput dom = LibFred::InfoDomainByFqdn(test_fqdn).exec(ctx, "UTC");
        LibFred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(update_domains_by_nsset, update_domains_by_nsset_fixture)
{
    Fred::Backend::Whois::Domain dom = impl.get_domains_by_nsset(test_nsset, 1).content.at(0);
    BOOST_CHECK(dom.changed == now_utc);
    BOOST_CHECK(dom.last_transfer == now_utc);
    BOOST_CHECK(dom.sponsoring_registrar == transfer_handle);

    LibFred::OperationContextCreator ctx;
    BOOST_CHECK(dom.validated_to_time_actual.get_value() ==
            Fred::Backend::Whois::domain_validation_expiration_datetime_actual(ctx, domain.id).get_value());

    Optional<boost::posix_time::ptime> eta = Fred::Backend::Whois::domain_expiration_datetime_actual(ctx, domain.id);
    BOOST_CHECK(dom.expire_time_actual.get_value() == eta.get_value());
}


BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_absent_nsset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_nsset("absent-nsset", 1), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_nsset_no_nsset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_domains_by_nsset("", 1), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_nsset
BOOST_AUTO_TEST_SUITE_END();//TestWhois
