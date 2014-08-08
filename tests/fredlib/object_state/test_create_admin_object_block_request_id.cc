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

const std::string server_name = "test-create-administrative-object-block-request-id";

struct create_administrative_object_block_request_id_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    Fred::ObjectId test_domain_id;
    Fred::StatusList status_list;

    create_administrative_object_block_request_id_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        admin_contact2_handle(std::string("TEST-OSR-ADMIN-CONTACT-HANDLE") + xmark),
        registrant_contact_handle(std::string("TEST-OSR-REGISTRANT-CONTACT-HANDLE") + xmark),
        test_domain_fqdn ( std::string("fred")+xmark+".cz")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-AOB-ADMIN-CONTACT NAME")+xmark)
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
        ctx.commit_transaction();
    }
    ~create_administrative_object_block_request_id_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateAdminObjectBlockRequestId, create_administrative_object_block_request_id_fixture )

/**
 * test CreateAdminObjectBlockRequestId
 * ...
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(create_administrative_object_block_request_id)
{
    {
        Fred::OperationContext ctx;
        const std::string handle = Fred::CreateAdminObjectBlockRequestId(test_domain_id, status_list).exec(ctx);
        BOOST_CHECK(handle == test_domain_fqdn);
        ctx.commit_transaction();
    }
    Fred::OperationContext ctx;
    Database::Result status_result = ctx.get_conn().exec_params(
        "SELECT eos.name "
        "FROM object_state_request osr "
        "JOIN object_registry obr ON obr.id=osr.object_id "
        "JOIN enum_object_states eos ON (eos.id=osr.state_id AND obr.type=ANY(eos.types)) "
        "WHERE osr.object_id=$1::bigint AND "
              "osr.valid_from<=CURRENT_TIMESTAMP AND "
              "(osr.valid_to IS NULL OR CURRENT_TIMESTAMP<valid_to) AND "
              "obr.erdate IS NULL AND "
              "osr.canceled IS NULL AND "
              "eos.manual", Database::query_param_list(test_domain_id));
    BOOST_CHECK((status_list.size() + 1) <= status_result.size()); // status_list + 'serverBlocked'
    Fred::StatusList domain_status_list;
    for (::size_t idx = 0; idx < status_result.size(); ++idx) {
        domain_status_list.insert(static_cast< std::string >(status_result[idx][0]));
    }
    BOOST_CHECK(domain_status_list.find("serverBlocked") != domain_status_list.end());
    for (Fred::StatusList::const_iterator pStatus = status_list.begin(); pStatus != status_list.end(); ++pStatus) {
        BOOST_CHECK(domain_status_list.find(*pStatus) != domain_status_list.end());
    }
}

/**
 * test CreateAdminObjectBlockRequestIdBad
 * ...
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(create_administrative_object_block_request_id_bad)
{
    Fred::StatusList bad_status_list = status_list;
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Database::Result status_result = ctx.get_conn().exec("SELECT name FROM enum_object_states WHERE NOT (manual AND 3=ANY(types))");
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            bad_status_list.insert(status_result[idx][0]);
        }
        bad_status_list.insert(std::string("BadStatus") + xmark);
        Fred::CreateAdminObjectBlockRequestId(test_domain_id, bad_status_list).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateAdminObjectBlockRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_vector_of_state_not_found());
        BOOST_CHECK(ex.get_vector_of_state_not_found().size() == (bad_status_list.size() - status_list.size()));
    }

    Fred::StatusList status_list_a;
    Fred::StatusList status_list_b = status_list;
    status_list_a.insert(*status_list_b.begin());
    status_list_b.erase(status_list_b.begin());
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::CreateAdminObjectBlockRequestId(test_domain_id, status_list_a).exec(ctx);
        Fred::PerformObjectStateRequest(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::CreateAdminObjectBlockRequestId(test_domain_id, status_list_b).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateAdminObjectBlockRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_server_blocked_present());
        BOOST_CHECK(ex.get_server_blocked_present() == test_domain_id);
    }

}

BOOST_AUTO_TEST_SUITE_END();//TestCreateAdminObjectBlockRequestId
