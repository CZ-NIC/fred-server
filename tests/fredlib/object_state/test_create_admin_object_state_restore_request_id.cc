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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "setup_server_decl.h"
#include "time_clock.h"

#include "src/fredlib/registrar.h"
#include "src/fredlib/object_state/create_admin_object_block_request_id.h"
#include "src/fredlib/object_state/create_admin_object_state_restore_request_id.h"
#include "src/fredlib/opexception.h"

#include "util/util.h"

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/delete_domain.h"
#include "src/fredlib/domain/info_domain.h"

#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/object_states.h"
#include "src/contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "src/fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "tests/setup/fixtures.h"

const std::string server_name = "test-create-admin-object-state-restore-request-id";

struct create_admin_object_state_restore_request_id_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    Fred::ObjectId test_domain_id;
    Fred::StatusList status_list;
    const Fred::ObjectId logd_request_id;

    create_admin_object_state_restore_request_id_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        admin_contact2_handle(std::string("TEST-CAOSRR-ADMIN-CONTACT-HANDLE") + xmark),
        registrant_contact_handle(std::string("TEST-CAOSRR-REGISTRANT-CONTACT-HANDLE") + xmark),
        test_domain_fqdn(std::string("fred") + xmark + ".cz"),
        logd_request_id(23456)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-CAOSRR-ADMIN-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateDomain(test_domain_fqdn, registrar_handle, registrant_contact_handle)
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
            .exec(ctx);

        Database::Result status_result = ctx.get_conn().exec("SELECT name FROM enum_object_states WHERE manual AND 3=ANY(types) AND name!='serverBlocked'");
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            status_list.insert(status_result[idx][0]);
        }
        test_domain_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec_params(
            "SELECT id "
            "FROM object_registry "
            "WHERE name=$1::text AND "
                  "type=3 AND "
                  "erdate IS NULL", Database::query_param_list(test_domain_fqdn))[0][0]);
        const std::string handle = Fred::CreateAdminObjectBlockRequestId(test_domain_id, status_list).exec(ctx);
        BOOST_CHECK(handle == test_domain_fqdn);
        Fred::PerformObjectStateRequest(test_domain_id).exec(ctx);
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

    Fred::StatusList status_list_before;
    Fred::StatusList status_list_after;
    Database::Result status_result;
    {
        Fred::OperationContext ctx;
        status_result = ctx.get_conn().exec_params(query, param);
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            status_list_before.insert(static_cast< std::string >(status_result[idx][0]));
        }
        Fred::CreateAdminObjectStateRestoreRequestId(test_domain_id, "test CreateAdminObjectStateRestoreRequestId operation", logd_request_id).exec(ctx);
        BOOST_CHECK(true);
        Fred::PerformObjectStateRequest(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    Fred::OperationContext ctx;
    status_result = ctx.get_conn().exec_params(query, param);
    for (::size_t idx = 0; idx < status_result.size(); ++idx) {
        status_list_after.insert(static_cast< std::string >(status_result[idx][0]));
    }
    BOOST_CHECK(status_list_before.size() == (status_list_after.size() + status_list.size() + 1));
    Fred::StatusList status_list_sum = status_list; // status_list_after + status_list + "serverBlocked"
    status_list_sum.insert("serverBlocked");
    for (Fred::StatusList::const_iterator pStatus = status_list_after.begin(); pStatus != status_list_after.end(); ++pStatus) {
        status_list_sum.insert(*pStatus);
    }
    bool success = true;
    for (Fred::StatusList::const_iterator pStatus = status_list_sum.begin(); pStatus != status_list_sum.end(); ++pStatus) {
        if (status_list_before.find(*pStatus) == status_list_before.end()) {
            success = false;
            break;
        }
    }
    BOOST_CHECK(success); // status_list_before == status_list_sum
}

///**
// * test CreateAdminObjectStateRestoreRequestIdBad
// * ...
// * calls in test shouldn't throw
// */
BOOST_AUTO_TEST_CASE(create_admin_object_state_restore_request_id_bad)
{
    Fred::ObjectId not_used_id;
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        not_used_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec("SELECT (MAX(id)+1000)*2 FROM object_registry")[0][0]);
        Fred::CreateAdminObjectStateRestoreRequestId(not_used_id, "test CreateAdminObjectStateRestoreRequestId operation", logd_request_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateAdminObjectStateRestoreRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_object_id_not_found());
        BOOST_CHECK(ex.get_object_id_not_found() == not_used_id);
    }

    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::CreateAdminObjectStateRestoreRequestId(test_domain_id, "test CreateAdminObjectStateRestoreRequestId operation", logd_request_id).exec(ctx);
        ctx.commit_transaction();
    }
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::CreateAdminObjectStateRestoreRequestId(test_domain_id, "test CreateAdminObjectStateRestoreRequestId operation", logd_request_id).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateAdminObjectStateRestoreRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_server_blocked_absent());
        BOOST_CHECK(ex.get_server_blocked_absent() == test_domain_id);
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateAdminObjectStateRestoreRequestId
