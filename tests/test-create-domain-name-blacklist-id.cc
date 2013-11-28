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
#include "fredlib/domain/create_object_state_request_id.h"
#include "fredlib/domain/create_domain_name_blacklist_id.h"
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

BOOST_AUTO_TEST_SUITE(TestCreateDomainNameBlacklistId)

const std::string server_name = "test-create-domain-name-blacklist-id";

#define LOGME(WHAT) \
std::cout << __FILE__ << "(" << __LINE__ << "): " << WHAT << " [in " << __PRETTY_FUNCTION__ << "]" << std::endl

struct create_domain_name_blacklist_id_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    std::string regexp_test_domain_fqdn;
    Fred::ObjectId test_domain_id;

    create_domain_name_blacklist_id_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-CDNB-ADMIN-CONTACT-HANDLE") + xmark)
    , registrant_contact_handle(std::string("TEST-CDNB-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn(std::string("fred") + xmark + ".cz")
    , regexp_test_domain_fqdn(std::string("^fred") + xmark + "\\.cz$")
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-CDNB-ADMIN-CONTACT NAME")+xmark)
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

        test_domain_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec_params(
            "SELECT id "
            "FROM object_registry "
            "WHERE name=$1::text AND "
                  "type=3 AND "
                  "erdate IS NULL", Database::query_param_list(test_domain_fqdn))[0][0]);

        ctx.commit_transaction();
    }
    ~create_domain_name_blacklist_id_fixture()
    {}
};

/**
 * test CreateDomainNameBlacklistId
 * ...
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(create_domain_name_blacklist_id, create_domain_name_blacklist_id_fixture)
{
    {
        Fred::OperationContext ctx;
        Fred::CreateDomainNameBlacklistId(test_domain_id, "successfully tested").exec(ctx);
        ctx.commit_transaction();
    }
    Fred::OperationContext ctx;
    Database::Result blacklist_result = ctx.get_conn().exec_params(
        "SELECT COUNT(*) "
        "FROM domain_blacklist "
        "WHERE regexp=$1::text AND "
              "valid_from<CURRENT_TIMESTAMP AND (CURRENT_TIMESTAMP<valid_to OR valid_to IS NULL)",
            Database::query_param_list(regexp_test_domain_fqdn));
    BOOST_CHECK(static_cast< unsigned >(blacklist_result[0][0]) == 1);
}

/**
 * test CreateDomainNameBlacklistIdBad
 * ...
 * calls in test should throw
 */
BOOST_FIXTURE_TEST_CASE(create_domain_name_blacklist_id_bad, create_domain_name_blacklist_id_fixture)
{
    Fred::ObjectId not_used_id;
    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        not_used_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec("SELECT (MAX(id)+1000)*2 FROM object_registry")[0][0]);
        Fred::CreateDomainNameBlacklistId(not_used_id, "must throw exception").exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_object_id_not_found());
        BOOST_CHECK(ex.get_object_id_not_found() == not_used_id);
    }

    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::CreateDomainNameBlacklistId(test_domain_id, "successfully tested").exec(ctx);
        Fred::CreateDomainNameBlacklistId(test_domain_id, "must throw exception").exec(ctx); // already_blacklisted_domain
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_already_blacklisted_domain());
        BOOST_CHECK(ex.get_already_blacklisted_domain() == test_domain_id);
    }

    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::CreateDomainNameBlacklistId(test_domain_id, "must throw exception",
          boost::posix_time::ptime(boost::gregorian::date(2006, 7, 31)),
          boost::posix_time::ptime(boost::gregorian::date(2005, 7, 31))).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_out_of_turn());
    }

    try {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::CreateDomainNameBlacklistId blacklist(test_domain_id, "must throw exception");
        blacklist.set_valid_to(boost::posix_time::ptime(boost::gregorian::date(2006, 7, 31)));
        blacklist.exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_out_of_turn());
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateDomainNameBlacklistId
