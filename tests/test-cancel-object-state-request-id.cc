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
#include "fredlib/registrar.h"
#include "fredlib/object_state/cancel_object_state_request_id.h"
#include "fredlib/object_state/create_object_state_request_id.h"
#include "fredlib/opexception.h"
#include "util/util.h"

#include "fredlib/contact/create_contact.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/domain/delete_domain.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"
#include "fredlib/domain/info_domain_compare.h"

#include "fredlib/contact_verification/contact.h"
#include "fredlib/object_states.h"
#include "contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestCancelObjectStateRequestId)

const std::string server_name = "test-cancel-object-state-request-id";

struct cancel_object_state_request_id_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    Fred::ObjectId test_domain_id;
    Fred::StatusList status_list;

    cancel_object_state_request_id_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-COSR-ADMIN-CONTACT-HANDLE") + xmark)
    , registrant_contact_handle(std::string("TEST-COSR-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn ( std::string("fred")+xmark+".cz")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-COSR-ADMIN-CONTACT NAME")+xmark)
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
    ~cancel_object_state_request_id_fixture()
    {}
};

/**
 * test CancelObjectStateRequestId
 * ...
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(cancel_object_state_request_id, cancel_object_state_request_id_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CancelObjectStateRequestId(test_domain_id, status_list).exec(ctx);
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
 * test CancelObjectStateRequestIdBad
 * ...
 * calls in test should throw
 */
BOOST_FIXTURE_TEST_CASE(cancel_object_state_request_id_bad, cancel_object_state_request_id_fixture)
{
    Fred::ObjectId not_used_id;
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        not_used_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec("SELECT (MAX(id)+1000)*2 FROM object_registry")[0][0]);
        Fred::CancelObjectStateRequestId(not_used_id, status_list).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CancelObjectStateRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_object_id_not_found());
        BOOST_CHECK(ex.get_object_id_not_found() == not_used_id);
    }

    Fred::StatusList bad_status_list = status_list;
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Database::Result status_result = ctx.get_conn().exec("SELECT name FROM enum_object_states WHERE NOT (manual AND 3=ANY(types))");
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            bad_status_list.insert(status_result[idx][0]);
        }
        bad_status_list.insert(std::string("BadStatus") + xmark);
        Fred::CancelObjectStateRequestId(test_domain_id, bad_status_list).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CancelObjectStateRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_state_not_found());
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCancelObjectStateRequestId
