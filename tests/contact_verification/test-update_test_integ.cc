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
 *  integration tests for UpdateContactTest
 */

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/update_test.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "random_data_generator.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include <boost/foreach.hpp>

/* TODO - FIXME - only temporary for uuid mockup */
#include  <cstdlib>
#include "util/random_data_generator.h"

BOOST_AUTO_TEST_SUITE(TestUpdateContactTest_integ)

const std::string server_name = "test-contact_verification-update_test_integ";

typedef Fred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
typedef Fred::InfoContactCheckOutput::ContactTestResultData ContactTestResultData;
typedef Fred::InfoContactCheckOutput::ContactTestResultState ContactTestResultState;
typedef Fred::InfoContactCheckOutput InfoContactCheckOutput;
typedef Fred::InfoContactCheckOutput::ContactTestResultState ContactTestState;

struct eval {
    static void exec(const std::string& testname, const InfoContactCheckOutput& data_pre_update, const InfoContactCheckOutput& data_post_update, const std::string& old_status, const std::string& new_status, Optional<long long> old_logd_request, Optional<long long> new_logd_request, Optional<std::string> old_error_msg, Optional<std::string> new_error_msg) {
        // everything is the same except the last state in history
        BOOST_CHECK_EQUAL( data_pre_update.contact_history_id, data_post_update.contact_history_id );
        BOOST_CHECK_EQUAL( data_pre_update.handle, data_post_update.handle );
        BOOST_CHECK_EQUAL( data_pre_update.local_create_time, data_post_update.local_create_time );
        BOOST_CHECK_EQUAL( data_pre_update.testsuite_name, data_post_update.testsuite_name );
        BOOST_CHECK_EQUAL( data_pre_update.check_state_history.size(), data_post_update.check_state_history.size() );
        for(
            std::vector<ContactCheckState>::const_iterator it = data_pre_update.check_state_history.begin(), post_it = data_post_update.check_state_history.begin();
            it != data_pre_update.check_state_history.end();
            ++it, ++post_it)
        {
            BOOST_CHECK_MESSAGE(
                it->to_string() == post_it->to_string(),
                data_pre_update.to_string() + "\n" + data_post_update.to_string()
                );
        }

        BOOST_CHECK_EQUAL( data_pre_update.tests.size(), data_post_update.tests.size());
        std::vector<ContactTestResultData>::const_iterator post_it = data_post_update.tests.begin();
        for(std::vector<ContactTestResultData>::const_iterator it = data_pre_update.tests.begin(); it != data_pre_update.tests.end(); ++it, ++post_it) {
            if(it->test_name != testname) {
                BOOST_CHECK_EQUAL(it->to_string(), post_it->to_string());
            } else {
                if(old_status != new_status || old_logd_request != new_logd_request || old_error_msg.print_quoted() != new_error_msg.print_quoted() ) {

                    BOOST_CHECK_EQUAL( it->state_history.size() + 1, post_it->state_history.size());
                    for(
                        std::vector<ContactTestState>::const_iterator
                            pre_history_it = it->state_history.begin(),
                            post_history_it = post_it->state_history.begin();
                        pre_history_it != it->state_history.end();
                       ++pre_history_it, ++post_history_it
                    ) {
                       BOOST_CHECK_EQUAL(pre_history_it->to_string(), post_history_it->to_string() );
                    }

                    // new state in history
                    // update_time is reasonable
                    ptime now = second_clock::universal_time();
                    ptime update_time_min = now - minutes(1);
                    ptime update_time_max = now + minutes(1);

                    BOOST_CHECK_MESSAGE(
                        post_it->state_history.back().local_update_time > update_time_min,
                        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(post_it->state_history.back().local_update_time)
                        + " 'now' is:" + boost::posix_time::to_simple_string(now) );
                    BOOST_CHECK_MESSAGE(
                        post_it->state_history.back().local_update_time < update_time_max,
                        "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(post_it->state_history.back().local_update_time)
                        + " 'now' is:" + boost::posix_time::to_simple_string(now) );
                    BOOST_CHECK_EQUAL(post_it->state_history.back().status_name, new_status);
                    BOOST_CHECK_EQUAL(post_it->state_history.back().logd_request_id, new_logd_request);
                    BOOST_CHECK_EQUAL(post_it->state_history.back().error_msg.print_quoted(), new_error_msg.print_quoted());
                } else {
                    BOOST_CHECK_EQUAL( it->state_history.size(), post_it->state_history.size());
                    for(
                        std::vector<ContactTestState>::const_iterator
                            pre_history_it = it->state_history.begin(),
                            post_history_it = post_it->state_history.begin();
                        pre_history_it != it->state_history.end();
                        ++pre_history_it, ++post_history_it
                    ) {
                        BOOST_CHECK_EQUAL(pre_history_it->to_string(), post_history_it->to_string() );
                    }
                }
            }
        }
    }
};

