/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  integration tests for admin/contact/verification/fill_check_queue.cc
 */

#include "src/backend/admin/contact/verification/fill_check_queue.hh"
#include "src/backend/admin/contact/verification/run_all_enqueued_checks.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "libfred/registrable_object/contact/verification/exceptions.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/list_checks.hh"
#include "libfred/registrable_object/contact/verification/list_enum_objects.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"
#include "libfred/registrable_object/contact/verification/update_test.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/delete_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_diff.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "test/backend/admin/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <memory>

namespace TestStatus = ::LibFred::ContactTestStatus;
namespace CheckStatus = ::LibFred::ContactCheckStatus;

typedef std::vector<Fred::Backend::Admin::Contact::Verification::Queue::enqueued_check> T_enq_ch;
typedef std::map<std::string, std::shared_ptr<Fred::Backend::Admin::Contact::Verification::Test> > T_testimpl_map;

void clean_queue() {
    std::string status_array =
        std::string("{")
        + boost::join(::LibFred::ContactCheckStatus::get_not_yet_resolved(), ",")
        + "}";

    ::LibFred::OperationContextCreator ctx;
    ctx.get_conn().exec_params(
        "DELETE "
        "   FROM contact_test_result_history AS test_history "
        "   WHERE "
        "       EXISTS ("
        "           SELECT * "
        "               FROM "
        "                   contact_test_result AS test_ "
        "                   JOIN contact_check AS c_ch "
        "                       ON test_.contact_check_id = c_ch.id "
        "                   JOIN enum_contact_check_status AS enum_status "
        "                       ON c_ch.enum_contact_check_status_id = enum_status.id "
        "               WHERE test_history.contact_test_result_id = test_.id "
        "                   AND enum_status.handle = ANY($1::varchar[]) "
        "       )",
        Database::query_param_list
            (status_array)
        );

    ctx.get_conn().exec_params(
        "DELETE "
        "   FROM contact_test_result AS test_ "
        "   WHERE "
        "       EXISTS ("
        "           SELECT * "
        "               FROM "
        "                   contact_check AS c_ch "
        "                   JOIN enum_contact_check_status AS enum_status "
        "                       ON c_ch.enum_contact_check_status_id = enum_status.id "
        "               WHERE test_.contact_check_id = c_ch.id "
        "                   AND enum_status.handle = ANY($1::varchar[]) "
        "       )",
        Database::query_param_list
            (status_array)
        );

    ctx.get_conn().exec_params(
        "DELETE "
        "   FROM contact_check_history AS check_history "
        "   WHERE "
        "       EXISTS ("
        "           SELECT * "
        "               FROM "
        "                   contact_check AS c_ch "
        "                   JOIN enum_contact_check_status AS enum_status "
        "                       ON c_ch.enum_contact_check_status_id = enum_status.id "
        "               WHERE check_history.contact_check_id = c_ch.id "
        "                   AND enum_status.handle = ANY($1::varchar[]) "
        "       )",
        Database::query_param_list
            (status_array)
        );

    ctx.get_conn().exec_params(
        "DELETE FROM contact_check AS c_ch "
        "   WHERE "
        "       EXISTS ("
        "           SELECT * "
        "               FROM "
        "                   enum_contact_check_status AS enum_status "
        "               WHERE c_ch.enum_contact_check_status_id = enum_status.id "
        "                   AND enum_status.handle = ANY($1::varchar[]) "
        "       )",
        Database::query_param_list
            (status_array)
        );

    ctx.commit_transaction();
}

int get_queue_length() {
    std::string status_array =
        std::string("{")
        + boost::join(::LibFred::ContactCheckStatus::get_not_yet_resolved(), ",")
        + "}";

    ::LibFred::OperationContextCreator ctx;
    Database::Result res = ctx.get_conn().exec_params(
        "SELECT COUNT(c_ch.id) AS count_ "
        "   FROM contact_check AS c_ch "
        "       JOIN enum_contact_check_status AS enum_status "
        "           ON c_ch.enum_contact_check_status_id = enum_status.id "
        "   WHERE "
        "       enum_status.handle = ANY($1::varchar[])",
        Database::query_param_list
            (status_array)
    );

    if(res.size() != 1) {
        throw std::runtime_error("invalid query result");
    }
    return static_cast<int>(res[0]["count_"]);

}

void empty_automatic_testsuite() {
    ::LibFred::OperationContextCreator ctx;
    ctx.get_conn().exec_params(
        "DELETE FROM contact_testsuite_map "
        "   WHERE enum_contact_testsuite_id = "
        "       (SELECT id FROM enum_contact_testsuite WHERE handle=$1::varchar);",
        Database::query_param_list(::LibFred::TestsuiteHandle::AUTOMATIC) );

    ctx.commit_transaction();
}

T_testimpl_map create_dummy_automatic_testsuite() {
    std::map< std::string, std::shared_ptr<Fred::Backend::Admin::Contact::Verification::Test> > test_impls;

    ::LibFred::OperationContextCreator ctx;
    std::shared_ptr<Fred::Backend::Admin::Contact::Verification::Test> temp_ptr
        (new DummyTestReturning(TestStatus::OK));

    std::string handle = dynamic_cast<DummyTestReturning*>(temp_ptr.get())->get_handle();

    test_impls[handle] = temp_ptr;

    setup_testdef_in_testsuite(handle, ::LibFred::TestsuiteHandle::AUTOMATIC);
    ctx.commit_transaction();

    return test_impls;
}

struct setup_already_checked_contacts {

    int count_;
    std::vector<unsigned long long> ids_;

    setup_already_checked_contacts(int _count)
        : count_(_count)
    {
        // create contacts if necessary and enqueue those
        ::LibFred::OperationContextCreator ctx;

        int pre_existing_count = 0;
        Database::Result pre_existing_res = ctx.get_conn().exec(
            "SELECT o_r.id AS contact_id_ "
            "   FROM object_registry AS o_r "
            "       JOIN contact USING(id); ");
        for(Database::Result::Iterator it = pre_existing_res.begin(); it != pre_existing_res.end(); ++it) {
            ids_.push_back( static_cast<unsigned long long>( (*it)["contact_id_"] ) );
            pre_existing_count++;
        }

        for(int i=0; i < count_ - pre_existing_count; ++i) {
           Test::contact contact;
           ids_.push_back(contact.info_data.id);
        }

        clean_queue();
        empty_automatic_testsuite();

        T_testimpl_map dummy_testsuite = create_dummy_automatic_testsuite();
        std::vector<std::string> started_check_handles;
        for(int i=1; i <= count_; ++i) {
            T_enq_ch enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 1).exec();
            BOOST_CHECK_EQUAL( enqueued_checks.size(), 1);
            started_check_handles.push_back(
                run_all_enqueued_checks(dummy_testsuite).front()
            );
            ::LibFred::OperationContextCreator ctx;
            ::LibFred::UpdateContactCheck(
                uuid::from_string(*started_check_handles.rbegin()),
                ::LibFred::ContactCheckStatus::OK
            )
            .exec(ctx);
            ctx.commit_transaction();
        }

        BOOST_CHECK_EQUAL( started_check_handles.size(), count_);
    }
};

void create_check_for_all_unchecked_contacts(const std::string& testsuite_handle) {
    ::LibFred::OperationContextCreator ctx;

    setup_testdef testdef;
    setup_testdef_in_testsuite(testdef.testdef_handle_, testsuite_handle);

    Database::Result never_checked_contacts_res = ctx.get_conn().exec(
        "SELECT o_r.id AS contact_id_ "
        "   FROM contact AS c "
        "       JOIN object_registry AS o_r USING(id) "
        "   WHERE NOT EXISTS ( "
        "       SELECT * "
        "           FROM contact_history AS c_h "
        "           JOIN contact_check AS c_ch ON c_ch.contact_history_id = c_h.historyid "
        "       WHERE o_r.id = c_h.id "
        "   ) "
    );

    for(Database::Result::Iterator it = never_checked_contacts_res.begin();
        it != never_checked_contacts_res.end();
        ++it
    ) {
        ::LibFred::OperationContextCreator ctx1;
        std::string handle = ::LibFred::CreateContactCheck(
            static_cast<unsigned long long>( (*it)["contact_id_"] ),
            testsuite_handle
        )
        .exec(ctx1);
        ctx1.commit_transaction();

        ::LibFred::OperationContextCreator ctx2;
        ::LibFred::UpdateContactCheck(
            uuid::from_string(handle),
            ::LibFred::ContactCheckStatus::OK
        )
        .exec(ctx2);
        ctx2.commit_transaction();
    }

}

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestFillAutomaticQueue, Test::instantiate_db_template)

const std::string server_name = "test-contact_verification_integration-fill_automatic_check_queue";

