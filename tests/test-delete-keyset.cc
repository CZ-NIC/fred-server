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

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "src/fredlib/registrar.h"
#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/keyset/delete_keyset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/opexception.h"
#include "util/util.h"

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

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestDeleteKeyset)

const std::string server_name = "test-delete-keyset";


struct delete_keyset_fixture
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
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
                .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        ctx.commit_transaction();
    }
    ~delete_keyset_fixture()
    {}
};

/**
 * test DeleteKeyset
 * create test keyset, delete test keyset, check erdate of test keyset is null
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(delete_keyset, delete_keyset_fixture )
{
    Fred::OperationContext ctx;

    Fred::InfoKeysetOutput keyset_info1 = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx);
    BOOST_CHECK(keyset_info1.info_keyset_data.delete_time.isnull());

    Fred::DeleteKeyset(test_keyset_handle).exec(ctx);
    ctx.commit_transaction();

    std::vector<Fred::InfoKeysetOutput> keyset_history_info1 = Fred::InfoKeysetHistory(
    keyset_info1.info_keyset_data.roid).exec(ctx);

    BOOST_CHECK(!keyset_history_info1.at(0).info_keyset_data.delete_time.isnull());

    Fred::InfoKeysetOutput keyset_info1_with_change = keyset_info1;
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

}//delete_keyset


/**
 * test DeleteKeyset with wrong handle
 */

BOOST_FIXTURE_TEST_CASE(delete_keyset_with_wrong_handle, delete_keyset_fixture )
{
    std::string bad_test_keyset_handle = std::string("bad")+test_keyset_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteKeyset(bad_test_keyset_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_keyset_handle());
        BOOST_CHECK(ex.get_unknown_keyset_handle().compare(bad_test_keyset_handle) == 0);
    }
}

/**
 * test DeleteKeyset linked
 */

BOOST_FIXTURE_TEST_CASE(delete_linked_keyset, delete_keyset_fixture )
{
    {
        Fred::OperationContext ctx;
        //create linked object

        Fred::CreateDomain(test_domain_fqdn //const std::string& fqdn
            , registrar_handle //const std::string& registrar
            , admin_contact_handle //registrant
            ).set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_keyset(test_keyset_handle)
            .exec(ctx);

       ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteKeyset(test_keyset_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteKeyset::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_linked_to_keyset_handle());
        BOOST_CHECK(ex.get_object_linked_to_keyset_handle().compare(test_keyset_handle) == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestDeleteKeyset
