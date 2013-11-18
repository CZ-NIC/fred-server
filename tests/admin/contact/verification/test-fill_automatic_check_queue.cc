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
 *  integration tests for admin/contact/verification/fill_automatic_check_queue.cc
 */

#include "admin/contact/verification/run_all_enqueued_checks.h"
#include "admin/contact/verification/fill_automatic_check_queue.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/verification/enum_testsuite_name.h"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "tests/admin/contact/verification/setup_utils.h"
#include "tests/fredlib/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

namespace Test = Fred::ContactTestStatus;
namespace Check = Fred::ContactCheckStatus;

typedef std::vector< boost::tuple<std::string, long long, long long> > T_enq_ch;
typedef std::map<std::string, boost::shared_ptr<Admin::ContactVerificationTest> > T_testimpl_map;

void clean_queue() {
    Fred::OperationContext ctx;
    ctx.get_conn().exec_params(
        "DELETE FROM contact_test_result "
        "   WHERE enum_contact_test_status_id = "
        "       (SELECT id FROM enum_contact_test_status WHERE name=$1::varchar);",
        Database::query_param_list(Test::ENQUEUED) );

    ctx.get_conn().exec_params(
        "DELETE FROM contact_check "
        "   WHERE enum_contact_check_status_id = "
        "       (SELECT id FROM enum_contact_check_status WHERE name=$1::varchar);",
        Database::query_param_list(Check::ENQUEUED) );

    ctx.commit_transaction();
}

int get_queue_length() {
    Fred::OperationContext ctx;
    Database::Result res = ctx.get_conn().exec_params(
        "SELECT COUNT(id) AS count_ "
        "   FROM contact_check "
        "   WHERE enum_contact_check_status_id = "
        "       (SELECT id FROM enum_contact_check_status WHERE name=$1::varchar);",
        Database::query_param_list(Test::ENQUEUED) );

    if(res.size() != 1) {
        throw std::runtime_error("invalid query result");
    }
    return static_cast<int>(res[0]["count_"]);

}

void empty_automatic_testsuite() {
    Fred::OperationContext ctx;
    ctx.get_conn().exec_params(
        "DELETE FROM contact_testsuite_map "
        "   WHERE enum_contact_testsuite_id = "
        "       (SELECT id FROM enum_contact_testsuite WHERE name=$1::varchar);",
        Database::query_param_list(Fred::TestsuiteName::AUTOMATIC) );

    ctx.commit_transaction();
}

T_testimpl_map create_dummy_automatic_testsuite() {
    std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > test_impls;

    Fred::OperationContext ctx;
    boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr
        (new DummyTestReturning(Test::OK));

    test_impls[temp_ptr->get_name()] = temp_ptr;

    setup_testdef_in_testsuite(temp_ptr->get_name(), Fred::TestsuiteName::AUTOMATIC);
    ctx.commit_transaction();

    return test_impls;
}

struct setup_already_checked_contacts {

    int count_;
    std::vector<long long> ids_;

    setup_already_checked_contacts(int _count)
        : count_(_count)
    {
        // create contacts if necessary and enqueue those
        Fred::OperationContext ctx;

        int pre_existing_count = 0;
        Database::Result pre_existing_res = ctx.get_conn().exec(
            "SELECT o_r.id AS contact_id_ "
            "   FROM object_registry AS o_r "
            "       JOIN contact USING(id); ");
        for(Database::Result::Iterator it = pre_existing_res.begin(); it != pre_existing_res.end(); ++it) {
            ids_.push_back( static_cast<long long>( (*it)["contact_id_"] ) );
            pre_existing_count++;
        }

        for(int i=0; i < count_ - pre_existing_count; ++i) {
           setup_contact contact;
           ids_.push_back(contact.contact_id_);
        }

        clean_queue();
        empty_automatic_testsuite();

        T_testimpl_map dummy_testsuite = create_dummy_automatic_testsuite();
        std::vector<std::string> started_check_handles;
        for(int i=1; i <= count_; ++i) {
            T_enq_ch enqueued_checks = Admin::fill_automatic_check_queue(1);
            BOOST_CHECK_EQUAL( enqueued_checks.size(), 1);
            started_check_handles.push_back(
                Admin::run_all_enqueued_checks(dummy_testsuite).front()
            );
        }

        BOOST_CHECK_EQUAL( started_check_handles.size(), count_);
    }
};

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestFillAutomaticQueue, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification_integration-fill_automatic_check_queue";

/**
parameter of fill_automatic_check_queue must correctly affect number of newly enqueued checks
@pre clean queue
@post correct number of enqueued checks
 */
BOOST_AUTO_TEST_CASE(test_Max_queue_lenght_parameter)
{
    create_dummy_automatic_testsuite();
    T_enq_ch enqueued;

    enqueued = Admin::fill_automatic_check_queue(10);
    BOOST_CHECK_EQUAL(get_queue_length(), 10);
    BOOST_CHECK_EQUAL(enqueued.size(), 10);

    enqueued = Admin::fill_automatic_check_queue(30);
    BOOST_CHECK_EQUAL(get_queue_length(), 30);
    BOOST_CHECK_EQUAL(enqueued.size(), 20);

    enqueued = Admin::fill_automatic_check_queue(20);
    BOOST_CHECK_EQUAL(get_queue_length(), 30);
    BOOST_CHECK_EQUAL(enqueued.size(), 0);

}

/**
 when queue is full new checks mustn't be created
 @pre full queue (relative to max_queue_lenght value)
 @post no new checks enqueued
 */
BOOST_AUTO_TEST_CASE(test_Try_fill_full_queue)
{
    create_dummy_automatic_testsuite();

    T_enq_ch enqueued;

    enqueued = Admin::fill_automatic_check_queue(101);
    BOOST_CHECK_EQUAL(enqueued.size(), 101);

    enqueued = Admin::fill_automatic_check_queue(101);
    BOOST_CHECK_EQUAL(get_queue_length(), 101);
    BOOST_CHECK_EQUAL(enqueued.size(), 0);

    enqueued = Admin::fill_automatic_check_queue(101);
    BOOST_CHECK_EQUAL(get_queue_length(), 101);
    BOOST_CHECK_EQUAL(enqueued.size(), 0);
}

/**
 enqueuing never checked contacts first
 @pre empty check queue
 @pre set of already checked contacts
 @pre set of never checked contacts
 @post never checked contacts got enqueued for check first
 */

BOOST_AUTO_TEST_CASE(test_Enqueueing_never_checked_contacts)
{
    // testing logic - is going to be used repeatably
    struct nested {
        /**
         * will check if all enqueued checks relates to contact from never checked set AND...
         * removes contacts (for which checks are enqueued) from never_checked set
         */

        static void enqueued_in_never_checked(
            const T_enq_ch& _enqueued_checks,
            const std::vector<long long>& _never_checked_contacts_ids
        ) {
            bool is_enqueued;
            for(T_enq_ch::const_iterator it_enqueued = _enqueued_checks.begin(); it_enqueued != _enqueued_checks.end(); ++it_enqueued) {
                is_enqueued = false;

                for(std::vector<long long>::const_iterator it_never_ch = _never_checked_contacts_ids.begin(); it_never_ch != _never_checked_contacts_ids.end(); ++it_never_ch) {
                    if(it_enqueued->get<1>() == *it_never_ch) {
                        is_enqueued = true;
                    }
                }
                BOOST_CHECK_EQUAL(is_enqueued, true);
            }
        }

        static void update_never_checked(
            const T_enq_ch& _enqueued_checks,
            std::vector<long long>& _never_checked_contacts_ids
        ) {
            std::map<long long, int> never_checked_copy;
            for(std::vector<long long>::const_iterator it = _never_checked_contacts_ids.begin();
               it != _never_checked_contacts_ids.end();
               ++it) {
               never_checked_copy[*it] = 1;
            }

            for(T_enq_ch::const_iterator it_enqueued = _enqueued_checks.begin(); it_enqueued != _enqueued_checks.end(); ++it_enqueued) {
               never_checked_copy.erase(it_enqueued->get<1>());
            }


            std::vector<long long> result;
            for(std::map<long long, int>::const_iterator it = never_checked_copy.begin();
                it != never_checked_copy.end();
                ++it
            ) {
                result.push_back(it->first);
            }

            _never_checked_contacts_ids = result;
        }
    };

    setup_already_checked_contacts(50);

    // make set of new, never checked contacts
    std::vector<long long> never_checked_contacts;
    for(int i=0; i<50; ++i) {
        never_checked_contacts.push_back(setup_contact().contact_id_);
    }

    // test scenarios
    T_enq_ch enqueued_checks = Admin::fill_automatic_check_queue(10);
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Admin::fill_automatic_check_queue(20);
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Admin::fill_automatic_check_queue(30);
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Admin::fill_automatic_check_queue(40);
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Admin::fill_automatic_check_queue(50);
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Admin::fill_automatic_check_queue(50);
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 0);
}

/**
 enqueueing the oldest already checked contacts first
 @pre empty check queue
 @pre set of already checked contacts
 @post the oldest already checked contacts got enqueued for check first
 */
BOOST_AUTO_TEST_CASE(test_Enqueueing_already_checked_contacts)
{
    setup_already_checked_contacts(20);

    std::vector<long long> ids;

    Fred::OperationContext ctx;

    Database::Result already_checked_res = ctx.get_conn().exec_params(
        "SELECT o_r.id AS contact_id_, MAX(c_ch.update_time) AS last_update_ "
        "   FROM contact_check AS c_ch "
        "       JOIN object_history AS o_h ON c_ch.contact_history_id = o_h.historyid "
        "       JOIN object_registry AS o_r ON o_h.id = o_r.id "
        "   GROUP BY contact_id_ "
        "   ORDER by last_update_ ASC "
        "   LIMIT $1::integer ",
        Database::query_param_list(20) );

    BOOST_CHECK_EQUAL(already_checked_res.size(), 20);

    for(Database::Result::Iterator it = already_checked_res.begin(); it != already_checked_res.end(); ++it) {
        ids.push_back( static_cast<long long>( (*it)["contact_id_"] ) );
    }

    T_enq_ch enqueued_checks;
    std::vector<long long>::const_iterator it_checked = ids.begin();

    for(int i = 1; i<=20; ++i) {
        enqueued_checks.clear();
        enqueued_checks = Admin::fill_automatic_check_queue(i);
        BOOST_CHECK_EQUAL(enqueued_checks.size(), 1);
        BOOST_CHECK_EQUAL(enqueued_checks.back().get<1>(), *it_checked);

        ++it_checked;
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
