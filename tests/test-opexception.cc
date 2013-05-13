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

//declaration of exception tag related methods getter and chaining setter with error_info type
#define DECLARE_EXCEPTION_DATA(ex_data_tag) \
typedef boost::error_info<BOOST_JOIN(struct ExceptionTag_,ex_data_tag),std::string> BOOST_JOIN(ErrorInfo_,ex_data_tag);\
template <class DERIVED_EXCEPTION> struct BOOST_JOIN(ExceptionData_,ex_data_tag)\
{\
public:\
    typedef BOOST_JOIN(ErrorInfo_,ex_data_tag) error_info_type;\
private:\
    const std::string* get_str_ptr()\
    {\
        return boost::get_error_info<error_info_type>(*(static_cast<DERIVED_EXCEPTION*>(this)));\
    }\
public:\
    DERIVED_EXCEPTION& BOOST_JOIN(set_,ex_data_tag)(const std::string& arg)\
    {\
        DERIVED_EXCEPTION& ex = *static_cast<DERIVED_EXCEPTION*>(this);\
        ex << error_info_type(arg);\
        return ex;\
    }\
    std::string BOOST_JOIN(get_,ex_data_tag)()\
    {\
        const std::string* str_ptr = get_str_ptr();\
        return str_ptr ? *str_ptr : std::string("");\
    }\
    bool BOOST_JOIN(is_set_,ex_data_tag)()\
    {\
        const std::string* str_ptr = get_str_ptr();\
        return str_ptr;\
    }\
protected:\
    BOOST_JOIN(ExceptionData_,ex_data_tag)(){}\
    BOOST_JOIN(~ExceptionData_,ex_data_tag)() throw () {}\
}\



DECLARE_EXCEPTION_DATA(unknown_registrar_handle);
DECLARE_EXCEPTION_DATA(unknown_contact_handle);
DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier);

BOOST_AUTO_TEST_SUITE(TestOperationException)

const std::string server_name = "test-opexception";

///exception instance for tests
struct TestException
: virtual Fred::OperationException
  , ExceptionData_unknown_contact_handle<TestException>
  , ExceptionData_unknown_registrar_handle<TestException>
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
        throw;//to check std::exception
    }
    , std::exception
    , check_std_exception);
}

BOOST_AUTO_TEST_SUITE_END();//TestOperationException

