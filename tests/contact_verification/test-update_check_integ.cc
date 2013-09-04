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

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/update_check.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_check_status.h"
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

/* TODO - FIXME - only temporary for uuid mockup */
#include  <cstdlib>
#include "util/random_data_generator.h"

BOOST_AUTO_TEST_SUITE(TestUpdateContactCheck_integ)

const std::string server_name = "test-contact_verification-update_check_integ";

typedef Fred::InfoContactCheckOutput::ContactCheckState ContactCheckState;
typedef Fred::InfoContactCheckOutput::ContactTestResultData ContactTestResultData;
typedef Fred::InfoContactCheckOutput::ContactTestResultState ContactTestResultState;
typedef Fred::InfoContactCheckOutput InfoContactCheckOutput;

struct fixture_has_ctx {
    Fred::OperationContext ctx;
    ~fixture_has_ctx() {
        ctx.commit_transaction();
    }
};

struct eval {
    static void exec(const InfoContactCheckOutput& data_pre_update, const InfoContactCheckOutput& data_post_update, const std::string& old_status, const std::string& new_status, Optional<long long> old_logd_request, Optional<long long> new_logd_request) {
        // everything is the same except the last state in history
        BOOST_CHECK_EQUAL( data_pre_update.contact_history_id, data_post_update.contact_history_id );
        BOOST_CHECK_EQUAL( data_pre_update.handle, data_post_update.handle );
        BOOST_CHECK_EQUAL( data_pre_update.local_create_time, data_post_update.local_create_time );
        BOOST_CHECK_EQUAL( data_pre_update.testsuite_name, data_post_update.testsuite_name );
        {
            BOOST_CHECK_EQUAL( data_pre_update.tests.size(), data_post_update.tests .size());
            std::vector<ContactTestResultData>::const_iterator post_it = data_post_update.tests.begin();
            for(std::vector<ContactTestResultData>::const_iterator it = data_pre_update.tests.begin(); it != data_pre_update.tests.end(); ++it, ++post_it) {
                BOOST_CHECK_EQUAL(it->to_string(), post_it->to_string());
            }
        }

        if(old_status != new_status || old_logd_request != new_logd_request) {
            BOOST_CHECK_EQUAL( data_pre_update.check_state_history.size() + 1, data_post_update.check_state_history.size() );
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

            // new state in history
            // update_time is reasonable
            ptime now = second_clock::universal_time();
            ptime update_time_min = now - minutes(1);
            ptime update_time_max = now + minutes(1);

            BOOST_CHECK_MESSAGE(
                data_post_update.check_state_history.back().local_update_time > update_time_min,
                "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(data_post_update.check_state_history.back().local_update_time)
                + " 'now' is:" + boost::posix_time::to_simple_string(now) );
            BOOST_CHECK_MESSAGE(
                data_post_update.check_state_history.back().local_update_time < update_time_max,
                "invalid contact_check.create_time: " + boost::posix_time::to_simple_string(data_post_update.check_state_history.back().local_update_time)
                + " 'now' is:" + boost::posix_time::to_simple_string(now) );
            BOOST_CHECK_EQUAL(data_post_update.check_state_history.back().status_name, new_status);
            BOOST_CHECK_EQUAL(data_post_update.check_state_history.back().logd_request_id, new_logd_request);
        } else {
            BOOST_CHECK_EQUAL( data_pre_update.check_state_history.size(), data_post_update.check_state_history.size() );
            std::vector<ContactCheckState>::const_iterator post_it = data_post_update.check_state_history.begin();
            for(std::vector<ContactCheckState>::const_iterator it = data_pre_update.check_state_history.begin(); it != data_pre_update.check_state_history.end(); ++it, ++post_it) {
                BOOST_CHECK_EQUAL(it->to_string(), post_it->to_string());
            }
        }
    }
};

struct fixture_create_check : public virtual fixture_has_ctx {
    std::string check_handle_;
    Optional<long long> logd_request_;

    fixture_create_check(
        const std::string& _status,
        Optional<long long> _logd_request = Optional<long long>()
    )
        : logd_request_(_logd_request)
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
        Fred::CreateContactCheck create_check(contact_handle, testsuite_name, logd_request_);
        check_handle_ = create_check.exec(ctx);
    }
};

