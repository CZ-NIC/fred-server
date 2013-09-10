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

BOOST_AUTO_TEST_SUITE(TestInfoContactCheck_integ)

const std::string server_name = "test-contact_verification-info_check_integ";

struct fixture_has_ctx {
    Fred::OperationContext ctx;
};

struct setup_check {
    std::string check_handle_;
    std::string status_;
    Optional<long long> logd_request_;

    setup_check(
        Fred::OperationContext& _ctx,
        Optional<long long> _logd_request = Optional<long long>()
    ) :
        status_(Fred::ContactCheckStatus::ENQUEUED),
        logd_request_(_logd_request)
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
        BOOST_REQUIRE(check_handle_.empty() != true );
    }
};

struct setup_check_status {
    std::string status_name_;

    setup_check_status(Fred::OperationContext& _ctx) {
        Database::Result res;
        status_name_ = "STATUS_" + RandomDataGenerator().xnumstring(10);
        res = _ctx.get_conn().exec(
            "INSERT "
            "   INTO enum_contact_check_status "
            "   (id, name, description ) "
            "   VALUES (" + RandomDataGenerator().xnumstring(6) + ", '"+status_name_+"', '"+status_name_+"_desc') "
            "   RETURNING id;" );

        BOOST_REQUIRE(res.size()==1);
    }
};

struct setup_test_status {
    std::string status_name_;

    setup_test_status(Fred::OperationContext& _ctx)
    {
        Database::Result res;
        status_name_ = "STATUS_" + RandomDataGenerator().xnumstring(10);
        res = _ctx.get_conn().exec(
            "INSERT "
            "   INTO enum_contact_test_status "
            "   (id, name, description ) "
            "   VALUES (" + RandomDataGenerator().xnumstring(6) + ", '"+status_name_+"', '"+status_name_+"_desc') "
            "   RETURNING id;" );

        BOOST_REQUIRE(res.size()==1);
    }
};

struct setup_logd_request_id {
    long long logd_request_id;

    setup_logd_request_id() {
        logd_request_id = RandomDataGenerator().xuint();
    }
};

struct setup_error_msg {
    std::string error_msg;

    setup_error_msg() {
        error_msg = "ERROR_MSG_" + RandomDataGenerator().xnumstring(20);
    }
};

struct setup_nonexistent_check_handle {
    std::string check_handle_;

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
            check_handle_ = boost::lexical_cast<std::string>(BOOST::UUIDS::RANDOM_GENERATOR::generate());
            res = _ctx.get_conn().exec(
                "SELECT handle "
                "   FROM contact_check "
                "   WHERE handle='"+check_handle_+"';"
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

/**
 setting existing check handle and executing operation
 @pre existing check handle with some tests and history
 @post correct data in InfoContactCheckOutput
 */
BOOST_FIXTURE_TEST_CASE(test_Exec, fixture_has_ctx)
{
    using std::map;
    using std::vector;
    using std::string;

    setup_check check(ctx);

    unsigned check_history_steps = 5;
    unsigned test_count = 5;
    vector<unsigned> tests_history_steps;
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
        check_status_history.push_back(setup_check_status(ctx).status_name_);
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
    for(int i=0; i<test_count; ++i) {
        test_names.push_back(setup_testdef(ctx).testdef_name_);
        tests_status_history.at(i).push_back(Fred::ContactTestStatus::RUNNING);
        tests_logd_request_history.at(i).push_back(Optional<long long>(setup_logd_request_id().logd_request_id));
        tests_error_msg_history.at(i).push_back(Optional<string>());

        setup_testdef_in_testsuite_of_check(ctx, test_names.at(i), check.check_handle_);

        Fred::CreateContactTest create_test(
            check.check_handle_,
            test_names.at(i),
            tests_logd_request_history.at(i).back() );

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
            tests_status_history.at(i).push_back(setup_test_status(ctx).status_name_);
            tests_logd_request_history.at(i).push_back(Optional<long long>(setup_logd_request_id().logd_request_id));
            tests_error_msg_history.at(i).push_back(Optional<string>(setup_error_msg().error_msg));

            Fred::UpdateContactTest update_test(
                check.check_handle_,
                test_names.at(i),
                tests_status_history.at(i).back(),
                tests_logd_request_history.at(i).back(),
                tests_error_msg_history.at(i).back() );

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
            BOOST_CHECK_EQUAL(tests_status_history.at(i).at(j), info.tests.at(i).state_history.at(j).status_name);
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
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_check_handle, fixture_has_ctx)
{
    setup_nonexistent_check_handle handle(ctx);

    Fred::InfoContactCheck dummy(handle.check_handle_);

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
