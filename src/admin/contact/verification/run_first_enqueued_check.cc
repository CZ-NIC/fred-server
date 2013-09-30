#include "admin/contact/verification/run_first_enqueued_check.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/update_test.h"
#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/update_check.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/opexception.h"

#include <boost/foreach.hpp>

namespace  Admin {

    /**
     * Lock some contact_check with running status.
     * If no existing check can be be locked check with status enqueued is looked for, it's status updated and locking is retried.
     * Iterates until some contact_check is locked succesfully or there is neither any lockable check with running status nor check with enqueued status.
     *
     * @return locked check id
     */
    static std::string lazy_get_locked_running_check(Fred::OperationContext& _ctx);
    static void update_some_enqueued_check_to_running(void);

    /**
     * Lock some contact_test with enqueued status related to given check.
     * If no existing test can be be locked tries to create new test from testsuite of given check
     * (with testname not yet existing for given check_id.testsuite) and locking is retried.
     * Iterates until some contact_test is locked succesfully or all tests from testsuite of given check are created (and can't be locked).
     *
     * @param _check_id check whose tests are tried to lock (and whose missing tests are created if necessary)
     * @return locked test name
     */
    static std::string lazy_create_locked_enqueued_test(Fred::OperationContext& _ctx, const std::string& _check_handle);
    static void try_create_next_test(const std::string& _check_handle);

    /**
     * Updating check status based on tests results
     */
    static std::string evaluate_check_status_after_tests_finished(std::vector<std::string> _test_statuses);

    /**
     * Triggering possible related actions after check has finished.
     */
    static void post_run_hooks(const std::string& _check_handle);


    /**
     * main function and the only one visible to the outer world
     */
    std::string run_first_enqueued_check(const std::map<std::string, boost::shared_ptr<Admin::ContactVerificationTest> >& _tests) {
        Fred::OperationContext ctx_locked_check;
        std::string check_handle;
        try {
            check_handle = lazy_get_locked_running_check(ctx_locked_check);
        } catch (ExceptionNoEnqueuedChecksAvailable&) {
            throw;
        }

        /* right now this is just a query for history_id
           as there is not much in check's history and no tests either it is acceptable */
        Fred::InfoContactCheckOutput check_info (
            Fred::InfoContactCheck(check_handle).exec(ctx_locked_check) );

        std::vector<std::string> test_statuses;
        try {
            while(true) {
                Fred::OperationContext ctx_locked_test;
                std::string test_name;
                // can throw exception_locking_failed
                test_name = lazy_create_locked_enqueued_test(ctx_locked_test, check_handle);
                Fred::UpdateContactTest(check_handle, test_name, Fred::ContactTestStatus::RUNNING).exec(ctx_locked_test);
                ctx_locked_test.commit_transaction();

                try {
                    test_statuses.push_back(
                        _tests.at(test_name)->run(check_info.contact_history_id) );
                } catch(...) {
                    Fred::OperationContext ctx_testrun_error;
                    test_statuses.push_back(Fred::ContactTestStatus::ERROR);
                    Fred::UpdateContactTest(check_handle, test_name, test_statuses.back()).exec(ctx_testrun_error);
                    // TODO log problem
                    ctx_testrun_error.get_log();
                    ctx_testrun_error.commit_transaction();

                    // let it propagate so the check can be updated to reflect this situation
                    throw;
                }

                Fred::OperationContext ctx_post_testrun;
                Fred::UpdateContactTest(check_handle, test_name, test_statuses.back()).exec(ctx_post_testrun);
                ctx_post_testrun.commit_transaction();
            }
        } catch (ExceptionCheckTestsuiteFullyCreated&) {
            // just breaking the loop to get to the end
            // definitely not re-throwing this exception
        } catch (...) {
            Fred::UpdateContactCheck(
                        check_handle,
                        evaluate_check_status_after_tests_finished(test_statuses)
                    ).exec(ctx_locked_check);

            ctx_locked_check.commit_transaction();

            // not calling post_run_hooks because of the exception

            throw;
        }

        Fred::UpdateContactCheck(
            check_handle,
            evaluate_check_status_after_tests_finished(test_statuses)
        ).exec(ctx_locked_check);

        ctx_locked_check.commit_transaction();

        post_run_hooks(check_handle);

        return check_handle;
    }

