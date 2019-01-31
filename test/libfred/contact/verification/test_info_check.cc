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

#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/update_test.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/db_settings.hh"
#include "util/optional_value.hh"
#include "util/is_equal_optional_nullable.hh"
#include "util/random_data_generator.hh"

#include "test/libfred/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/foreach.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestInfoContactCheck_integ, Test::instantiate_db_template)

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

    setup_testsuite suite;
    setup_check check(suite.testsuite_handle);

    int check_history_steps = 5;
    int test_count = 5;
    vector<int> tests_history_steps;
    for(int i=0; i<test_count; ++i) {
        tests_history_steps.push_back(RandomDataGenerator().xnum1_6());
    }

    vector<string> check_status_history;
    vector<Optional<unsigned long long> > check_logd_request_history;

    vector<string> test_handles;

    vector<vector<string> >                 tests_status_history(test_count);
    vector<vector<Optional<unsigned long long> > >   tests_logd_request_history(test_count);
    vector<vector<Optional<string> > >      tests_error_msg_history(test_count);

    check_status_history.push_back( ::LibFred::ContactCheckStatus::ENQUEUE_REQ);
    check_logd_request_history.push_back(Optional<unsigned long long>() );

    // building check history
    for(int i=1; i<check_history_steps; ++i) {
        check_status_history.push_back(setup_check_status().status_handle);
        check_logd_request_history.push_back(setup_logd_request_id().logd_request_id);

        ::LibFred::UpdateContactCheck update_check(
            uuid::from_string(check.check_handle_),
            check_status_history.back(),
            check_logd_request_history.back() );

        try {
            ::LibFred::OperationContextCreator ctx1;
            update_check.exec(ctx1);
            ctx1.commit_transaction();
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(string("exception (3):") + exp.what());
        }
    }

    // building check tests

    //test_handles.push_back(check.testsuite_handle_.test.testdef_handle_);
    BOOST_FOREACH (const setup_testdef& def, suite.testdefs) {
        test_handles.push_back(def.testdef_handle_);
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CreateContactTest(uuid::from_string(check.check_handle_), def.testdef_handle_).exec(ctx);
        ctx.commit_transaction();
    }

    tests_status_history.front().push_back(::LibFred::ContactTestStatus::ENQUEUED);
    tests_logd_request_history.front().push_back(Optional<unsigned long long>());
    tests_error_msg_history.front().push_back(Optional<string>());

    // starting from 1 because first history step is already CREATEd
    for(int j=1; j<tests_history_steps.at(0); ++j) {
        tests_status_history.at(0).push_back(setup_test_status().status_handle_);
        tests_logd_request_history.at(0).push_back(Optional<unsigned long long>(setup_logd_request_id().logd_request_id));
        tests_error_msg_history.at(0).push_back(Optional<string>(setup_error_msg().error_msg));

        ::LibFred::UpdateContactTest update_test(
            uuid::from_string(check.check_handle_),
            test_handles.at(0),
            tests_status_history.at(0).at(j),
            tests_logd_request_history.at(0).at(j),
            tests_error_msg_history.at(0).at(j) );

        try {
            ::LibFred::OperationContextCreator ctx2;
            update_test.exec(ctx2);
            ctx2.commit_transaction();
        } catch(const ::LibFred::InternalError& exp) {
           BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(string("exception (3):") + exp.what());
        }
    }
    for(int i=1; i<test_count; ++i) {
        test_handles.push_back(setup_testdef().testdef_handle_);
        tests_status_history.at(i).push_back(::LibFred::ContactTestStatus::ENQUEUED);
        tests_logd_request_history.at(i).push_back(Optional<unsigned long long>(setup_logd_request_id().logd_request_id));
        tests_error_msg_history.at(i).push_back(Optional<string>());

        setup_testdef_in_testsuite_of_check(test_handles.at(i), check.check_handle_);

        ::LibFred::CreateContactTest create_test(
            uuid::from_string(check.check_handle_),
            test_handles.at(i),
            tests_logd_request_history.at(i).at(0) );

        try {
            ::LibFred::OperationContextCreator ctx3;
            create_test.exec(ctx3);
            ctx3.commit_transaction();
        } catch(const ::LibFred::InternalError& exp) {
            BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(string("exception (3):") + exp.what());
        }

        // starting from 1 because first history step is already CREATEd
        for(int j=1; j<tests_history_steps.at(i); ++j) {
            tests_status_history.at(i).push_back(setup_test_status().status_handle_);
            tests_logd_request_history.at(i).push_back(Optional<unsigned long long>(setup_logd_request_id().logd_request_id));
            tests_error_msg_history.at(i).push_back(Optional<string>(setup_error_msg().error_msg));

            ::LibFred::UpdateContactTest update_test(
                uuid::from_string(check.check_handle_),
                test_handles.at(i),
                tests_status_history.at(i).at(j),
                tests_logd_request_history.at(i).at(j),
                tests_error_msg_history.at(i).at(j) );

            try {
                ::LibFred::OperationContextCreator ctx4;
                update_test.exec(ctx4);
                ctx4.commit_transaction();
            } catch(const ::LibFred::InternalError& exp) {
               BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
            } catch(const boost::exception& exp) {
               BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
            } catch(const std::exception& exp) {
               BOOST_FAIL(string("exception (3):") + exp.what());
            }
        }
    }

    ::LibFred::InfoContactCheck info_op( uuid::from_string( check.check_handle_) );
    ::LibFred::InfoContactCheckOutput info;
    try {
        ::LibFred::OperationContextCreator ctx5;
        info = info_op.exec(ctx5);
    } catch(const ::LibFred::InternalError& exp) {
        BOOST_FAIL("exception (1):" + boost::diagnostic_information(exp) + exp.what() );
    } catch(const boost::exception& exp) {
        BOOST_FAIL("exception (2):" + boost::diagnostic_information(exp));
    } catch(const std::exception& exp) {
        BOOST_FAIL(string("exception (3):") + exp.what());
    }

    for(int i=0; i<check_history_steps; ++i) {
        BOOST_CHECK_EQUAL(check_status_history.at(i), info.check_state_history.at(i).status_handle);
        BOOST_CHECK_MESSAGE(
            Util::is_equal(
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
                info.tests.at(i).state_history.at(j).status_handle);
            BOOST_CHECK_MESSAGE(
                Util::is_equal(
                    tests_logd_request_history.at(i).at(j),
                    info.tests.at(i).state_history.at(j).logd_request_id
                ),
                tests_logd_request_history.at(i).at(j).print_quoted()
                + "\n differs from \n"
                + info.tests.at(i).state_history.at(j).logd_request_id.print_quoted()
            );
            BOOST_CHECK_MESSAGE(
                Util::is_equal(
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
    setup_nonexistent_check_handle handle;

    ::LibFred::InfoContactCheck dummy( uuid::from_string( handle.check_handle) );

    bool caught_the_right_exception = false;
    try {
        ::LibFred::OperationContextCreator ctx1;
        dummy.exec(ctx1);
    } catch(const ::LibFred::ExceptionUnknownCheckHandle& exp) {
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