struct fixture_create_update_check : public fixture_create_check {
    std::string old_status_;
    std::string new_status_;
    Optional<long long> old_logd_request_;
    Optional<long long> new_logd_request_;
    Fred::InfoContactCheckOutput data_pre_update_;
    Fred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    fixture_create_update_check(
        const std::string& _new_status,
        Optional<long long> _old_logd_request,
        Optional<long long> _new_logd_request,
        const std::string& _timezone = "UTC"
    )
     : old_status_(Fred::ContactCheckStatus::ENQUEUED),
       old_logd_request_(_old_logd_request),
       fixture_create_check(Fred::ContactCheckStatus::ENQUEUED, _old_logd_request),
       new_status_(_new_status),
       new_logd_request_(_new_logd_request),
       timezone_(_timezone)
    {
        Fred::InfoContactCheck info_check(check_handle_);

        try {
            data_pre_update_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
           BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
        }

        Fred::UpdateContactCheck update(check_handle_, new_status_, new_logd_request_);
        try {
            update.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update check (3):") + exp.what());
        }

        try {
            data_post_update_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
        }

        eval::exec(data_pre_update_, data_post_update_, old_status_, new_status_, old_logd_request_, new_logd_request_);
    }
};

struct fixture_create_update_update_check : public virtual fixture_has_ctx {
    std::string check_handle_;
    std::string status1_;
    std::string status2_;
    std::string status3_;
    Optional<long long> logd_request1_;
    Optional<long long> logd_request2_;
    Optional<long long> logd_request3_;
    Fred::InfoContactCheckOutput data_post_create_;
    Fred::InfoContactCheckOutput data_post_reset_;
    Fred::InfoContactCheckOutput data_post_update_;
    std::string timezone_;

    fixture_create_update_update_check(
        const std::string& _status2,
        const std::string& _status3,
        Optional<long long> _logd_request1,
        Optional<long long> _logd_request2,
        Optional<long long> _logd_request3,
        const std::string& _timezone = "UTC"
    )
     : status1_(Fred::ContactCheckStatus::ENQUEUED),
       status2_(_status2),
       status3_(_status3),
       logd_request1_(_logd_request1),
       logd_request2_(_logd_request2),
       logd_request3_(_logd_request3),
       timezone_(_timezone)
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
        Fred::CreateContactCheck create_check(contact_handle, testsuite_name, logd_request1_);
        check_handle_ = create_check.exec(ctx);

        Fred::InfoContactCheck info_check(check_handle_);

        try {
            data_post_create_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
           BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
           BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
           BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
        }

        Fred::UpdateContactCheck reset(check_handle_, status2_, logd_request2_);
        try {
            reset.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update check (3):") + exp.what());
        }

        try {
            data_post_reset_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
        }

        Fred::UpdateContactCheck update(check_handle_, status3_, logd_request3_);
        try {
            update.exec(ctx);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("failed to update check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("failed to update check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("failed to update check (3):") + exp.what());
        }

        try {
            data_post_update_ = info_check.exec(ctx, timezone_);
        } catch(const Fred::InternalError& exp) {
            BOOST_FAIL("non-existent check (1):" + boost::diagnostic_information(exp) + exp.what() );
        } catch(const boost::exception& exp) {
            BOOST_FAIL("non-existent check (2):" + boost::diagnostic_information(exp));
        } catch(const std::exception& exp) {
            BOOST_FAIL(std::string("non-existent check (3):") + exp.what());
        }

        eval::exec(data_post_reset_, data_post_update_, status2_, status3_, logd_request2_, logd_request3_);
    }
};

struct fixture_create_nonexistent_check_handle : public virtual fixture_has_ctx {
    std::string check_handle;

    fixture_create_nonexistent_check_handle() {
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

struct fixture_create_nonexistent_status_name : public virtual fixture_has_ctx {
    std::string status_name;

    fixture_create_nonexistent_status_name() {
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

struct fixture_create_status : public virtual fixture_has_ctx {
    std::string status_name;

    fixture_create_status() {
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
 @pre handle of existing contact_check with status=X and logd_request=1
 @post correct values present in InfoContactCheck output
 @post no change in history values in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_request1_to_statusX_logd_request1)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id3 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::ENQUEUED,
        logd_request_id1, logd_request_id1);
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::RUNNING,
        logd_request_id2, logd_request_id3, logd_request_id3);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=1
 @pre existing status name Y different from status X set to check right now
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_request1_to_statusY_logd_request1)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id3 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::RUNNING,
        logd_request_id1, logd_request_id1);
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::TO_BE_DECIDED,
        logd_request_id2, logd_request_id3, logd_request_id3);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=1
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_request1_to_statusX_logd_requestNULL)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::ENQUEUED,
        logd_request_id1, Optional<long long>());
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::RUNNING,
        Optional<long long>(), logd_request_id2, Optional<long long>());
}

