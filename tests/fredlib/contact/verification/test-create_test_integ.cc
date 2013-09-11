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

/**
 *  @file
 *  integration tests for CreateContactTest operation
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "random_data_generator.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"

BOOST_AUTO_TEST_SUITE(TestCreateContactTest_integ)

const std::string server_name = "test-contact_verification-create_test_integ";

struct fixture_ctx {
    Fred::OperationContext ctx;
};

struct setup_check {
    std::string check_handle_;
    Optional<long long> logd_request_;

    setup_check(
        Fred::OperationContext& _ctx,
        Optional<long long> _logd_request = Optional<long long>()
    )
        : logd_request_(_logd_request)
    {
        // registrar
        std::string registrar_handle = static_cast<std::string>(
            _ctx.get_conn().exec("SELECT handle FROM registrar LIMIT 1;")[0][0] );

        BOOST_REQUIRE(registrar_handle.empty() != true);

        // contact
        std::string contact_handle = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6);
        Fred::CreateContact create_contact(contact_handle, registrar_handle);
        create_contact.exec(_ctx);

        // testsuite
        std::string testsuite_name = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6) + "_TESTSUITE_NAME";
        _ctx.get_conn().exec(
            "INSERT INTO enum_contact_testsuite "
            "   (name, description)"
            "   VALUES ('"+testsuite_name+"', 'description some text')"
            "   RETURNING id;"
        );

        // check
        Fred::CreateContactCheck create_check(contact_handle, testsuite_name, logd_request_);
        check_handle_ = create_check.exec(_ctx);
    }
};

struct setup_nonexistent_check_handle {
    std::string check_handle;

    setup_nonexistent_check_handle(Fred::OperationContext& _ctx) {
        struct BOOST { struct UUIDS { struct RANDOM_GENERATOR {
            static std::string generate() {
                srand(time(NULL));
                std::vector<unsigned char> bytes;

                // generate random 128bits = 16 bytes
                for (int i = 0; i < 16; ++i) {
                    bytes.push_back( RandomDataGenerator().xletter()%256 );
                }
                /* some specific uuid rules
                 * http://www.cryptosys.net/pki/Uuid.c.html
                 */
                bytes.at(6) = static_cast<char>(0x40 | (bytes.at(6) & 0xf));
                bytes.at(8) = static_cast<char>(0x80 | (bytes.at(8) & 0x3f));

                // buffer for hex representation of one byte + terminating zero
                char hex_rep[3];

                // converting raw bytes to hex string representation
                std::string result;
                for (std::vector<unsigned char>::iterator it = bytes.begin(); it != bytes.end(); ++it) {
                    sprintf(hex_rep,"%02x",*it);
                    // conversion target is hhhh - so in case it gets wrong just cut off the tail
                    hex_rep[2] = 0;
                    result += hex_rep;
                }

                // hyphens for canonical form
                result.insert(8, "-");
                result.insert(13, "-");
                result.insert(18, "-");
                result.insert(23, "-");

                return result;
            }
        }; }; };

        /* end of temporary ugliness - please cut and replace between ASAP*/

        Database::Result res;
        do {
            check_handle = boost::lexical_cast<std::string>(BOOST::UUIDS::RANDOM_GENERATOR::generate());
            res = _ctx.get_conn().exec(
                "SELECT handle "
                "   FROM contact_check "
                "   WHERE handle='"+check_handle+"';"
            );
        } while(res.size() != 0);
    }
};

struct setup_testdef {
    long testdef_id_;
    std::string testdef_name_;
    std::string testdef_description_;

    setup_testdef(Fred::OperationContext& _ctx) {
        testdef_name_ = "CREATE_CNT_TEST_" + RandomDataGenerator().xnumstring(6) + "_NAME";
        testdef_description_ = testdef_name_ + "_DESCRIPTION";
        testdef_id_ = static_cast<long>(
            _ctx.get_conn().exec(
                "INSERT INTO enum_contact_test "
                "   (name, description) "
                "   VALUES ('"+testdef_name_+"', '"+testdef_description_+"') "
                "   RETURNING id;"
            )[0][0]);
    }
};

struct setup_testdef_in_testsuite_of_check {
    setup_testdef_in_testsuite_of_check(Fred::OperationContext& _ctx, const std::string testdef_name, const std::string check_handle) {
    BOOST_REQUIRE(
        _ctx.get_conn().exec(
            "INSERT INTO contact_testsuite_map "
            "   (enum_contact_test_id, enum_contact_testsuite_id) "
            "   VALUES ("
            "       (SELECT id FROM enum_contact_test WHERE name='"+testdef_name+"' ), "
            "       (SELECT enum_contact_testsuite_id FROM contact_check WHERE handle='"+check_handle+"') "
            "   ) "
            "   RETURNING enum_contact_test_id;"
        ).size() == 1);
    }
};

struct setup_nonexistent_testdef_name {
    std::string testdef_name;

    setup_nonexistent_testdef_name(Fred::OperationContext& _ctx) {
        Database::Result res;
        do {
            testdef_name = "CREATE_CNT_TEST_" + RandomDataGenerator().xnumstring(10) + "_TEST_NAME";
            res = _ctx.get_conn().exec(
                "SELECT name FROM enum_contact_testsuite WHERE name='"+testdef_name+"';" );
        } while(res.size() != 0);
    }
};

struct setup_logd_request_id {
    long long logd_request_id;

    setup_logd_request_id() {
        logd_request_id = RandomDataGenerator().xuint();
    }
};

