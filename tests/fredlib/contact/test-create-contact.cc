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
#include "src/fredlib/contact/merge_contact.h"
#include "src/fredlib/contact/merge_contact_selection.h"
#include "src/fredlib/contact/merge_contact_email_notification_data.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/contact/info_contact.h"

#include "util/util.h"

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

static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

BOOST_AUTO_TEST_SUITE(TestCreateContact)

const std::string server_name = "test-create-contact";

struct create_contact_fixture
{
    std::string registrar_handle;
    std::string xmark;
    std::string create_contact_handle;

    create_contact_fixture()
    : xmark(RandomDataGenerator().xnumstring(6))
    , create_contact_handle(std::string("TEST-CREATE-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar
    }
    ~create_contact_fixture()
    {}
};

DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);

/**
 * test CreateContact with wrong registrar
 */
BOOST_FIXTURE_TEST_CASE(create_contact_wrong_registrar, create_contact_fixture)
{
    Fred::OperationContext ctx;

    std::string bad_registrar_handle = registrar_handle+xmark;
    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreateContact(create_contact_handle, bad_registrar_handle)
        .set_authinfo("testauthinfo")
        .set_logd_request_id(0)
        .exec(ctx);
    }
    catch(const Fred::CreateContact::Exception& ex)
    {
        ex << ErrorInfo_unknown_registrar_handle("modifying const EX& by operator<<");
        //ex.set_internal_error("unable to modify const EX& by setter - ok");
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        throw;
    }
    , std::exception
    , check_std_exception);
}

/**
 * test CreateContact with wrong ssntype
 */
BOOST_FIXTURE_TEST_CASE(create_contact_wrong_ssntype, create_contact_fixture)
{
    Fred::OperationContext ctx;
    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreateContact(create_contact_handle, registrar_handle)
        .set_authinfo("testauthinfo")
        .set_logd_request_id(0)
        .set_ssntype("BAD")
        .set_ssn("any")
        .exec(ctx);
    }
    catch(const Fred::CreateContact::Exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_unknown_ssntype());
        throw;
    }
    , std::exception
    , check_std_exception);
}


BOOST_AUTO_TEST_SUITE_END();//TestCreateContact

