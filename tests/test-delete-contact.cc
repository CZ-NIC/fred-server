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
#include "fredlib/registrar.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/contact/info_contact_history.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/opexception.h"
#include "util/util.h"

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

BOOST_AUTO_TEST_SUITE(TestDeleteContact)

const std::string server_name = "test-delete-contact";


struct test_contact_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;

    test_contact_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle  = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(test_contact_handle,registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_fixture()
    {}
};

/**
 * test DeleteContact
 * create test contact, delete test contact, check erdate of test contact is null
 * calls in test shouldn't throw
 */
BOOST_FIXTURE_TEST_CASE(delete_contact, test_contact_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput contact_info1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    BOOST_CHECK(contact_info1.info_contact_data.delete_time.isnull());

    Fred::DeleteContact(test_contact_handle).exec(ctx);
    ctx.commit_transaction();

    std::vector<Fred::InfoContactOutput> contact_history_info1 = Fred::InfoContactHistory(
        contact_info1.info_contact_data.roid).exec(ctx);

    BOOST_CHECK(!contact_history_info1.at(0).info_contact_data.delete_time.isnull());

    Fred::InfoContactOutput contact_info1_with_change = contact_info1;
    contact_info1_with_change.info_contact_data.delete_time = contact_history_info1.at(0).info_contact_data.delete_time;

    BOOST_CHECK(contact_info1_with_change == contact_history_info1.at(0));

    BOOST_CHECK(!contact_history_info1.at(0).info_contact_data.delete_time.isnull());

    BOOST_CHECK(contact_history_info1.at(0).next_historyid.isnull());
    BOOST_CHECK(!contact_history_info1.at(0).history_valid_from.is_not_a_date_time());
    BOOST_CHECK(!contact_history_info1.at(0).history_valid_to.isnull());
    BOOST_CHECK(contact_history_info1.at(0).history_valid_from <= contact_history_info1.at(0).history_valid_to);

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        , Database::query_param_list(test_contact_handle))[0][0]));
}//delete_contact


/**
 * test DeleteContact with wrong handle
 */

BOOST_FIXTURE_TEST_CASE(delete_contact_with_wrong_handle, test_contact_fixture )
{
    std::string bad_test_contact_handle = std::string("bad")+test_contact_handle;
    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteContact(bad_test_contact_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_unknown_contact_handle());
        BOOST_CHECK(ex.get_unknown_contact_handle().compare(bad_test_contact_handle) == 0);
    }
}

/**
 * test DeleteContact linked
 */

BOOST_FIXTURE_TEST_CASE(delete_linked_contact, test_contact_fixture )
{
    {
        Fred::OperationContext ctx;
        //create linked object
        std::string test_nsset_handle = std::string("TEST-NSSET-HANDLE")+xmark;
        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(test_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
                (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3"))) //add_dns
                (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))) //add_dns
                ).exec(ctx);

       ctx.commit_transaction();
    }

    try
    {
        Fred::OperationContext ctx;//new connection to rollback on error
        Fred::DeleteContact(test_contact_handle).exec(ctx);
        ctx.commit_transaction();
    }
    catch(const Fred::DeleteContact::Exception& ex)
    {
        BOOST_CHECK(ex.is_set_object_linked_to_contact_handle());
        BOOST_CHECK(ex.get_object_linked_to_contact_handle().compare(test_contact_handle) == 0);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestDeleteContact
