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
#include "libfred/object_state/create_admin_object_block_request_id.hh"
#include "libfred/object_state/create_admin_object_state_restore_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
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
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-create-admin-object-state-restore-request-id";

struct create_admin_object_state_restore_request_id_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    ::LibFred::ObjectId test_domain_id;
    ::LibFred::StatusList status_list;
    const ::LibFred::ObjectId logd_request_id;

    create_admin_object_state_restore_request_id_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        admin_contact2_handle(std::string("TEST-CAOSRR-ADMIN-CONTACT-HANDLE") + xmark),
        registrant_contact_handle(std::string("TEST-CAOSRR-REGISTRANT-CONTACT-HANDLE") + xmark),
        test_domain_fqdn(std::string("fred") + xmark + ".cz"),
        logd_request_id(23456)
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-CAOSRR-ADMIN-CONTACT NAME")+xmark)
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

        ::LibFred::CreateDomain(test_domain_fqdn, registrar_handle, registrant_contact_handle)
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
            .exec(ctx);

        Database::Result status_result = ctx.get_conn().exec("SELECT name FROM enum_object_states WHERE manual AND 3=ANY(types) AND name!='serverBlocked'");
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            status_list.insert(status_result[idx][0]);
        }
        test_domain_id = static_cast< ::LibFred::ObjectId >(ctx.get_conn().exec_params(
            "SELECT id "
            "FROM object_registry "
            "WHERE name=$1::text AND "
                  "type=3 AND "
                  "erdate IS NULL", Database::query_param_list(test_domain_fqdn))[0][0]);
        const std::string handle = ::LibFred::CreateAdminObjectBlockRequestId(test_domain_id, status_list).exec(ctx);
        BOOST_CHECK(handle == test_domain_fqdn);
        ::LibFred::PerformObjectStateRequest(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    ~create_admin_object_state_restore_request_id_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateAdminObjectStateRestoreRequestId, create_admin_object_state_restore_request_id_fixture)

/**
 * test CreateAdminObjectStateRestoreRequestId
 * ...
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(create_admin_object_state_restore_request_id)
{
    const std::string query =
        "SELECT eos.name "
        "FROM object_state os "
        "JOIN enum_object_states eos ON (eos.id=os.state_id) "
        "WHERE os.object_id=$1::bigint AND "
              "os.valid_from<=CURRENT_TIMESTAMP AND "
              "(os.valid_to IS NULL OR CURRENT_TIMESTAMP<valid_to)";
    const Database::query_param_list param(test_domain_id);

    ::LibFred::StatusList status_list_before;
    ::LibFred::StatusList status_list_after;
    Database::Result status_result;
    {
        ::LibFred::OperationContextCreator ctx;
        status_result = ctx.get_conn().exec_params(query, param);
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            status_list_before.insert(static_cast< std::string >(status_result[idx][0]));
        }
        ::LibFred::CreateAdminObjectStateRestoreRequestId(test_domain_id, "test CreateAdminObjectStateRestoreRequestId operation", logd_request_id).exec(ctx);
        BOOST_CHECK(true);
        ::LibFred::PerformObjectStateRequest(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    ::LibFred::OperationContextCreator ctx;
    status_result = ctx.get_conn().exec_params(query, param);
    for (::size_t idx = 0; idx < status_result.size(); ++idx) {
        status_list_after.insert(static_cast< std::string >(status_result[idx][0]));
    }
    BOOST_CHECK(status_list_before.size() == (status_list_after.size() + status_list.size() + 1));
    ::LibFred::StatusList status_list_sum = status_list; // status_list_after + status_list + "serverBlocked"
    status_list_sum.insert("serverBlocked");
    for (::LibFred::StatusList::const_iterator pStatus = status_list_after.begin(); pStatus != status_list_after.end(); ++pStatus) {
        status_list_sum.insert(*pStatus);
    }
    bool success = true;
    for (::LibFred::StatusList::const_iterator pStatus = status_list_sum.begin(); pStatus != status_list_sum.end(); ++pStatus) {
        if (status_list_before.find(*pStatus) == status_list_before.end()) {
            success = false;
            break;
        }
    }
    BOOST_CHECK(success); // status_list_before == status_list_sum
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateAdminObjectStateRestoreRequestId
