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
 *  integration tests for InfoContactCheck operation
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/update_check.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/update_test.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/optional_nullable_equal.h"
#include "random_data_generator.h"

#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"

/* TODO - FIXME - only temporary for uuid mockup */
#include  <cstdlib>
#include "util/random_data_generator.h"

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestInfoContactCheck_integ, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification-info_check_integ";

/**
 setting existing check handle and executing operation
 @pre existing check handle with some tests and history
 @post correct data in InfoContactCheckOutput
 */
BOOST_AUTO_TEST_CASE(test_Exec)
{
    using std::map;
    using std::vector;
    using std::string;

    Fred::OperationContext ctx;

    setup_check check;

    int check_history_steps = 5;
    int test_count = 5;
    vector<int> tests_history_steps;
    for(int i=0; i<test_count; ++i) {
        tests_history_steps.push_back(RandomDataGenerator().xnum1_6());
    }

    vector<string> check_status_history;
    vector<Optional<long long> > check_logd_request_history;

    vector<string> test_names;
    // strange type?
    //   map - because related to individial test...
    //   map<     vector<           >::iterator - ...because relation in map is given by test_names
    //                                                vector<             > - because HISTORY value
    vector<vector<string> >                 tests_status_history(test_count);
    vector<vector<Optional<long long> > >   tests_logd_request_history(test_count);
    vector<vector<Optional<string> > >      tests_error_msg_history(test_count);

    check_status_history.push_back( Fred::ContactCheckStatus::ENQUEUED);
    check_logd_request_history.push_back(Optional<long long>() );

    // building check history
    for(int i=1; i<check_history_steps; ++i) {
        check_status_history.push_back(setup_check_status().status_name_);
        check_logd_request_history.push_back(setup_logd_request_id().logd_request_id);

        Fred::UpdateContactCheck update_check(
            check.check_handle_, check_status_history.back(),
            check_logd_request_history.back() );
        try {
            update_check.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(string("failed to update check (3):") + exp.what());
        }
    }

    // building check tests
    // first test is already created in check by setup

    test_names.push_back(check.testsuite_.test.testdef_name_);

    tests_status_history.front().push_back(Fred::ContactTestStatus::ENQUEUED);
    tests_logd_request_history.front().push_back(Optional<long long>());
    tests_error_msg_history.front().push_back(Optional<string>());

    // starting from 1 because first history step is already CREATEd
    for(int j=1; j<tests_history_steps.at(0); ++j) {
        tests_status_history.at(0).push_back(setup_test_status().status_name_);
        tests_logd_request_history.at(0).push_back(Optional<long long>(setup_logd_request_id().logd_request_id));
        tests_error_msg_history.at(0).push_back(Optional<string>(setup_error_msg().error_msg));

        Fred::UpdateContactTest update_test(
            check.check_handle_,
            test_names.at(0),
            tests_status_history.at(0).at(j),
            tests_logd_request_history.at(0).at(j),
            tests_error_msg_history.at(0).at(j) );

        try {
            update_test.exec(ctx);
        } catch(const Fred::InternalError& exp) {
           BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(string("failed to update test (3):") + exp.what());
        }
    }
    for(int i=1; i<test_count; ++i) {
        test_names.push_back(setup_testdef().testdef_name_);
        tests_status_history.at(i).push_back(Fred::ContactTestStatus::ENQUEUED);
        tests_logd_request_history.at(i).push_back(Optional<long long>(setup_logd_request_id().logd_request_id));
        tests_error_msg_history.at(i).push_back(Optional<string>());

        setup_testdef_in_testsuite_of_check(test_names.at(i), check.check_handle_);

        Fred::CreateContactTest create_test(
            check.check_handle_,
            test_names.at(i),
            tests_logd_request_history.at(i).at(0) );

        try {
            create_test.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to create test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to create test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(string("failed to create test (3):") + exp.what());
        }

        // starting from 1 because first history step is already CREATEd
        for(int j=1; j<tests_history_steps.at(i); ++j) {
            tests_status_history.at(i).push_back(setup_test_status().status_name_);
            tests_logd_request_history.at(i).push_back(Optional<long long>(setup_logd_request_id().logd_request_id));
            tests_error_msg_history.at(i).push_back(Optional<string>(setup_error_msg().error_msg));

            Fred::UpdateContactTest update_test(
                check.check_handle_,
                test_names.at(i),
                tests_status_history.at(i).at(j),
                tests_logd_request_history.at(i).at(j),
                tests_error_msg_history.at(i).at(j) );

            try {
                update_test.exec(ctx);
            } catch(const Fred::InternalError& exp) {
               BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
            } catch(const boost::exception& exp) {
               BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
            } catch(const std::exception& exp) {
               BOOST_FAIL(string("failed to update test (3):") + exp.what());
            }
        }
    }

    Fred::InfoContactCheck info_op(check.check_handle_);
    Fred::InfoContactCheckOutput info;
    try {
        info = info_op.exec(ctx);
    } catch(const Fred::InternalError& exp) {
        BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(string("failed to update test (3):") + exp.what());
    }

    for(int i=0; i<check_history_steps; ++i) {
        BOOST_CHECK_EQUAL(check_status_history.at(i), info.check_state_history.at(i).status_name);
        BOOST_CHECK_MESSAGE(
            equal(
                check_logd_request_history.at(i),
                info.check_state_history.at(i).logd_request_id
            ),
            check_logd_request_history.at(i).print_quoted()
            + "\n differs from \n"
            + info.check_state_history.at(i).logd_request_id.print_quoted()
        );
    }

    for(int i=0; i<test_count; ++i) {
        for(int j=0; j<tests_history_steps.at(i); ++j) {
            BOOST_CHECK_EQUAL(
                tests_status_history.at(i).at(j),
                info.tests.at(i).state_history.at(j).status_name);
            BOOST_CHECK_MESSAGE(
                equal(
                    tests_logd_request_history.at(i).at(j),
                    info.tests.at(i).state_history.at(j).logd_request_id
                ),
                tests_logd_request_history.at(i).at(j).print_quoted()
                + "\n differs from \n"
                + info.tests.at(i).state_history.at(j).logd_request_id.print_quoted()
            );
            BOOST_CHECK_MESSAGE(
                equal(
                        tests_error_msg_history.at(i).at(j),
                    info.tests.at(i).state_history.at(j).error_msg
                ),
                tests_error_msg_history.at(i).at(j).print_quoted()
                + "\n differs from \n"
                + info.tests.at(i).state_history.at(j).error_msg.print_quoted()
            );
        }
    }
}

/**
 setting nonexistent check handle and executing operation
 @pre nonexistent check handle
 @post ExceptionUnknownCheckHandle
 */
BOOST_AUTO_TEST_CASE(test_Exec_nonexistent_check_handle)
{
    Fred::OperationContext ctx;

    setup_nonexistent_check_handle handle;

    Fred::InfoContactCheck dummy(handle.check_handle);

    bool caught_the_right_exception = false;
    try {
        dummy.exec(ctx);
    } catch(const Fred::InfoContactCheck::ExceptionUnknownCheckHandle& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
