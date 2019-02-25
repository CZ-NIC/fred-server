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
#include "src/backend/admin/contact/verification/contact_states/delete_all.hh"
#include "src/backend/admin/contact/verification/contact_states/enum.hh"
#include "src/backend/admin/contact/verification/related_records.hh"
#include "src/backend/admin/contact/verification/run_all_enqueued_checks.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opexception.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"
#include "libfred/registrable_object/contact/verification/update_test.hh"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <memory>
#include <set>

#include "util/log/context.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

static Optional<std::string> lock_some_enqueued_check(LibFred::OperationContext& ctx)
{
    Database::Result locked_check_res = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT c_ch.handle AS handle_ "
                "FROM contact_check AS c_ch "
                    "WHERE c_ch.enum_contact_check_status_id = "
                    "( SELECT id FROM enum_contact_check_status WHERE handle = $1::varchar ) "
                "LIMIT 1 "
                "FOR UPDATE OF c_ch ",
            // clang-format on
            Database::query_param_list(LibFred::ContactCheckStatus::ENQUEUED));

    if (locked_check_res.size() == 1)
    {
        return static_cast<std::string>(locked_check_res[0]["handle_"]);
    }

    return Optional<std::string>();
}


static Optional<std::string> lock_some_running_check_with_only_enqueued_tests(LibFred::OperationContext& ctx)
{
    Database::Result locked_check_res = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT c_ch.handle AS handle_ "
                "FROM contact_check AS c_ch "
                    "WHERE c_ch.enum_contact_check_status_id = "
                        "( SELECT id FROM enum_contact_check_status WHERE handle = $1::varchar ) "
                        "AND NOT EXISTS ( "
                            "SELECT * "
                                "FROM contact_test_result AS c_t_r "
                                    "JOIN enum_contact_test_status AS enum_c_t_s "
                                        "ON enum_c_t_s.id = c_t_r.enum_contact_test_status_id "
                                "WHERE c_t_r.contact_check_id = c_ch.id "
                                    "AND enum_c_t_s.handle != $2::varchar "
                        ") "
                "LIMIT 1 "
                "FOR UPDATE OF c_ch ",
            // clang-format on
            Database::query_param_list(LibFred::ContactCheckStatus::RUNNING)(
                    LibFred::ContactTestStatus::
                    ENQUEUED));

    if (locked_check_res.size() == 1)
    {
        return static_cast<std::string>(locked_check_res[0]["handle_"]);
    }

    return Optional<std::string>();
}


static Optional<std::string> lock_some_running_check_with_some_finished_test(LibFred::OperationContext& ctx)
{
    Database::Result locked_check_res = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT c_ch.handle AS handle_ "
                "FROM contact_check AS c_ch "
                    "WHERE c_ch.enum_contact_check_status_id = "
                        "( SELECT id FROM enum_contact_check_status WHERE handle = $1::varchar ) "
                        "AND EXISTS ("
                            "SELECT * "
                                "FROM contact_test_result AS c_t_r "
                                    "JOIN enum_contact_test_status AS enum_c_t_s "
                                        "ON enum_c_t_s.id = c_t_r.enum_contact_test_status_id "
                                "WHERE c_t_r.contact_check_id = c_ch.id "
                                    "AND enum_c_t_s.handle != $2::varchar "
                                    "AND enum_c_t_s.handle != $3::varchar "
                        ") "
                "LIMIT 1 "
                "FOR UPDATE OF c_ch ",
            // clang-format on
            Database::query_param_list(LibFred::ContactCheckStatus::RUNNING)(
                    LibFred::ContactTestStatus::
                    ENQUEUED)(LibFred::ContactTestStatus::RUNNING));

    if (locked_check_res.size() == 1)
    {
        return static_cast<std::string>(locked_check_res[0]["handle_"]);
    }

    return Optional<std::string>();
}


