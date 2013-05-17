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
#include "fredlib/contact/merge_contact.h"
#include "fredlib/contact/merge_contact_selection.h"
#include "fredlib/contact/merge_contact_email_notification_data.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
#include "fredlib/keyset/create_keyset.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/keyset/info_keyset.h"
#include "fredlib/keyset/info_keyset_history.h"
#include "fredlib/keyset/info_keyset_compare.h"
#include "fredlib/nsset/info_nsset.h"
#include "fredlib/nsset/info_nsset_history.h"
#include "fredlib/nsset/info_nsset_compare.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"
#include "fredlib/domain/info_domain_compare.h"
#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/info_contact_history.h"
#include "fredlib/contact/info_contact_compare.h"


#include "util/util.h"

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

static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier, std::string);
DECLARE_EXCEPTION_DATA(testing_int_data, int);

BOOST_AUTO_TEST_SUITE(TestOperationException)

const std::string server_name = "test-opexception";

///exception instance for tests
struct TestException
: virtual Fred::OperationException
  , ExceptionData_unknown_contact_handle<TestException>
  , ExceptionData_unknown_registrar_handle<TestException>
  , ExceptionData_testing_int_data<TestException>
{};

BOOST_AUTO_TEST_CASE(throw_child)
{
    BOOST_CHECK_EXCEPTION(
    try
    {
        BOOST_THROW_EXCEPTION(
                TestException()
                .set_unknown_registrar_handle("test_registrar")
                .set_unknown_contact_handle("test_contact")
                .set_testing_int_data(5)
                << ErrorInfo_unknown_registry_object_identifier("test_roid")//add anything

                );
    }
    catch(boost::exception& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));

        BOOST_TEST_MESSAGE("\nwhen not interested in exception child type using error_info instance");
        const std::string* test_data_ptr = 0;
        test_data_ptr = boost::get_error_info<ErrorInfo_unknown_contact_handle>(ex);
        if(test_data_ptr)
        {
            BOOST_TEST_MESSAGE(*test_data_ptr);
            BOOST_CHECK(std::string("test_contact").compare(*test_data_ptr)==0);
        }
        test_data_ptr = boost::get_error_info<ErrorInfo_unknown_registrar_handle>(ex);
        if(test_data_ptr)
        {
            BOOST_TEST_MESSAGE(*test_data_ptr);
            BOOST_CHECK(std::string("test_registrar").compare(*test_data_ptr)==0);
        }

        BOOST_TEST_MESSAGE("\nwith known child exception type using wrapper");
        if(dynamic_cast<TestException&>(ex).is_set_unknown_contact_handle())
        {
            BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_contact_handle());
            BOOST_CHECK(std::string("test_contact").compare(dynamic_cast<TestException&>(ex).get_unknown_contact_handle())==0);
        }
        if(dynamic_cast<TestException&>(ex).is_set_unknown_registrar_handle())
        {
            BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle());
            BOOST_CHECK(std::string("test_registrar").compare(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle())==0);
        }
        if(dynamic_cast<TestException&>(ex).is_set_testing_int_data())
        {
            BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_testing_int_data());
            BOOST_CHECK(5 == dynamic_cast<TestException&>(ex).get_testing_int_data());
        }
        throw;//to check std::exception
    }
    , std::exception
    , check_std_exception);
}

BOOST_AUTO_TEST_SUITE_END();//TestOperationException

