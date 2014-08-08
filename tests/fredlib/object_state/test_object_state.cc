/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#include <exception>

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"
#include "random_data_generator.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"

/**
 *  @file
 *  test contact info
 */

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "tests/setup/fixtures.h"

BOOST_FIXTURE_TEST_SUITE(TestObjectState, Test::Fixture::instantiate_db_template)

const std::string server_name = "test-object-state";


struct test_contact_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;

    test_contact_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
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


BOOST_FIXTURE_TEST_CASE(get_object_states, test_contact_fixture )
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput contact_info1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<Fred::ObjectStateData> states;
    states = Fred::GetObjectStates(contact_info1.info_contact_data.id).exec(ctx);
    BOOST_CHECK(states.empty());
    Fred::CreateObjectStateRequestId(contact_info1.info_contact_data.id,
        Util::set_of<std::string>(Fred::ObjectState::MOJEID_CONTACT)).exec(ctx);
    Fred::PerformObjectStateRequest().set_object_id(contact_info1.info_contact_data.id).exec(ctx);

    states = Fred::GetObjectStates(contact_info1.info_contact_data.id).exec(ctx);
    BOOST_CHECK(states.at(0).state_name == Fred::ObjectState::MOJEID_CONTACT);
}

BOOST_AUTO_TEST_CASE(get_object_state_descriptions)
{
    Fred::OperationContext ctx;
    BOOST_CHECK(Fred::GetObjectStateDescriptions("EN").exec(ctx)[24]
        == "MojeID contact");
}

BOOST_AUTO_TEST_SUITE_END();//TestObjectState