struct fixture_create_check {
    std::string check_handle_;
    Fred::OperationContext& ctx;

    fixture_create_check(Fred::OperationContext& _ctx)
        : ctx(_ctx)
    {
        // registrar
        std::string registrar_handle = static_cast<std::string>(
            ctx.get_conn().exec("SELECT handle FROM registrar LIMIT 1;")[0][0] );

        BOOST_REQUIRE(registrar_handle.empty() != true);

        // contact
        std::string contact_handle = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6);
        Fred::CreateContact create_contact(contact_handle, registrar_handle);
        create_contact.exec(ctx);

        // testsuite
        std::string testsuite_name = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(6) + "_TESTSUITE_NAME";
        ctx.get_conn().exec(
            "INSERT INTO enum_contact_testsuite "
            "   (name, description)"
            "   VALUES ('"+testsuite_name+"', 'description some text')"
            "   RETURNING id;"
        );

        // check
        Fred::CreateContactCheck create_check(contact_handle, testsuite_name);
        check_handle_ = create_check.exec(ctx);
    }
};

struct fixture_create_testdef {
    long testdef_id_;
    std::string testdef_name_;
    std::string testdef_description_;
    Fred::OperationContext& ctx;

    fixture_create_testdef(Fred::OperationContext& _ctx)
        : ctx(_ctx)
    {
        testdef_name_ = "CREATE_CNT_TEST_" + RandomDataGenerator().xnumstring(6) + "_NAME";
        testdef_description_ = testdef_name_ + "_DESCRIPTION";
        testdef_id_ = static_cast<long>(
            ctx.get_conn().exec(
                "INSERT INTO enum_contact_test "
                "   (name, description) "
                "   VALUES ('"+testdef_name_+"', '"+testdef_description_+"') "
                "   RETURNING id;"
            )[0][0]);
    }
};

struct fixture_create_test : public fixture_create_check {
    std::string test_name_;
    std::string status_;
    Optional<long long> logd_request_;

    fixture_create_test(
        Fred::OperationContext& _ctx,
        std::string _test_name,
        std::string _status,
        Optional<long long> _logd_request
    ) :
        fixture_create_check(_ctx),
        test_name_(_test_name),
        status_(_status),
        logd_request_(_logd_request)
    {
        Fred::CreateContactTest create_test(check_handle_, test_name_, logd_request_);
        create_test.exec(ctx);
    }

};

struct fixture_create_test_update_test : public fixture_create_test {
    std::string old_status_;
    std::string new_status_;
    Optional<long long> old_logd_request_;
    Optional<long long> new_logd_request_;
    Optional<std::string> old_error_msg_;
    Optional<std::string> new_error_msg_;
    Fred::InfoContactCheckOutput data_pre_update_;
    Fred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    fixture_create_test_update_test(
        Fred::OperationContext& _ctx,
        const std::string& _test_name,
        const std::string& _new_status,
        Optional<long long> _old_logd_request,
        Optional<long long> _new_logd_request,
        Optional<std::string> _new_error_msg,
        const std::string& _timezone = "UTC"
    )
     : old_status_(Fred::ContactTestStatus::RUNNING),
       new_status_(_new_status),
       old_logd_request_(_old_logd_request),
       new_logd_request_(_new_logd_request),
       old_error_msg_(Optional<std::string>()),
       new_error_msg_(_new_error_msg),
       timezone_(_timezone),
       fixture_create_test(_ctx, _test_name, Fred::ContactTestStatus::RUNNING, _old_logd_request)
    {
        Fred::InfoContactCheck info_check(check_handle_);

        try {
            data_pre_update_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
           BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        Fred::UpdateContactTest update(check_handle_, test_name_, new_status_, new_logd_request_, new_error_msg_);
        try {
            update.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update test (3):") + exp.what());
        }

        try {
            data_post_update_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        eval::exec(test_name_, data_pre_update_, data_post_update_, old_status_, new_status_, old_logd_request_, new_logd_request_, old_error_msg_, new_error_msg_);
    }
};

struct fixture_create_update_update_check : public virtual fixture_create_test {
    std::string check_handle_;
    const std::string status1_;
    std::string status2_;
    std::string status3_;
    Optional<long long> logd_request1_;
    Optional<long long> logd_request2_;
    Optional<long long> logd_request3_;
    const Optional<std::string> error_msg1_;
    Optional<std::string> error_msg2_;
    Optional<std::string> error_msg3_;
    Fred::InfoContactCheckOutput data_post_create_;
    Fred::InfoContactCheckOutput data_post_reset_;
    Fred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    fixture_create_update_update_check(
        Fred::OperationContext& _ctx,
        const std::string& _test_name,
        const std::string& _status2,
        const std::string& _status3,
        Optional<long long> _logd_request1,
        Optional<long long> _logd_request2,
        Optional<long long> _logd_request3,
        Optional<std::string> _error_msg2,
        Optional<std::string> _error_msg3,
        const std::string& _timezone = "UTC"
    )
     : status1_(Fred::ContactCheckStatus::ENQUEUED),
       status2_(_status2),
       status3_(_status3),
       logd_request1_(_logd_request1),
       logd_request2_(_logd_request2),
       logd_request3_(_logd_request3),
       error_msg2_(_error_msg2),
       error_msg3_(_error_msg3),
       timezone_(_timezone),
       fixture_create_test(_ctx, _test_name, status1_, logd_request1_)
    {
        Fred::InfoContactCheck info_check(check_handle_);

        try {
            data_post_create_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
           BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        Fred::UpdateContactTest reset(check_handle_, test_name_, status2_, logd_request2_, error_msg2_);
        try {
            reset.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update test (3):") + exp.what());
        }

        try {
            data_post_reset_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        Fred::UpdateContactTest update(check_handle_, test_name_, status3_, logd_request3_, error_msg3_);
        try {
            update.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update test (3):") + exp.what());
        }

        try {
            data_post_update_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent test (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent test (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent test (3):") + exp.what());
        }

        eval::exec(test_name_, data_post_reset_, data_post_update_, status2_, status3_, logd_request2_, logd_request3_, error_msg2_, error_msg3_);
    }
};

struct fixture_create_nonexistent_check_handle {
    std::string check_handle;
    Fred::OperationContext& ctx;

    fixture_create_nonexistent_check_handle(Fred::OperationContext& _ctx)
        : ctx(_ctx)
    {
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
            res = ctx.get_conn().exec(
                "SELECT handle "
                "   FROM contact_check "
                "   WHERE handle='"+check_handle+"';"
            );
        } while(res.size() != 0);
    }
};

struct fixture_create_nonexistent_status_name {
    std::string status_name;
    Fred::OperationContext& ctx;

    fixture_create_nonexistent_status_name(Fred::OperationContext& _ctx)
        : ctx(_ctx)
    {
        Database::Result res;
        do {
            status_name = "STATUS_" + RandomDataGenerator().xnumstring(10) + "_TESTSUITE_NAME";
            res = ctx.get_conn().exec(
                "SELECT name "
                "   FROM enum_contact_check_status "
                "   WHERE name='"+status_name+"';" );
        } while(res.size() != 0);
    }
};

struct fixture_create_status {
    std::string status_name;
    Fred::OperationContext& ctx;

    fixture_create_status(Fred::OperationContext& _ctx)
        : ctx(_ctx)
    {
        Database::Result res;
        status_name = "STATUS_" + RandomDataGenerator().xnumstring(10) + "_TESTSUITE_NAME";
        res = ctx.get_conn().exec(
            "INSERT "
            "   INTO enum_contact_check_status "
            "   (id, name, description ) "
            "   VALUES (" + RandomDataGenerator().xnumstring(6) + ", '"+status_name+"', '"+status_name+"_desc') "
            "   RETURNING id;" );

        BOOST_REQUIRE(res.size()==1);
    }
};


/**
combinations of exec() with different original test state and different new values:
same/different status
same/different logd request (including NULL values)
same/different error message (including NULL values)
 */
BOOST_AUTO_TEST_CASE(test_Update)
{
    Fred::OperationContext ctx;

    fixture_create_status status1(ctx);
    fixture_create_status status2(ctx);

    std::vector<std::string> status_post_created;
    std::vector<boost::tuple<std::string, std::string, std::string> > status_post_reset;
    status_post_created.push_back(Fred::ContactTestStatus::RUNNING);
    status_post_reset.push_back(boost::make_tuple(Fred::ContactTestStatus::RUNNING, status1.status_name, status1.status_name));
    status_post_created.push_back(status1.status_name);
    status_post_reset.push_back(boost::make_tuple(Fred::ContactTestStatus::RUNNING, status1.status_name, status2.status_name));

    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id3 = RandomDataGenerator().xuint();
    std::vector<std::pair<Optional<long long>, Optional<long long> > > logd_request_post_created;
    std::vector<boost::tuple<Optional<long long>, Optional<long long>, Optional<long long> > > logd_request_post_reset;
    logd_request_post_created.push_back( std::make_pair( Optional<long long>(), Optional<long long>() ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, Optional<long long>() ) );
    logd_request_post_created.push_back( std::make_pair( Optional<long long>(), logd_request_id1 ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, logd_request_id1 ) );
    logd_request_post_created.push_back( std::make_pair( logd_request_id1, logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(Optional<long long>(), Optional<long long>(), Optional<long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, Optional<long long>(), Optional<long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, Optional<long long>() ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, Optional<long long>(), logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, logd_request_id2 ) );
    logd_request_post_reset.push_back( boost::make_tuple(logd_request_id1, logd_request_id2, logd_request_id3 ) );

    Optional<std::string> error_msg_id1 = RandomDataGenerator().xnstring(20);
    Optional<std::string> error_msg_id2 = RandomDataGenerator().xnstring(20);
    std::vector<Optional<std::string> > error_msg_post_created;
    std::vector<std::pair<Optional<std::string>, Optional<std::string> > > error_msg_post_reset;
    error_msg_post_created.push_back( Optional<std::string>() );
    error_msg_post_created.push_back( error_msg_id1 );
    error_msg_post_reset.push_back( std::make_pair(Optional<std::string>(), Optional<std::string>() ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, error_msg_id2 ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, Optional<std::string>() ) );
    error_msg_post_reset.push_back( std::make_pair(error_msg_id1, error_msg_id1 ) );
    error_msg_post_reset.push_back( std::make_pair(Optional<std::string>(), error_msg_id1 ) );

    typedef std::pair< Optional<long long>, Optional<long long> > logd_request_pair;

    BOOST_FOREACH(const std::string& status, status_post_created) {
        BOOST_FOREACH(logd_request_pair& logd_request, logd_request_post_created) {
            BOOST_FOREACH(const Optional<std::string>& error_msg, error_msg_post_created) {
                fixture_create_testdef def(ctx);
                fixture_create_test_update_test(ctx, def.testdef_name_, status, logd_request.first, logd_request.second, error_msg );
            }
        }
    }
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END();