/**
parameter of fill_automatic_check_queue must correctly affect number of newly enqueued checks
@pre clean queue
@post correct number of enqueued checks
 */
BOOST_AUTO_TEST_CASE(test_Max_queue_length_parameter)
{
    for(int i=0; i<100; ++i) {
        Test::contact contact;
    }

    create_dummy_automatic_testsuite();
    T_enq_ch enqueued;

    int queue_length = 0;
    enqueued = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 10).exec();
    // number of enqueued check is exact, if there some check resolved as ignored has no effect
    BOOST_CHECK_EQUAL(enqueued.size(), 10);
    queue_length = get_queue_length();
    BOOST_CHECK_MESSAGE(queue_length <= 10, "check queue too long");

    enqueued = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 30).exec();
    BOOST_CHECK_EQUAL(enqueued.size(), 30 - queue_length);
    queue_length = get_queue_length();
    BOOST_CHECK_MESSAGE(queue_length <= 30, "check queue too long");

    enqueued = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 20).exec();
    BOOST_CHECK_EQUAL(enqueued.size(), std::max(20 - queue_length, 0));
    BOOST_CHECK_MESSAGE(get_queue_length() <= 30, "check queue too long");
}

/**
 when queue is full new checks mustn't be created
 @pre full queue (relative to max_queue_length value)
 @post no new checks enqueued
 */
BOOST_AUTO_TEST_CASE(test_Try_fill_full_queue)
{
    for(int i=0; i<25; ++i) {
        Test::contact contact;
    }

    create_dummy_automatic_testsuite();

    T_enq_ch enqueued;

    int queue_length = 0;
    enqueued = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 20).exec();
    BOOST_CHECK_EQUAL(enqueued.size(), 20);
    queue_length = get_queue_length();
    BOOST_CHECK_MESSAGE(queue_length <= 20, "check queue too long");

    enqueued = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 11).exec();
    BOOST_CHECK_MESSAGE(get_queue_length() <= 20, "check queue too long");
    BOOST_CHECK_EQUAL(enqueued.size(), std::max(11 - queue_length, 0));
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
            const std::vector<unsigned long long>& _never_checked_contacts_ids
        ) {
            bool is_enqueued;
            for(T_enq_ch::const_iterator it_enqueued = _enqueued_checks.begin(); it_enqueued != _enqueued_checks.end(); ++it_enqueued) {
                is_enqueued = false;

                for(std::vector<unsigned long long>::const_iterator it_never_ch = _never_checked_contacts_ids.begin(); it_never_ch != _never_checked_contacts_ids.end(); ++it_never_ch) {
                    if(it_enqueued->contact_id == *it_never_ch) {
                        is_enqueued = true;
                    }
                }
                BOOST_CHECK_EQUAL(is_enqueued, true);
            }
        }

        static void update_never_checked(
            const T_enq_ch& _enqueued_checks,
            std::vector<unsigned long long>& _never_checked_contacts_ids
        ) {
            std::map<unsigned long long, int> never_checked_copy;
            for(std::vector<unsigned long long>::const_iterator it = _never_checked_contacts_ids.begin();
               it != _never_checked_contacts_ids.end();
               ++it) {
               never_checked_copy[*it] = 1;
            }

            for(T_enq_ch::const_iterator it_enqueued = _enqueued_checks.begin(); it_enqueued != _enqueued_checks.end(); ++it_enqueued) {
               never_checked_copy.erase(it_enqueued->contact_id);
            }


            std::vector<unsigned long long> result;
            for(std::map<unsigned long long, int>::const_iterator it = never_checked_copy.begin();
                it != never_checked_copy.end();
                ++it
            ) {
                result.push_back(it->first);
            }

            _never_checked_contacts_ids = result;
        }
    };

    create_dummy_automatic_testsuite();

    create_check_for_all_unchecked_contacts(::LibFred::TestsuiteHandle::AUTOMATIC);

    const unsigned already_checked_count = 50;

    setup_already_checked_contacts tmp(already_checked_count);

    // make set of new, never checked contacts
    std::vector<unsigned long long> never_checked_contacts;
    for(int i=0; i<100; ++i) {
        never_checked_contacts.push_back(Test::contact().info_data.id);
    }

    // test scenarios
    T_enq_ch enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 10).exec();
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 20).exec();
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 30).exec();
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 40).exec();
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 50).exec();
    BOOST_CHECK_EQUAL(enqueued_checks.size(), 10);
    nested::enqueued_in_never_checked(enqueued_checks, never_checked_contacts);
    nested::update_never_checked(enqueued_checks, never_checked_contacts);

    enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 50).exec();
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
    // IMPORTANT: check the rest of them AFTER - otherwise their checks would be OLDER!!!
    setup_already_checked_contacts(20);

    create_check_for_all_unchecked_contacts(::LibFred::TestsuiteHandle::AUTOMATIC);

    std::vector<unsigned long long> ids;

    ::LibFred::OperationContextCreator ctx;

    Database::Result oldest_checked_res = ctx.get_conn().exec_params(
        "SELECT o_r.id AS contact_id_, MAX(c_ch.update_time) AS last_update_ "
        "   FROM contact_check AS c_ch "
        "       JOIN object_history AS o_h ON c_ch.contact_history_id = o_h.historyid "
        "       JOIN object_registry AS o_r ON o_h.id = o_r.id "
        "       JOIN enum_contact_testsuite AS enum_c_t ON c_ch.enum_contact_testsuite_id = enum_c_t.id "
        "   WHERE enum_c_t.handle='" + ::LibFred::TestsuiteHandle::AUTOMATIC + "' "
        "   GROUP BY contact_id_ "
        "   ORDER by last_update_ ASC "
        "   LIMIT $1::integer ",
        Database::query_param_list(20) );

    BOOST_CHECK_EQUAL(oldest_checked_res.size(), 20);

    for(Database::Result::Iterator it = oldest_checked_res.begin(); it != oldest_checked_res.end(); ++it) {
        ids.push_back( static_cast<unsigned long long>( (*it)["contact_id_"] ) );
    }

    T_enq_ch enqueued_checks;
    std::vector<unsigned long long>::const_iterator it_checked = ids.begin();

    for(int i = 1; i<=20; ++i) {
        enqueued_checks.clear();
        enqueued_checks = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, i).exec();
        BOOST_CHECK_EQUAL(enqueued_checks.size(), 1);
        BOOST_CHECK_EQUAL(enqueued_checks.back().contact_id, *it_checked);

        ++it_checked;
    }
}

struct setup_special_contact {
    enum allowed_states {
        validated, serverBlocked, mojeid
    };
    std::map<allowed_states, std::string> state_names_;

    enum allowed_roles {
        technical, owner, none
    };

    std::string contact_handle_;
    Test::registrar registrar_;
    ::LibFred::InfoContactOutput data_;
    unsigned long long contact_id_;

    Optional<std::string> country_code_;
    Optional<allowed_states> state_;
    Optional<allowed_roles> role_;

    setup_special_contact(
        Optional<std::string> _country_code,
        Optional<allowed_states> _state,
        Optional<allowed_roles> _role
    );
};

void setup_contact_as_technical(const setup_special_contact& contact_) {
    // create nsset
    std::string nsset_handle_;
    // prevent handle collisions
    while(true) {
        try {
            ::LibFred::OperationContextCreator ctx1;

            nsset_handle_ = "NSSET_" + RandomDataGenerator().xnumstring(15);
            ::LibFred::CreateNsset(nsset_handle_, contact_.registrar_.info_data.handle).exec(ctx1);

            ctx1.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        } catch (::LibFred::InternalError& ) {
            continue;
        }
        break;
    }

    // set contact as technical
    ::LibFred::OperationContextCreator ctx2;
    ::LibFred::UpdateNsset(nsset_handle_, contact_.registrar_.info_data.handle)
        .add_tech_contact(contact_.contact_handle_).exec(ctx2);
    ctx2.commit_transaction();
}

void setup_contact_as_owner(const setup_special_contact& contact_) {
    // create domain with contact as owner
    std::string domain_fqdn_;
    // prevent name collisions
    while(true) {
        try {
            ::LibFred::OperationContextCreator ctx1;

            domain_fqdn_ = "DOMAIN" + RandomDataGenerator().xnumstring(15) + ".cz";

            ::LibFred::CreateDomain(domain_fqdn_, contact_.registrar_.info_data.handle, contact_.contact_handle_).exec(ctx1);

            ctx1.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        } catch (::LibFred::InternalError& ) {
            continue;
        }
        break;
    }
}

void setup_contact_in_state(const setup_special_contact& contact_, const std::string& state, ::LibFred::OperationContext& ctx) {
    ::LibFred::CreateObjectStateRequestId(contact_.contact_id_, boost::assign::list_of(state)).exec(ctx);
    ::LibFred::PerformObjectStateRequest(contact_.contact_id_).exec(ctx);
}

setup_special_contact::setup_special_contact(
    Optional<std::string> _country_code,
    Optional<allowed_states> _state,
    Optional<allowed_roles> _role
) :
    country_code_(_country_code),
    state_(_state),
    role_(_role)
{
    state_names_[validated] = "validatedContact";
    state_names_[serverBlocked]    = "serverBlocked";
    state_names_[mojeid]    = "mojeidContact";

    // prevent name collisions
    while(true) {
        try {
            ::LibFred::OperationContextCreator ctx;

            contact_handle_ = "CONTACT_" + RandomDataGenerator().xnumstring(10);
            ::LibFred::CreateContact create(contact_handle_, registrar_.info_data.handle);
            if(country_code_.isset()) {
                ::LibFred::Contact::PlaceAddress place;
                place.country = country_code_.get_value_or_default();
                create.set_place(place);
            }
            create.exec(ctx);
            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        } catch (::LibFred::InternalError& ) {
            continue;
        }
        break;
    }

    ::LibFred::OperationContextCreator ctx;
    data_ = ::LibFred::InfoContactByHandle(contact_handle_)
        .exec(ctx);

    contact_id_ = static_cast<unsigned long long>(
        ctx.get_conn().exec(
            "SELECT id "
            "   FROM contact "
            "   JOIN object_registry AS o_r USING(id) "
            "   WHERE o_r.name='" + contact_handle_ + "' "
        )[0][0]);

    if(state_.isset()) {
        setup_contact_in_state(*this, state_names_[state_.get_value()], ctx);
    }

    ctx.commit_transaction();

    if(role_.isset()) {
        if(role_.get_value() == technical) {
            setup_contact_as_technical(*this);
        } else if(role_.get_value() == owner) {
            setup_contact_as_owner(*this);
        }
    }
}

void process_filtered_contacts_testcase(
    const std::vector<
            boost::tuple<
                std::string,
                setup_special_contact::allowed_states,
                setup_special_contact::allowed_roles,
                bool> >& testcases,
    const Fred::Backend::Admin::Contact::Verification::Queue::contact_filter& filter
) {
    typedef setup_special_contact ssc;

    typedef std::string string;

    typedef std::vector<
        boost::tuple<
            string,
            ssc::allowed_states,
            ssc::allowed_roles,
            bool> > // should be enqueued?
        Ttestdata;

    std::vector<std::shared_ptr<setup_special_contact> > test_contacts;
    std::vector<unsigned long long> contacts_to_be_enqueued;
    for(Ttestdata::const_iterator it = testcases.begin();
        it != testcases.end();
        ++it
    ) {
        test_contacts
            .push_back(
                std::make_shared<TestContactVerification::TestFillAutomaticQueue::setup_special_contact>(
                    setup_special_contact( (*it).get<0>(), (*it).get<1>(), (*it).get<2>() )
        ));

        if( (*it).get<3>() ) {
            contacts_to_be_enqueued.push_back(
                (*test_contacts.rbegin())->contact_id_
            );
        }
    }

    T_enq_ch enqueued_contacts;
    enqueued_contacts = Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(::LibFred::TestsuiteHandle::AUTOMATIC, 10)
        .set_contact_filter(filter)
        .exec();

    std::set<unsigned long long> unique_enqueued_contacts;
    for(T_enq_ch::const_iterator it = enqueued_contacts.begin();
        it != enqueued_contacts.end();
        ++it
    ) {
        unique_enqueued_contacts.insert(it->contact_id);
    }

    BOOST_CHECK_EQUAL(contacts_to_be_enqueued.size(), unique_enqueued_contacts.size());

    for(std::set<unsigned long long>::const_iterator it = unique_enqueued_contacts.begin();
        it != unique_enqueued_contacts.end();
        ++it
    ) {
        BOOST_CHECK_MESSAGE(
            std::find(
                contacts_to_be_enqueued.begin(),
                contacts_to_be_enqueued.end(),
                *it
            ) != contacts_to_be_enqueued.end(),
            "wrong contacts are enqueued"
        );

    }
}

typedef setup_special_contact ssc;
typedef std::string string;
typedef std::vector<
    boost::tuple<
        string,
        ssc::allowed_states,
        ssc::allowed_roles,
        bool> > // should be enqueued?
    T_filtered_contacts_testdata;

