/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/create_domain_name_blacklist_id.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_FIXTURE_TEST_SUITE(TestDeleteDomain, Test::instantiate_db_template)

const std::string server_name = "test-delete-domain";

struct delete_enum_domain_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;

    delete_enum_domain_fixture()
    : xmark(RandomDataGenerator().xnumstring(9))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn ( std::string()+xmark.at(0)+'.'+xmark.at(1)+'.'+xmark.at(2)+'.'
                                    +xmark.at(3)+'.'+xmark.at(4)+'.'+xmark.at(5)+'.'
                                    +xmark.at(6)+'.'+xmark.at(7)+'.'+xmark.at(8)+".0.2.4.e164.arpa")
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateContact(registrant_contact_handle,registrar_handle)
                .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

        ::LibFred::CreateDomain(
                test_domain_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .set_enum_validation_expiration(boost::gregorian::day_clock::day_clock::universal_day()+boost::gregorian::days(5))
        .set_enum_publish_flag(false)
        .exec(ctx);
        ctx.commit_transaction();
    }
    ~delete_enum_domain_fixture()
    {}
};

struct delete_domain_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;

    delete_domain_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn ( std::string("fred")+xmark+".cz")
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateContact(registrant_contact_handle,registrar_handle)
                .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
                .set_disclosename(true)
                .set_place(place)
                .set_discloseaddress(true)
                .exec(ctx);

        ::LibFred::CreateDomain(
                test_domain_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
        .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
        .exec(ctx);
        ctx.commit_transaction();
    }
    ~delete_domain_fixture()
    {}
};

/**
 * test DeleteDomain
 * create test domain, delete test domain, check erdate of test domain is null
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(delete_domain, delete_domain_fixture )
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoDomainOutput domain_info1 = ::LibFred::InfoDomainByFqdn(test_domain_fqdn).exec(ctx);
    BOOST_CHECK(domain_info1.info_domain_data.delete_time.isnull());

    ::LibFred::DeleteDomainByFqdn(test_domain_fqdn).exec(ctx);

    std::vector<::LibFred::InfoDomainOutput> domain_history_info1 = ::LibFred::InfoDomainHistoryByRoid(
    domain_info1.info_domain_data.roid).exec(ctx);

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    ::LibFred::InfoDomainOutput domain_info1_with_change = domain_info1;
    domain_info1_with_change.info_domain_data.delete_time = domain_history_info1.at(0).info_domain_data.delete_time;

    BOOST_CHECK(domain_info1_with_change == domain_history_info1.at(0));

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    BOOST_CHECK(domain_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(domain_history_info1.at(0).history_valid_from <= domain_history_info1.at(0).history_valid_to.get_value());

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_domain_fqdn))[0][0]));

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT o.id FROM object o JOIN object_registry oreg ON o.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT d.id FROM domain d JOIN object_registry oreg ON d.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT dcm.contactid FROM domain_contact_map dcm JOIN object_registry oreg ON dcm.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT ev.domainid FROM enumval ev JOIN object_registry oreg ON ev.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    ctx.commit_transaction();
}//delete_domain

/**
 * test DeleteDomain
 * create test ENUM domain, delete test ENUM domain, check erdate of test ENUM domain is null
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(delete_enum_domain, delete_enum_domain_fixture )
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoDomainOutput domain_info1 = ::LibFred::InfoDomainByFqdn(test_domain_fqdn).exec(ctx);
    BOOST_CHECK(domain_info1.info_domain_data.delete_time.isnull());

    ::LibFred::DeleteDomainByFqdn(test_domain_fqdn).exec(ctx);

    std::vector<::LibFred::InfoDomainOutput> domain_history_info1 = ::LibFred::InfoDomainHistoryByRoid(
    domain_info1.info_domain_data.roid).exec(ctx);

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    ::LibFred::InfoDomainOutput domain_info1_with_change = domain_info1;
    domain_info1_with_change.info_domain_data.delete_time = domain_history_info1.at(0).info_domain_data.delete_time;

    BOOST_CHECK(domain_info1_with_change == domain_history_info1.at(0));

    BOOST_CHECK(!domain_history_info1.at(0).info_domain_data.delete_time.isnull());

    BOOST_CHECK(domain_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!domain_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(domain_history_info1.at(0).history_valid_from <= domain_history_info1.at(0).history_valid_to.get_value());

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_domain_fqdn))[0][0]));

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT o.id FROM object o JOIN object_registry oreg ON o.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT d.id FROM domain d JOIN object_registry oreg ON d.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT dcm.contactid FROM domain_contact_map dcm JOIN object_registry oreg ON dcm.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT ev.domainid FROM enumval ev JOIN object_registry oreg ON ev.domainid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_domain_fqdn)).size() == 0);

    ctx.commit_transaction();
}//delete_domain


/**
 * test DeleteDomain with wrong fqdn
 */

BOOST_FIXTURE_TEST_CASE(delete_domain_with_wrong_fqdn, delete_domain_fixture )
{
    std::string bad_test_domain_fqdn = std::string("bad")+test_domain_fqdn;
    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::DeleteDomainByFqdn(bad_test_domain_fqdn).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::DeleteDomainByFqdn::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_domain_fqdn());
        BOOST_CHECK(ex.get_unknown_domain_fqdn().compare(bad_test_domain_fqdn) == 0);
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestDeleteDomain
