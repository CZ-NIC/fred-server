/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/test/unit_test.hpp>
#include <string>

#include "src/fredlib/opcontext.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/clear_admin_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include <fredlib/contact.h>
#include <fredlib/domain.h>

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-clear-admin-object-state-request-id";

struct clear_admin_object_state_request_id_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    Fred::ObjectId test_domain_id;
    Fred::StatusList status_list;

    clear_admin_object_state_request_id_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-CAOSR-ADMIN-CONTACT-HANDLE") + xmark)
    , registrant_contact_handle(std::string("TEST-CAOSR-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn ( std::string("fred")+xmark+".cz")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-CAOSR-ADMIN-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateDomain(
                test_domain_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
            .exec(ctx);

        Database::Result status_result = ctx.get_conn().exec("SELECT name FROM enum_object_states WHERE manual AND 3=ANY(types)");
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            status_list.insert(status_result[idx][0]);
        }
        test_domain_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec_params(
            "SELECT id "
            "FROM object_registry "
            "WHERE name=$1::text AND "
                  "type=3 AND "
                  "erdate IS NULL", Database::query_param_list(test_domain_fqdn))[0][0]);
        Fred::CreateObjectStateRequestId(test_domain_id, status_list).exec(ctx);
        Fred::PerformObjectStateRequest(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    ~clear_admin_object_state_request_id_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestClearAdminObjectStateRequestId, clear_admin_object_state_request_id_fixture )

/**
 * test ClearAdminObjectStateRequestId
 * ...
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(clear_admin_object_state_request_id)
{
    {
        Fred::OperationContext ctx;
        Fred::ClearAdminObjectStateRequestId(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    Fred::OperationContext ctx;
    std::ostringstream query;
    Database::query_param_list param(test_domain_id);
    Fred::StatusList::const_iterator pStatus = status_list.begin();
    param(*pStatus);
    ++pStatus;
    query << "SELECT eos.name "
             "FROM object_state_request osr "
             "JOIN object_registry obr ON obr.id=osr.object_id "
             "JOIN enum_object_states eos ON (eos.id=osr.state_id AND obr.type=ANY(eos.types)) "
             "WHERE osr.object_id=$1::bigint AND "
                   "osr.valid_from<=CURRENT_TIMESTAMP AND "
                   "(osr.valid_to IS NULL OR CURRENT_TIMESTAMP<valid_to) AND "
                   "obr.erdate IS NULL AND "
                   "osr.canceled IS NULL AND "
                   "eos.manual AND "
                   "eos.name IN ($" << param.size() << "::text";
    while (pStatus != status_list.end()) {
        param(*pStatus);
        ++pStatus;
        query << ",$" << param.size() << "::text";
    }
    query << ")";
    Database::Result status_result = ctx.get_conn().exec_params(query.str(), param);
    BOOST_CHECK(status_result.size() == 0);
    Fred::PerformObjectStateRequest(test_domain_id).exec(ctx);
    ctx.commit_transaction();
}

/**
 * test ClearAdminObjectStateRequestIdBad
 * ...
 * calls in test should throw
 */
BOOST_AUTO_TEST_CASE(clear_admin_object_state_request_id_bad)
{
    Fred::ObjectId not_used_id;
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        not_used_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec("SELECT (MAX(id)+1000)*2 FROM object_registry")[0][0]);
        Fred::ClearAdminObjectStateRequestId(not_used_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::ClearAdminObjectStateRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_object_id_not_found());
        BOOST_CHECK(ex.get_object_id_not_found() == not_used_id);
    }

    {
        Fred::StatusList status_list;
        status_list.insert("serverBlocked");
        Fred::OperationContext ctx;
        Fred::CancelObjectStateRequestId(test_domain_id, status_list).exec(ctx);
        Fred::PerformObjectStateRequest(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::ClearAdminObjectStateRequestId(test_domain_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::ClearAdminObjectStateRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_server_blocked_absent());
        BOOST_CHECK(ex.get_server_blocked_absent() == test_domain_id);
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestClearAdminObjectStateRequestId
