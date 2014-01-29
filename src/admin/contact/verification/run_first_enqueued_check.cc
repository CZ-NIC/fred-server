#include "src/admin/contact/verification/run_first_enqueued_check.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/verification/enum_test_status.h"
#include "src/fredlib/contact/verification/create_test.h"
#include "src/fredlib/contact/verification/update_test.h"
#include "src/fredlib/contact/verification/create_check.h"
#include "src/fredlib/contact/verification/update_check.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include "src/fredlib/opexception.h"

#include <set>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

namespace  Admin {

    /**
     * Lock some contact_check with running status.
     * If no existing running check can be be locked check with status enqueued is looked for, it's status updated and locking is retried.
     * Iterates until some contact_check is locked succesfully or there is neither any lockable check with running status nor check with enqueued status.
     *
     * @return locked check id
     */
    static std::string lazy_get_locked_running_check(Fred::OperationContext& _ctx, Optional<long long> _logd_request_id);
    /**
     * Important side-effect: tests of this check are created in db with status enqueued.
     */
    static void update_some_enqueued_check_to_running(Optional<long long> _logd_request_id);

    /**
     * Lock some contact_test with running status related to given check.
     * If no existing running test can be be locked function tries to upgrade status of some existing test from enqueued to running
     * and locking is retried.
     * Iterates until some contact_test is locked succesfully or all tests of given check are already running (and can't be locked).
     *
     * @param _check_id check whose tests are tried to lock
     * @return locked test handle
     */
    static std::string lazy_get_locked_running_test(Fred::OperationContext& _ctx, const std::string& _check_handle, Optional<long long> _logd_request_id);
    static void update_some_enqueued_test_to_running(const std::string& _check_handle, Optional<long long> _logd_request_id);

    /**
     * Updating check status based on tests results
     */
    static std::string evaluate_check_status_after_tests_finished(std::vector<std::string> _test_statuses);

    // technicality - way how to "return" from nested function
    struct _ExceptionAllTestsAlreadyRunning : public std::exception {};
    struct ExceptionNoEnqueuedChecksAvailable : public std::exception {};

    /**
     * main function and the only one visible to the outer world
     */
    Optional<std::string> run_first_enqueued_check(const std::map<std::string, boost::shared_ptr<Admin::ContactVerificationTest> >& _tests, Optional<long long> _logd_request_id) {
        Fred::OperationContext ctx_locked_check;
        Optional<std::string> check_handle;
        try {
            check_handle = lazy_get_locked_running_check(ctx_locked_check, _logd_request_id);
        } catch (ExceptionNoEnqueuedChecksAvailable&) {
            return check_handle;
        }

        /* right now this is just a query for history_id
           as there is not much in check's history and no tests either it is acceptable */
        Fred::InfoContactCheckOutput check_info (
            Fred::InfoContactCheck(check_handle).exec(ctx_locked_check) );

        if(check_info.testsuite_handle == Fred::TestsuiteHandle::AUTOMATIC) {
            preprocess_automatic_check(check_handle);
        } else if(check_info.testsuite_handle == Fred::TestsuiteHandle::MANUAL) {
            preprocess_manual_check(check_handle);
        }

        std::vector<std::string> test_statuses;
        std::vector<Optional<std::string> > error_messages;
        ContactVerificationTest::T_run_result temp_result;

        try {
            while(true) {
                Fred::OperationContext ctx_locked_test;
                // can throw exception_locking_failed
                std::string test_handle = lazy_get_locked_running_test(ctx_locked_test, check_handle, _logd_request_id);

                try {
                    temp_result = _tests.at(test_handle)->run(check_info.contact_history_id);

                    test_statuses.push_back(temp_result.first);
                    error_messages.push_back(temp_result.second);

                    if( test_statuses.back() == Fred::ContactTestStatus::ENQUEUED
                        || test_statuses.back() == Fred::ContactTestStatus::RUNNING
                    ) {
                        throw Fred::InternalError("malfunction in implementation of test " + test_handle + ", run() returned bad status");
                    }
                } catch(...) {
                    ctx_locked_test.get_conn().exec("ROLLBACK;");

                    Fred::OperationContext ctx_testrun_error;
                    test_statuses.push_back(Fred::ContactTestStatus::ERROR);
                    error_messages.push_back(Optional<std::string>("exception in test implementation"));

                    Fred::UpdateContactTest(
                        check_handle,
                        test_handle,
                        test_statuses.back(),
                        _logd_request_id,
                        error_messages.back()
                    ).exec(ctx_testrun_error);

                    // TODO log problem
                    ctx_testrun_error.get_log();
                    ctx_testrun_error.commit_transaction();

                    // let it propagate so the check can be updated to reflect this situation
                    throw;
                }
                Fred::UpdateContactTest(
                    check_handle,
                    test_handle,
                    test_statuses.back(),
                    _logd_request_id,
                    error_messages.back()
                ).exec(ctx_locked_test);

                ctx_locked_test.commit_transaction();
            }
        } catch (_ExceptionAllTestsAlreadyRunning&) {
            // just breaking the loop to get to the end
            // definitely not re-throwing this exception
        } catch (...) {
            Fred::UpdateContactCheck update_operation(
                check_handle,
                evaluate_check_status_after_tests_finished(test_statuses)
            );

            if(_logd_request_id.isset()) {
                update_operation.set_logd_request_id(_logd_request_id.get_value());
            }

            update_operation.exec(ctx_locked_check);

            ctx_locked_check.commit_transaction();

            // not calling post_run_hooks because of the exception

            throw;
        }

        Fred::UpdateContactCheck update_operation(
            check_handle,
            evaluate_check_status_after_tests_finished(test_statuses)
        );

        if(_logd_request_id.isset()) {
            update_operation.set_logd_request_id(_logd_request_id.get_value());
        }

        update_operation.exec(ctx_locked_check);

        ctx_locked_check.commit_transaction();

        return check_handle;
    }

    std::string lazy_get_locked_running_check(Fred::OperationContext& _ctx, Optional<long long> _logd_request_id) {
        while(true) {
            Database::Result locked_check_res = _ctx.get_conn().exec_params(
                "SELECT c_ch.handle AS check_handle_ "
                "   FROM contact_check AS c_ch "
                "       JOIN enum_contact_check_status AS enum_status ON c_ch.enum_contact_check_status_id = enum_status.id "
                "   WHERE enum_status.handle = $1::varchar "
                "   LIMIT 1 "
                "   FOR UPDATE OF c_ch; ",
                Database::query_param_list(Fred::ContactCheckStatus::RUNNING)
            );

            if(locked_check_res.size() == 1) {
                return static_cast<std::string>(locked_check_res[0]["check_handle_"]);
            } else if(locked_check_res.size() == 0) {
                try {
                    update_some_enqueued_check_to_running(_logd_request_id);
                } catch(ExceptionNoEnqueuedChecksAvailable&) {
                    throw;
                }
                /* in next attempt to lock check (in next iteration) the chances are better
                 * because there is one more check available now */
            } else {
                throw Fred::InternalError("invalid count of returned records");
            }
        }
    }
    void update_some_enqueued_check_to_running(Optional<long long> _logd_request_id) {
        Fred::OperationContext ctx;

        Database::Result locked_check_res = ctx.get_conn().exec_params(
            "SELECT c_ch.handle AS handle_ "
            "   FROM contact_check AS c_ch "
            "   WHERE c_ch.enum_contact_check_status_id = "
            "       ( SELECT id FROM enum_contact_check_status WHERE handle = $1::varchar ) "
            "   LIMIT 1 "
            "   FOR UPDATE OF c_ch;",
            Database::query_param_list(Fred::ContactCheckStatus::ENQUEUED)
        );

        if(locked_check_res.size() != 1) {
            throw ExceptionNoEnqueuedChecksAvailable();
        }

        std::string check_handle(static_cast<std::string>( locked_check_res[0]["handle_"] ));

        Fred::UpdateContactCheck update_operation(
            check_handle,
            Fred::ContactCheckStatus::RUNNING);

        if(_logd_request_id.isset()) {
            update_operation.set_logd_request_id(_logd_request_id.get_value());
        }

        update_operation.exec(ctx);


        // instantiate tests in db

        Database::Result testhandles_res = ctx.get_conn().exec_params(
            "SELECT enum_test.handle            AS handle_ "
            "   FROM enum_contact_test          AS enum_test "
            "       JOIN contact_testsuite_map  AS c_map "
            "           ON enum_test.id = c_map.enum_contact_test_id "
            "       JOIN contact_check          AS check_ "
            "           ON check_.enum_contact_testsuite_id = c_map.enum_contact_testsuite_id "
            "   WHERE check_.handle=$1::uuid "
            "   ORDER by enum_test.id ASC; ",
            Database::query_param_list(check_handle)
        );
        if(testhandles_res.size() == 0) {
            throw Fred::InternalError(
                std::string("testsuite of check(id=")
                + check_handle
                + ") contains no tests");
        }

        for(Database::Result::Iterator it = testhandles_res.begin(); it != testhandles_res.end(); ++it) {
            Fred::CreateContactTest(
                check_handle,
                static_cast<std::string>( (*it)["handle_"] ),
                _logd_request_id
            ).exec(ctx);
        }

        ctx.commit_transaction();
    }

    std::string lazy_get_locked_running_test(Fred::OperationContext& _ctx, const std::string& _check_handle, Optional<long long> _logd_request_id) {
        while(true) {

            Database::Result locked_test_res = _ctx.get_conn().exec_params(
                "SELECT enum_test.handle AS test_handle_ "
                "   FROM contact_test_result AS c_t_r "
                "       JOIN enum_contact_test_status AS enum_status ON c_t_r.enum_contact_test_status_id = enum_status.id "
                "       JOIN contact_check AS c_ch ON c_t_r.contact_check_id = c_ch.id "
                "       JOIN enum_contact_test AS enum_test ON enum_test.id = c_t_r.enum_contact_test_id "
                "   WHERE c_ch.handle = $1::uuid "
                "       AND enum_status.handle = $2::varchar "
                "   LIMIT 1 "
                "   FOR UPDATE OF enum_test; ",
                Database::query_param_list
                    (_check_handle)
                    (Fred::ContactTestStatus::RUNNING)
            );

            if(locked_test_res.size() == 1) {
                return static_cast<std::string>(locked_test_res[0]["test_handle_"]);
            } else if(locked_test_res.size() == 0) {
                try {
                    update_some_enqueued_test_to_running(_check_handle, _logd_request_id);
                } catch(_ExceptionAllTestsAlreadyRunning&) {
                    throw;
                }
                /* in next attempt to lock test (in next iteration) the chances are better
                 * because there is one more test now */
            } else {
                throw Fred::InternalError("invalid count of returned records");
            }
        }
    }

    void update_some_enqueued_test_to_running(const std::string& _check_handle, Optional<long long> _logd_request_id) {
        Database::Result locked_testhandle_res;

        while(true) {
            Fred::OperationContext ctx;
            /* idea is to get test_handle which
             * a) is related to check
             * b) has status ENQUEUED
             */
            locked_testhandle_res = ctx.get_conn().exec_params(
                "SELECT enum_test.handle AS test_handle_ "
                "   FROM contact_test_result AS c_t_r "
                "       JOIN enum_contact_test_status AS enum_status "
                "           ON c_t_r.enum_contact_test_status_id = enum_status.id "
                "       JOIN enum_contact_test AS enum_test "
                "           ON c_t_r.enum_contact_test_id = enum_test.id "
                "       JOIN contact_check AS c_ch ON c_t_r.contact_check_id = c_ch.id "
                "   WHERE c_ch.handle = $1::uuid "
                "       AND enum_status.handle = $2::varchar "
                "   LIMIT 1 "
                "   FOR UPDATE OF c_t_r; ",
                Database::query_param_list
                    (_check_handle)
                    (Fred::ContactTestStatus::ENQUEUED)
            );

            if(locked_testhandle_res.size() == 0) {
                throw _ExceptionAllTestsAlreadyRunning();
            } else if(locked_testhandle_res.size() != 1) {
                throw Fred::InternalError("invalid count of locked tests ( >1)");
            } else {
                Fred::UpdateContactTest update_operation(
                    _check_handle,
                    static_cast<std::string>( locked_testhandle_res[0]["test_handle_"] ),
                    Fred::ContactTestStatus::RUNNING);

                if(_logd_request_id.isset()) {
                    update_operation.set_logd_request_id(_logd_request_id.get_value());
                }

                update_operation.exec(ctx);

                ctx.commit_transaction();

                break;
            }
        }
    }

