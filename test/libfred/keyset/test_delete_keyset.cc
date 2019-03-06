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
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/info_domain_impl.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/delete_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_diff.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-delete-keyset";

struct delete_keyset_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact_handle;
    std::string test_keyset_handle;
    std::string test_domain_fqdn;

    delete_keyset_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , test_keyset_handle ( std::string("TEST-DEL-KEYSET-")+xmark+"-HANDLE")
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
        ::LibFred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        ctx.commit_transaction();
    }
    ~delete_keyset_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestDeleteKeyset, delete_keyset_fixture)

/**
 * test DeleteKeyset
 * create test keyset, delete test keyset, check erdate of test keyset is null
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(delete_keyset)
{
    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoKeysetOutput keyset_info1 = ::LibFred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    BOOST_CHECK(keyset_info1.info_keyset_data.delete_time.isnull());

    ::LibFred::DeleteKeysetByHandle(test_keyset_handle).exec(ctx);

    std::vector<::LibFred::InfoKeysetOutput> keyset_history_info1 = ::LibFred::InfoKeysetHistoryByRoid(
    keyset_info1.info_keyset_data.roid).exec(ctx);

    BOOST_CHECK(!keyset_history_info1.at(0).info_keyset_data.delete_time.isnull());

    ::LibFred::InfoKeysetOutput keyset_info1_with_change = keyset_info1;
    keyset_info1_with_change.info_keyset_data.delete_time = keyset_history_info1.at(0).info_keyset_data.delete_time;

    BOOST_CHECK(keyset_info1_with_change == keyset_history_info1.at(0));

    BOOST_CHECK(!keyset_history_info1.at(0).info_keyset_data.delete_time.isnull());

    BOOST_CHECK(keyset_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!keyset_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!keyset_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(keyset_history_info1.at(0).history_valid_from <= keyset_history_info1.at(0).history_valid_to.get_value());

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_keyset_handle))[0][0]));

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT o.id FROM object o JOIN object_registry oreg ON o.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_keyset_handle)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT k.id FROM keyset k JOIN object_registry oreg ON k.id = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_keyset_handle)).size() == 0);

    BOOST_CHECK(ctx.get_conn().exec_params(
        "SELECT kcm.contactid FROM keyset_contact_map kcm JOIN object_registry oreg ON kcm.keysetid = oreg.id WHERE oreg.name = $1::text"
        , Database::query_param_list(test_keyset_handle)).size() == 0);

    ctx.commit_transaction();
}//delete_keyset


/**
 * test DeleteKeyset with wrong handle
 */

BOOST_AUTO_TEST_CASE(delete_keyset_with_wrong_handle)
{
    std::string bad_test_keyset_handle = std::string("bad")+test_keyset_handle;
    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::DeleteKeysetByHandle(bad_test_keyset_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::DeleteKeysetByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_keyset_handle());
        BOOST_CHECK(ex.get_unknown_keyset_handle().compare(bad_test_keyset_handle) == 0);
    }
}

/**
 * test DeleteKeyset linked
 */

BOOST_AUTO_TEST_CASE(delete_linked_keyset)
{
    {
        ::LibFred::OperationContextCreator ctx;
        //create linked object

        ::LibFred::CreateDomain(test_domain_fqdn //const std::string& fqdn
            , registrar_handle //const std::string& registrar
            , admin_contact_handle //registrant
            ).set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_keyset(test_keyset_handle)
            .exec(ctx);

       ctx.commit_transaction();
    }

    try
    {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::DeleteKeysetByHandle(test_keyset_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const ::LibFred::DeleteKeysetByHandle::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_linked_to_keyset_handle());
        BOOST_CHECK(ex.get_object_linked_to_keyset_handle().compare(test_keyset_handle) == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestDeleteKeyset
