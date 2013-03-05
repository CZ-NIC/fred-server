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


/**
 * test DeleteContact
 * create test contact, delete test contact, check erdate of test contact is null
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(delete_contact)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);
    std::string handle = std::string("TEST-DEL-CONTACT-HANDLE")+xmark;

    Fred::CreateContact(handle,registrar_handle).set_name(std::string("TEST-DEL-CONTACT NAME")+xmark)
        .set_name(std::string("TEST-DEL-CONTACT NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    Fred::DeleteContact(handle).exec(ctx);
    ctx.commit_transaction();

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        ,Database::query_param_list(handle))[0][0]));
}//delete_contact


BOOST_AUTO_TEST_SUITE_END();//TestDeleteContact