    std::string evaluate_check_status_after_tests_finished(std::vector<std::string> _test_statuses) {

        bool has_some_ok = false;
        bool has_some_fail = false;
        bool has_some_error = false;
        bool has_some_skipped = false;  // mainly to prevent from counting SKIPPED as OTHER
        bool has_some_running = false;
        bool has_some_other = false;

        BOOST_FOREACH(const std::string& status, _test_statuses) {
            if(status == Fred::ContactTestStatus::OK) {
                has_some_ok = true;
            } else if(status == Fred::ContactTestStatus::FAIL) {
                has_some_fail = true;
            } else if(status == Fred::ContactTestStatus::SKIPPED) {
                has_some_skipped = true;
            } else if(status == Fred::ContactTestStatus::ERROR) {
                has_some_error = true;
            } else if(status == Fred::ContactTestStatus::RUNNING) {
                has_some_running = true;
            } else {
                has_some_other = true;
            }
        }

        /* NOTE: checking if has_some_ok == true handles the improbably but possible
         * case of empty test statuses vector
         * NOTE: skipped test is ok - if it shouldn't be ok, than FAIL or ERROR are more appropriate
         */
        if(has_some_ok && !has_some_fail && !has_some_error && !has_some_running && !has_some_other) {
            return Fred::ContactCheckStatus::AUTO_OK;
        } else if(has_some_fail && !has_some_error && !has_some_running && !has_some_other ) {
            return Fred::ContactCheckStatus::AUTO_FAIL;
        } else if(has_some_error) {
            return Fred::ContactCheckStatus::AUTO_TO_BE_DECIDED;
        } else if( !has_some_error && has_some_running ) {
            // NOTE: it's important not to set RUNNING - leads to endless loop
            return Fred::ContactCheckStatus::AUTO_TO_BE_DECIDED;
        } else {
            return Fred::ContactCheckStatus::AUTO_TO_BE_DECIDED;
        }
    }

    void preprocess_automatic_check(const std::string& _check_handle) {
        // in case of need feel free to express yourself...
    }

    void preprocess_manual_check(const std::string& _check_handle) {
        Fred::OperationContext ctx;

        Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(
            _check_handle
        ).exec(ctx);

        Fred::InfoContactOutput contact_info = Fred::HistoryInfoContactByHistoryid(
            check_info.contact_history_id
        ).exec(ctx);

        ctx.get_conn().exec("SAVEPOINT state_savepoint");

        // cancel one state at a time because when exception is thrown, all changes would be ROLLBACKed
        try {
            std::set<std::string> object_states_to_erase =
                boost::assign::list_of("contactInManualVerification");
            Fred::CancelObjectStateRequestId(
                contact_info.info_contact_data.id,
                object_states_to_erase
            ).exec(ctx);
            ctx.get_conn().exec("RELEASE SAVEPOINT state_savepoint");
            ctx.get_conn().exec("SAVEPOINT state_savepoint");
        } catch(Fred::CancelObjectStateRequestId::Exception& e) {
            // in case it throws from with unknown cause
            if(e.is_set_state_not_found() == false) {
                throw;
            } else {
                ctx.get_conn().exec("ROLLBACK TO state_savepoint");
            }
        }
        try {
            std::set<std::string> object_states_to_erase =
                boost::assign::list_of("manuallyVerifiedContact");

            Fred::CancelObjectStateRequestId(
                contact_info.info_contact_data.id,
                object_states_to_erase
            ).exec(ctx);

            ctx.get_conn().exec("RELEASE SAVEPOINT state_savepoint");
            ctx.get_conn().exec("SAVEPOINT state_savepoint");
        } catch(Fred::CancelObjectStateRequestId::Exception& e) {
            // in case it throws from with unknown cause
            if(e.is_set_state_not_found() == false) {
                throw;
            } else {
                ctx.get_conn().exec("ROLLBACK TO state_savepoint");
            }
        }

        std::set<std::string> status;
        status.insert("contactInManualVerification");
        Fred::CreateObjectStateRequestId(
            contact_info.info_contact_data.id,
            status
        ).exec(ctx);

        ctx.commit_transaction();
    }
}