static Optional<std::string> lock_some_test(
        LibFred::OperationContext& ctx,
        const uuid& check_handle,
        const std::string& test_status)
{

    Database::Result locked_test_res = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT enum_test.handle AS test_handle_ "
                "FROM contact_test_result AS c_t_r "
                    "JOIN enum_contact_test_status AS enum_status ON c_t_r.enum_contact_test_status_id = enum_status.id "
                    "JOIN contact_check AS c_ch ON c_t_r.contact_check_id = c_ch.id "
                    "JOIN enum_contact_test AS enum_test ON enum_test.id = c_t_r.enum_contact_test_id "
                "WHERE c_ch.handle = $1::uuid "
                    "AND enum_status.handle = $2::varchar "
                "LIMIT 1 "
                "FOR UPDATE OF enum_test ",
            // clang-format on
            Database::query_param_list(
                    check_handle)(test_status));

    if (locked_test_res.size() == 1)
    {
        return static_cast<std::string>(locked_test_res[0]["test_handle_"]);
    }

    return Optional<std::string>();
}


static Optional<std::string> lock_some_running_test(
        LibFred::OperationContext& ctx,
        const uuid& check_handle)
{
    return lock_some_test(
            ctx,
            check_handle,
            LibFred::ContactTestStatus::RUNNING);
}


static Optional<std::string> lock_some_enqueued_test(
        LibFred::OperationContext& ctx,
        const uuid& check_handle)
{
    return lock_some_test(
            ctx,
            check_handle,
            LibFred::ContactTestStatus::ENQUEUED);
}


static void create_all_tests(
        LibFred::OperationContext& ctx,
        const uuid& check_handle,
        Optional<unsigned long long> logd_request_id)
{
    Database::Result testhandles_res = ctx.get_conn().exec_params(
            // clang-format off
            "SELECT enum_test.handle            AS handle_ "
                "FROM enum_contact_test          AS enum_test "
                    "JOIN contact_testsuite_map  AS c_map "
                        "ON enum_test.id = c_map.enum_contact_test_id "
                    "JOIN contact_check          AS check_ "
                        "ON check_.enum_contact_testsuite_id = c_map.enum_contact_testsuite_id "
                "WHERE check_.handle=$1::uuid "
                "ORDER by enum_test.id ASC ",
            // clang-format on
            Database::query_param_list(check_handle));
    if (testhandles_res.size() == 0)
    {
        throw LibFred::InternalError(
                std::string("testsuite of check(id=")
                + static_cast<std::string>(check_handle)
                + ") contains no tests");
    }

    for (Database::Result::Iterator it = testhandles_res.begin(); it != testhandles_res.end(); ++it)
    {
        LibFred::CreateContactTest create_test(
                uuid::from_string(check_handle),
                static_cast<std::string>((*it)["handle_"]));

        if (logd_request_id.isset())
        {
            create_test.set_logd_request_id(logd_request_id.get_value());
        }
        create_test.exec(ctx);
    }
}


static std::string evaluate_check_status_after_tests_finished(
        LibFred::OperationContext& ctx,
        const uuid& handle)
{

    LibFred::InfoContactCheckOutput info_check = LibFred::InfoContactCheck(handle).exec(ctx);

    bool has_some_ok = false;
    bool has_some_fail = false;
    bool has_some_error = false;
    bool has_some_running = false;
    bool has_some_other = false;

    for (std::vector<LibFred::InfoContactCheckOutput::ContactTestResultData>::const_iterator it =
             info_check.tests.begin();
         it != info_check.tests.end();
         ++it)
    {
        const std::string& status = it->state_history.rbegin()->status_handle;

        if (status == LibFred::ContactTestStatus::OK)
        {
            has_some_ok = true;
        }
        else if (status == LibFred::ContactTestStatus::FAIL)
        {
            has_some_fail = true;
        }
        else if (status == LibFred::ContactTestStatus::ERROR)
        {
            has_some_error = true;
        }
        else if (status == LibFred::ContactTestStatus::RUNNING)
        {
            has_some_running = true;
        }
        else if (status == LibFred::ContactTestStatus::SKIPPED)
        {
            // do nothing but don't count it as other
        }
        else
        {
            has_some_other = true;
        }
    }

    /* NOTE: checking if has_some_ok == true handles the improbably but possible
     * case of empty test statuses vector
     * NOTE: skipped test is ok - if it shouldn't be ok, than FAIL or ERROR are more appropriate
     */

    if (has_some_ok && !has_some_fail && !has_some_error && !has_some_running && !has_some_other)
    {
        return LibFred::ContactCheckStatus::AUTO_OK;
    }
    else if (has_some_fail && !has_some_error && !has_some_running && !has_some_other)
    {
        return LibFred::ContactCheckStatus::AUTO_FAIL;
    }
    else if (has_some_error)
    {
        return LibFred::ContactCheckStatus::AUTO_TO_BE_DECIDED;
    }
    else if (!has_some_error && has_some_running)
    {
        return LibFred::ContactCheckStatus::AUTO_TO_BE_DECIDED;
    }
    else
    {
        return LibFred::ContactCheckStatus::AUTO_TO_BE_DECIDED;
    }
}


static void preprocess_automatic_check(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{
    // in case of need feel free to express yourself...
}


static void preprocess_manual_check(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{
    LibFred::InfoContactCheckOutput check_info = LibFred::InfoContactCheck(_check_handle).exec(_ctx);

    LibFred::InfoContactOutput contact_info = LibFred::InfoContactHistoryByHistoryid(
            check_info.contact_history_id).exec(_ctx);

    ContactStates::cancel_all_states(
            _ctx,
            contact_info.info_contact_data.id);

    std::set<std::string> status;
    status.insert(ContactStates::CONTACT_IN_MANUAL_VERIFICATION);

    std::set<unsigned long long> state_request_ids;
    state_request_ids.insert(
            LibFred::CreateObjectStateRequestId(
                    contact_info.info_contact_data.id,
                    status).exec(_ctx)
            .second);

    LibFred::PerformObjectStateRequest(contact_info.info_contact_data.id).exec(_ctx);

    add_related_object_state_requests(
            _ctx,
            _check_handle,
            state_request_ids);
}


static void preprocess_thank_you_check(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{
    // right now it's exactly the same
    preprocess_manual_check(
            _ctx,
            _check_handle);
}


static void preprocess_check(
        LibFred::OperationContext& ctx,
        const uuid& handle)
{
    LibFred::InfoContactCheckOutput check_info(
            LibFred::InfoContactCheck(handle).exec(ctx));

    if (check_info.testsuite_handle == LibFred::TestsuiteHandle::AUTOMATIC)
    {
        preprocess_automatic_check(
                ctx,
                handle);

    }
    else if (check_info.testsuite_handle == LibFred::TestsuiteHandle::MANUAL)
    {
        preprocess_manual_check(
                ctx,
                handle);

    }
    else if (check_info.testsuite_handle == LibFred::TestsuiteHandle::THANK_YOU)
    {
        preprocess_thank_you_check(
                ctx,
                handle);
    }
}


static void run_test(
        LibFred::OperationContext& ctx,
        const std::map<std::string, std::shared_ptr<Test> >&
                _tests,
        const uuid& _check_handle,
        const std::string& _test_handle,
        std::set<unsigned long long>& _related_mail_ids,
        std::set<unsigned long long>& _related_message_ids,
        Optional<unsigned long long> _logd_request_id)
{
    Test::TestRunResult temp_result = _tests.at(_test_handle)->run(
            LibFred::InfoContactCheck(_check_handle).exec(ctx).contact_history_id);

    LibFred::UpdateContactTest(
            _check_handle,
            _test_handle,
            temp_result.status,
            _logd_request_id,
            temp_result.error_message).exec(ctx);

    _related_mail_ids.insert(
            temp_result.related_mail_archive_ids.begin(),
            temp_result.related_mail_archive_ids.end());
    _related_message_ids.insert(
            temp_result.related_message_archive_ids.begin(),
            temp_result.related_message_archive_ids.end());

    if (temp_result.status == LibFred::ContactTestStatus::ENQUEUED
        || temp_result.status == LibFred::ContactTestStatus::RUNNING
        )
    {
        throw LibFred::InternalError(
                "malfunction in implementation of test " + _test_handle +
                ", run() returned bad status");
    }
}


std::vector<std::string> run_all_enqueued_checks(
        const std::map<std::string, std::shared_ptr<Test> >& _tests,
        Optional<unsigned long long> _logd_request_id)
{
    Logging::Context log("run_all_enqueued_checks");

    using std::string;
    std::vector<std::string> handles;

    while (true)
    {
        while (true)
        {
            while (true)
            {
                LibFred::OperationContextCreator ctx_check_postprocess;
                Optional<string> check_handle_to_postprocess =
                    lock_some_running_check_with_some_finished_test(ctx_check_postprocess);
                if (!check_handle_to_postprocess.isset())
                {
                    break;
                }
                uuid check_uuid_to_postprocess(uuid::from_string(check_handle_to_postprocess.get_value()));
                LibFred::UpdateContactCheck(
                        check_uuid_to_postprocess,
                        evaluate_check_status_after_tests_finished(
                                ctx_check_postprocess,
                                check_uuid_to_postprocess),
                        _logd_request_id).exec(ctx_check_postprocess);
                ctx_check_postprocess.commit_transaction();
                handles.push_back(check_handle_to_postprocess.get_value());
            }

            LibFred::OperationContextCreator ctx_check_process;
            Optional<string> check_handle_to_process = lock_some_running_check_with_only_enqueued_tests(
                    ctx_check_process);
            if (!check_handle_to_process.isset())
            {
                break;
            }
            uuid check_uuid_to_process(uuid::from_string(check_handle_to_process.get_value()));
            preprocess_check(
                    ctx_check_process,
                    check_uuid_to_process);
            std::set<unsigned long long> related_mail_ids;
            std::set<unsigned long long> related_message_ids;
            while (true)
            {
                while (true)
                {
                    bool error_occurred = false;
                    Optional<string> test_handle_to_process;
                    {
                        LibFred::OperationContextCreator ctx_test_process;
                        test_handle_to_process = lock_some_running_test(
                                ctx_test_process,
                                check_uuid_to_process);
                        if (!test_handle_to_process.isset())
                        {
                            break;
                        }
                        try
                        {
                            run_test(
                                    ctx_test_process,
                                    _tests,
                                    check_uuid_to_process,
                                    test_handle_to_process.get_value(),
                                    related_mail_ids,
                                    related_message_ids,
                                    _logd_request_id);
                            ctx_test_process.commit_transaction();
                        }
                        catch (...)
                        {
                            error_occurred = true;
                        }
                    }
                    if (error_occurred)
                    {
                        try
                        {
                            LibFred::OperationContextCreator ctx_testrun_error;
                            LibFred::UpdateContactTest(
                                    check_uuid_to_process,
                                    test_handle_to_process.get_value(),
                                    LibFred::ContactTestStatus::ERROR,
                                    _logd_request_id,
                                    "exception in test implementation and/or execution").exec(
                                    ctx_testrun_error);

                            ctx_testrun_error.get_log().warning(
                                    (string("exception in test implementation ") +
                                     test_handle_to_process.get_value()).c_str());
                            ctx_testrun_error.commit_transaction();
                        }
                        catch (...)
                        {
                            throw LibFred::InternalError(
                                    (string("problem when setting test status to ERROR; check_handle=") +
                                     check_handle_to_process.get_value() + "; test_handle=" +
                                     test_handle_to_process.get_value()).c_str());
                        }
                    }
                }

                LibFred::OperationContextCreator ctx_test_preprocess;
                Optional<string> test_handle_to_preprocess = lock_some_enqueued_test(
                        ctx_test_preprocess,
                        check_uuid_to_process);
                if (!test_handle_to_preprocess.isset())
                {
                    break;
                }
                LibFred::UpdateContactTest(
                        check_uuid_to_process,
                        test_handle_to_preprocess.get_value(),
                        LibFred::ContactTestStatus::RUNNING,
                        _logd_request_id,
                        Optional<std::string>()).exec(ctx_test_preprocess);
                ctx_test_preprocess.commit_transaction();
            }

            add_related_messages(
                    ctx_check_process,
                    check_uuid_to_process,
                    related_message_ids);
            add_related_mail(
                    ctx_check_process,
                    check_uuid_to_process,
                    related_mail_ids);

            ctx_check_process.commit_transaction();
        }

        LibFred::OperationContextCreator ctx_check_preprocess;
        Optional<string> check_handle_to_preprocess = lock_some_enqueued_check(ctx_check_preprocess);
        if (!check_handle_to_preprocess.isset())
        {
            break;
        }
        uuid check_uuid_to_preprocess(uuid::from_string(check_handle_to_preprocess.get_value()));
        LibFred::UpdateContactCheck(
                check_uuid_to_preprocess,
                LibFred::ContactCheckStatus::RUNNING,
                _logd_request_id).exec(ctx_check_preprocess);
        create_all_tests(
                ctx_check_preprocess,
                check_uuid_to_preprocess,
                _logd_request_id);
        ctx_check_preprocess.commit_transaction();
    }

    return handles;
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