    std::string lazy_get_locked_running_check(Fred::OperationContext& _ctx) {
        while(true) {
            Database::Result locked_check_res = _ctx.get_conn().exec_params(
                "SELECT c_ch.handle AS check_handle_ "
                "   FROM contact_check AS c_ch "
                "       JOIN enum_contact_check_status AS enum_status ON c_ch.enum_contact_check_status_id = enum_status.id "
                "   WHERE enum_status.name = $1::varchar "
                "   LIMIT 1 "
                "   FOR UPDATE; ",
                Database::query_param_list(Fred::ContactCheckStatus::RUNNING)
            );

            if(locked_check_res.size() == 1) {
                return static_cast<std::string>(locked_check_res[0]["check_handle_"]);
            } else if(locked_check_res.size() == 0) {
                try {
                    update_some_enqueued_check_to_running();
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
    void update_some_enqueued_check_to_running(void) {
        Fred::OperationContext ctx;

        Database::Result locked_check_res = ctx.get_conn().exec_params(
            "SELECT c_ch.id AS id_ "
            "   FROM contact_check AS c_ch "
            "   WHERE c_ch.enum_contact_check_status_id = "
            "       ( SELECT id FROM enum_contact_check_status WHERE name = $1::varchar ) "
            "   LIMIT 1 "
            "   FOR UPDATE;",
            Database::query_param_list(Fred::ContactCheckStatus::ENQUEUED)
        );

        if(locked_check_res.size() != 1) {
            throw ExceptionNoEnqueuedChecksAvailable();
        }

        Database::Result updated_check_res = ctx.get_conn().exec_params(
            "UPDATE contact_check "
            "   SET enum_contact_check_status_id = "
            "       ( SELECT id FROM enum_contact_check_status WHERE name = $2::varchar ) "
            "   WHERE id = $1::bigint "
            "   RETURNING id ; ",
            Database::query_param_list
                ( static_cast<long long>(locked_check_res[0]["id_"]) )
                ( Fred::ContactCheckStatus::RUNNING )
        );

        if(updated_check_res.size() == 1) {
            ctx.commit_transaction();
        } else {
            throw Fred::InternalError("failed to update check to running status");
        }
    }

    std::string lazy_create_locked_enqueued_test(Fred::OperationContext& _ctx, const std::string& _check_handle) {
        while(true) {
            Database::Result locked_test_res = _ctx.get_conn().exec_params(
                "SELECT enum_test.name AS test_name_ "
                "   FROM contact_test_result AS c_t_r "
                "       JOIN enum_contact_test_status AS enum_status ON c_t_r.enum_contact_test_status_id = enum_status.id "
                "       JOIN contact_check AS c_ch ON c_t_r.contact_check_id = c_ch.id "
                "       JOIN enum_contact_test AS enum_test ON enum_test.id = c_t_r.enum_contact_test_id "
                "   WHERE c_ch.handle = $1::uuid "
                "       AND enum_status.name = $2::varchar "
                "   LIMIT 1 "
                "   FOR UPDATE; ",
                Database::query_param_list
                    (_check_handle)
                    (Fred::ContactTestStatus::ENQUEUED)
            );

            if(locked_test_res.size() == 1) {
                return static_cast<std::string>(locked_test_res[0]["test_name_"]);
            } else if(locked_test_res.size() == 0) {
                try {
                    try_create_next_test(_check_handle);
                } catch(ExceptionCheckTestsuiteFullyCreated&) {
                    throw;
                }
                /* in next attempt to lock test (in next iteration) the chances are better
                 * because there is one more test now */
            } else {
                throw Fred::InternalError("invalid count of returned records");
            }
        }
    }
    void try_create_next_test(const std::string& _check_handle) {
        while(true) {
            Fred::OperationContext ctx;
            /* idea is to get test_name which
             * a) is in testsuite of given check
             * b) is not "instantiated" as a record in contact_test_result for given check
             */
            Database::Result locked_test_name = ctx.get_conn().exec_params(
                "SELECT enum_test.name AS test_name_ "
                "   FROM enum_contact_test AS enum_test "
                "       JOIN contact_testsuite_map AS map_ ON map_.enum_contact_test_id = enum_test.id "
                "       JOIN contact_check AS c_ch ON c_ch.enum_contact_testsuite_id = map_.enum_contact_testsuite_id "
                "   WHERE c_ch.handle = $1::uuid "
                "       AND NOT EXISTS "
                "           (SELECT * "
                "               FROM contact_test_result "
                "                   JOIN contact_check ON contact_test_result.contact_check_id = contact_check.id "
                "               WHERE contact_check.handle = $1::uuid "
                "                   AND contact_test_result.enum_contact_test_id = enum_test.id "
                "           ) "
                "   LIMIT 1 "
                "   FOR SHARE OF enum_test; ",
                Database::query_param_list(_check_handle)
            );

            if(locked_test_name.size() == 0) {
                throw ExceptionCheckTestsuiteFullyCreated();
            } else if(locked_test_name.size() != 1) {
                throw Fred::InternalError("invalid count of returned records");
            }

            try {
                Fred::CreateContactTest(
                    _check_handle,
                    static_cast<std::string>( locked_test_name[0]["test_name_"] )
                ).exec(ctx);
            } catch (Fred::CreateContactTest::ExceptionCheckTestPairAlreadyExists& ) {
                /* Well, this might look controversial. Reason for swallowing the exception is that
                 * it just means someone created such test before SELECT test_name and CreateContactTest.exec
                 * in this procedure.
                 */
                continue;
            }

            ctx.commit_transaction();
        }
    }

    std::string evaluate_check_status_after_tests_finished(std::vector<std::string> _test_statuses) {

        bool has_some_ok = false;
        bool has_some_fail = false;
        bool has_some_other = false;

        BOOST_FOREACH(const std::string& status, _test_statuses) {
            if(status == Fred::ContactTestStatus::OK) {
                has_some_ok = true;
            } else if(status == Fred::ContactTestStatus::FAIL) {
                has_some_fail = true;
            } else {
                has_some_other = true;
            }
        }

        /* NOTE: checking if has_some_ok relates to the situation of empty vector
         * let's play it safe and don't say that check is ok*/
        if(has_some_ok && !has_some_fail && !has_some_other) {
            return Fred::ContactCheckStatus::OK;
        } else if(has_some_fail && !has_some_other) {
            return Fred::ContactCheckStatus::FAIL;
        } else {
            return Fred::ContactCheckStatus::TO_BE_DECIDED;
        }
    }

    void post_run_hooks(const std::string& _check_handle) {

    }
}