/**
 executing CreateContactCheck with only mandatory setup
 @pre valid contact handle
 @pre existing test name
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_mandatory_setup, fixture_ctx)
{
    setup_check check(ctx);
    setup_testdef testdef(ctx);
    setup_testdef_in_testsuite_of_check(ctx, testdef.testdef_name_, check.check_handle_);

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name_);
    std::string timezone = "UTC";

    try {
        create_test.exec(ctx);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to create test (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to create test (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("failed to create test (3):") + exp.what());
    }

    Fred::InfoContactCheck info_check(check.check_handle_);
    Fred::InfoContactCheckOutput result_data;

    try {
        result_data = info_check.exec(ctx, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
    }

    BOOST_REQUIRE(result_data.tests.size() == 1);
    BOOST_CHECK_EQUAL(result_data.tests.front().test_name, testdef.testdef_name_);

    // create_time is reasonable
    ptime now = second_clock::universal_time();
    ptime create_time_min = now - minutes(1);
    ptime create_time_max = now + minutes(1);

    BOOST_CHECK_MESSAGE(
        result_data.tests.front().local_create_time > create_time_min,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_CHECK_MESSAGE(
        result_data.tests.front().local_create_time < create_time_max,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_REQUIRE(result_data.tests.front().state_history.size() == 1);
    BOOST_CHECK_EQUAL(
        result_data.tests.front().state_history.front().status_name,
        Fred::ContactTestStatus::RUNNING);
    BOOST_CHECK(result_data.tests.front().state_history.front().logd_request_id.isnull() );
    BOOST_CHECK(result_data.tests.front().state_history.front().error_msg.isnull() );
    BOOST_CHECK_EQUAL(
        result_data.tests.front().state_history.front().local_update_time,
        result_data.tests.front().local_create_time );
}

/**
 executing CreateContactCheck with full mandatory + optional setup
 @pre valid contact handle
 @pre existing test name
 @post correct values present in InfoContactCheckOutput::to_string()
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_optional_setup, fixture_ctx)
{
    setup_check check(ctx);
    setup_testdef testdef(ctx);
    setup_testdef_in_testsuite_of_check(ctx, testdef.testdef_name_, check.check_handle_);
    setup_logd_request_id logd_request;

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name_, logd_request.logd_request_id);
    std::string timezone = "UTC";

    try {
        create_test.exec(ctx);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to create test (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to create test (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("failed to create test (3):") + exp.what());
    }

    Fred::InfoContactCheck info_check(check.check_handle_);
    Fred::InfoContactCheckOutput result_data;

    try {
        result_data = info_check.exec(ctx, timezone);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
    }

    BOOST_REQUIRE(result_data.tests.size() == 1);
    BOOST_CHECK_EQUAL(result_data.tests.front().test_name, testdef.testdef_name_);

    // create_time is reasonable
    ptime now = second_clock::universal_time();
    ptime create_time_min = now - minutes(1);
    ptime create_time_max = now + minutes(1);

    BOOST_CHECK_MESSAGE(
        result_data.tests.front().local_create_time > create_time_min,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_CHECK_MESSAGE(
        result_data.tests.front().local_create_time < create_time_max,
        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(result_data.tests.front().local_create_time)
        + " 'now' is:" + boost::posix_time::to_simple_string(now) );

    BOOST_REQUIRE(result_data.tests.front().state_history.size() == 1);
    BOOST_CHECK_EQUAL(
        result_data.tests.front().state_history.front().status_name,
        Fred::ContactTestStatus::RUNNING);
    BOOST_CHECK_EQUAL(result_data.tests.front().state_history.front().logd_request_id, logd_request.logd_request_id);
    BOOST_CHECK(result_data.tests.front().state_history.front().error_msg.isnull() );
    BOOST_CHECK_EQUAL(
        result_data.tests.front().state_history.front().local_update_time,
        result_data.tests.front().local_create_time );
}

/**
 setting nonexistent check handle and existing status values and executing operation
 @pre nonexistent check handle
 @pre existing status name
 @post ExceptionUnknownCheckHandle
*/
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_check_handle, fixture_ctx)
{
    setup_nonexistent_check_handle check(ctx);
    setup_testdef testdef(ctx);

    Fred::CreateContactTest create_test(check.check_handle, testdef.testdef_name_);

    bool caught_the_right_exception = false;
    try {
        create_test.exec(ctx);
    } catch(const Fred::CreateContactTest::ExceptionUnknownCheckHandle& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting existing check handle and nonexistent test values and executing operation
 @pre existing check handle
 @pre nonexistent test name
 @post ExceptionUnknownTestName
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_test_name, fixture_ctx)
{
    setup_check check(ctx);
    setup_nonexistent_testdef_name testdef(ctx);

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name);

    bool caught_the_right_exception = false;
    try {
        create_test.exec(ctx);
    } catch(const Fred::CreateContactTest::ExceptionUnknownTestName& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting existing check handle and nonexistent test values and executing operation
 @pre existing check handle
 @pre existent test name not in testsuite of this check
 @post ExceptionUnknownTestName
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_test_name_not_in_suite, fixture_ctx)
{
    setup_check check(ctx);
    setup_testdef testdef(ctx);
    // deliberately OMITTING setup_testdef_in_testsuite_of_check(...)

    Fred::CreateContactTest create_test(check.check_handle_, testdef.testdef_name_);

    bool caught_the_right_exception = false;
    try {
        create_test.exec(ctx);
    } catch(const Fred::CreateContactTest::ExceptionTestNotInMyTestsuite& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

BOOST_AUTO_TEST_SUITE_END();