/**
 enqueuing contacts correctly according to given filter criteria
 @pre set of contacts with different countries
 @post correct contacts enqueued in correct order (incl. special case - empty set)
 */
BOOST_AUTO_TEST_CASE(test_Enqueueing_filtered_contacts_country)
{
    create_dummy_automatic_testsuite();

    T_filtered_contacts_testdata testdata;

    // country is intentionaly different than 'cz' so post build testdata don't interfere
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,   ssc::none,  false));
    testdata.push_back(boost::make_tuple("JP", ssc::serverBlocked,   ssc::none,  true));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,   ssc::none,  false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,   ssc::none,  false));

    Fred::Backend::Admin::Contact::Verification::Queue::contact_filter filter;
    filter.country_code = "JP";

    process_filtered_contacts_testcase(testdata, filter);
}

/**
 enqueuing contacts correctly according to given filter criteria
 @pre set of contacts with different countries
 @post correct contacts enqueued in correct order (incl. special case - empty set)
 */
BOOST_AUTO_TEST_CASE(test_Enqueueing_filtered_contacts_state)
{
    T_filtered_contacts_testdata testdata;

    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,      ssc::owner,  false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,      ssc::owner,  false));
    testdata.push_back(boost::make_tuple("DE", ssc::validated,   ssc::owner,  true));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,      ssc::owner,  false));

    Fred::Backend::Admin::Contact::Verification::Queue::contact_filter filter;
    filter.states.insert("validatedContact");
    filter.country_code = "DE";

    process_filtered_contacts_testcase(testdata, filter);
}

/**
 enqueuing contacts correctly according to given filter criteria
 @pre set of contacts with different countries
 @post correct contacts enqueued in correct order (incl. special case - empty set)
 */
BOOST_AUTO_TEST_CASE(test_Enqueueing_filtered_contacts_role)
{
    T_filtered_contacts_testdata testdata;

    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,   ssc::owner,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,   ssc::owner,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,   ssc::owner,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,   ssc::technical,   true));

    Fred::Backend::Admin::Contact::Verification::Queue::contact_filter filter;
    filter.roles.insert(Fred::Backend::Admin::Contact::Verification::Queue::tech_c);
    filter.country_code = "DE";

    process_filtered_contacts_testcase(testdata, filter);
}

/**
 enqueuing contacts correctly according to given filter criteria
 @pre set of contacts with different properties
 @post correct contacts enqueued in correct order (incl. special case - empty set)
 */
BOOST_AUTO_TEST_CASE(test_Enqueueing_filtered_contacts_combined)
{
    T_filtered_contacts_testdata testdata;

    testdata.push_back(boost::make_tuple("DE", ssc::validated,      ssc::owner,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::validated,      ssc::owner,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::validated,      ssc::owner,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,         ssc::owner,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::serverBlocked,         ssc::owner,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::serverBlocked,         ssc::owner,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::mojeid,         ssc::owner,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::mojeid,         ssc::owner,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::mojeid,         ssc::owner,       false));

    testdata.push_back(boost::make_tuple("DE", ssc::validated,      ssc::technical,       false));
    // this one should be enqueued
    testdata.push_back(boost::make_tuple("JP", ssc::validated,      ssc::technical,       true));
    testdata.push_back(boost::make_tuple("SK", ssc::validated,      ssc::technical,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,         ssc::technical,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::serverBlocked,         ssc::technical,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::serverBlocked,         ssc::technical,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::mojeid,         ssc::technical,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::mojeid,         ssc::technical,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::mojeid,         ssc::technical,       false));

    testdata.push_back(boost::make_tuple("DE", ssc::validated,      ssc::none,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::validated,      ssc::none,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::validated,      ssc::none,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::serverBlocked,         ssc::none,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::serverBlocked,         ssc::none,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::serverBlocked,         ssc::none,       false));
    testdata.push_back(boost::make_tuple("DE", ssc::mojeid,         ssc::none,       false));
    testdata.push_back(boost::make_tuple("JP", ssc::mojeid,         ssc::none,       false));
    testdata.push_back(boost::make_tuple("SK", ssc::mojeid,         ssc::none,       false));

    Fred::Backend::Admin::Contact::Verification::Queue::contact_filter filter;
    filter.roles.insert(Fred::Backend::Admin::Contact::Verification::Queue::tech_c);
    filter.country_code = "JP";
    filter.states.insert("validatedContact");

    process_filtered_contacts_testcase(testdata, filter);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