/**
 @pre handle of existing contact_check with status=X and logd_request=1
 @pre existing status name Y different from status X set to check right now
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_request1_to_statusY_logd_requestNULL)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::RUNNING,
        logd_request_id1, Optional<long long>());
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::TO_BE_DECIDED,
        Optional<long long>(), logd_request_id2, Optional<long long>());
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @pre valid logd request 1
 @post correct values present in InfoContactCheck output
 @post no change in history values in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_requestNULL_to_statusX_logd_request1)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id3 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::ENQUEUED,
        Optional<long long>(), logd_request_id1 );
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::RUNNING,
        logd_request_id2, Optional<long long>(), logd_request_id3);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @pre existing status name Y different from status X set to check right now
 @pre valid logd request 1
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_requestNULL_to_statusY_logd_request1)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id2 = RandomDataGenerator().xuint();
    Optional<long long> logd_request_id3 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::RUNNING,
        Optional<long long>(), logd_request_id1 );
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::TO_BE_DECIDED,
        logd_request_id2, Optional<long long>(), logd_request_id3);
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @post correct values present in InfoContactCheck output
 @post no change in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_requestNULL_to_statusX_logd_requestNULL)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::ENQUEUED,
        Optional<long long>(), Optional<long long>() );
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::RUNNING,
        logd_request_id1, Optional<long long>(), Optional<long long>());
}

/**
 @pre handle of existing contact_check with status=X and logd_request=NULL
 @pre existing status name Y different from status X set to check right now
 @post correct values present in InfoContactCheck output
 @post correct new record in history in InfoContactCheck output
 */
BOOST_AUTO_TEST_CASE(test_Update_statusX_logd_requestNULL_to_statusY_logd_requestNULL)
{
    Optional<long long> logd_request_id1 = RandomDataGenerator().xuint();

    fixture_create_update_check(
        Fred::ContactCheckStatus::RUNNING,
        Optional<long long>(), Optional<long long>() );
    fixture_create_update_update_check(
        Fred::ContactCheckStatus::RUNNING, Fred::ContactCheckStatus::TO_BE_DECIDED,
        logd_request_id1, Optional<long long>(), Optional<long long>());
}

struct fixture_nonexistent_check_handle_existing_status_name : public fixture_create_nonexistent_check_handle, fixture_create_status {};
/**
 setting nonexistent check handle and existing status values and executing operation
 @pre nonexistent check handle
 @pre existing status name
 @post ExceptionUnknownCheckHandle
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_check_handle, fixture_nonexistent_check_handle_existing_status_name)
{
    Fred::UpdateContactCheck dummy(check_handle, Fred::ContactCheckStatus::RUNNING);

    bool caught_the_right_exception = false;
    try {
        dummy.exec(ctx);
    } catch(const Fred::UpdateContactCheck::ExceptionUnknownCheckHandle& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

/**
 setting existing check handle and nonexistent status values and executing operation
 @pre existing check handle
 @pre nonexistent status name
 @post ExceptionUnknownStatusName
 */
BOOST_FIXTURE_TEST_CASE(test_Exec_nonexistent_status_name, fixture_create_nonexistent_status_name)
{
    fixture_create_status status;
    fixture_create_check check(status.status_name);

    Fred::UpdateContactCheck dummy(check.check_handle_, status_name);

    bool caught_the_right_exception = false;
    try {
        dummy.exec(ctx);
    } catch(const Fred::UpdateContactCheck::ExceptionUnknownStatusName& exp) {
        caught_the_right_exception = true;
    } catch(...) {
        BOOST_FAIL("incorrect exception caught");
    }

    if(! caught_the_right_exception) {
        BOOST_FAIL("should have caught the exception");
    }
}

BOOST_AUTO_TEST_SUITE_END();
