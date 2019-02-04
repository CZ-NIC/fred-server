#include "src/backend/admin/contact/verification/enqueue_check.hh"

#include "libfred/db_settings.hh"
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
#include "util/log/context.hh"

#include <boost/algorithm/string/join.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

std::string request_check_enqueueing(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        const std::string& _testsuite_handle,
        Optional<unsigned long long> _logd_request_id)
{
    Logging::Context log("request_check_enqueueing");

    try
    {
        std::string created_handle = LibFred::CreateContactCheck(
                _contact_id,
                _testsuite_handle,
                _logd_request_id).exec(_ctx);

        return created_handle;
    }
    catch (const LibFred::ExceptionUnknownContactId&)
    {
        throw ExceptionUnknownContactId();
    }
    catch (const LibFred::ExceptionUnknownTestsuiteHandle&)
    {
        throw ExceptionUnknownTestsuiteHandle();
    }
}


void confirm_check_enqueueing(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        Optional<unsigned long long> _logd_request_id)
{
    Logging::Context log("confirm_check_enqueueing");

    LibFred::InfoContactCheckOutput info;

    try
    {
        info = LibFred::InfoContactCheck(uuid::from_string(_check_handle)).exec(_ctx);
    }
    catch (...)
    {
        throw ExceptionCheckNotUpdateable();
    }

    if (info.check_state_history.rbegin()->status_handle != LibFred::ContactCheckStatus::ENQUEUE_REQ)
    {
        throw ExceptionCheckNotUpdateable();
    }

    try
    {
        using namespace LibFred::ContactCheckStatus;

        LibFred::UpdateContactCheck(
                _check_handle,
                ENQUEUED,
                _logd_request_id).exec(_ctx);

        Database::Result obsolete_handles_res = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT c_ch.handle AS handle_ "
                "   FROM contact_check AS c_ch "
                "       JOIN enum_contact_check_status AS enum_status ON c_ch.enum_contact_check_status_id = enum_status.id "
                "       JOIN contact_history AS c_h1 ON c_ch.contact_history_id = c_h1.historyid "
                "       JOIN contact_history AS c_h2 ON c_h1.id = c_h2.id "
                "   WHERE enum_status.handle = ANY($1::varchar[]) "
                "       AND c_h2.historyid = $2::integer "
                // Don't want to invalidate our brand new check...
                "       AND c_ch.handle != $3::uuid ",
                // clang-format on
                Database::query_param_list(
                        std::string(
                                "{") + ENQUEUE_REQ + "," + ENQUEUED +
                        "}")(info.contact_history_id)(_check_handle));

        for (Database::Result::Iterator it = obsolete_handles_res.begin();
             it != obsolete_handles_res.end();
             ++it)
        {
            LibFred::UpdateContactCheck(
                    uuid::from_string(static_cast<std::string>((*it)["handle_"])),
                    INVALIDATED,
                    _logd_request_id).exec(_ctx);
        }

    }
    catch (const LibFred::ExceptionUnknownCheckHandle&)
    {
        throw ExceptionUnknownCheckHandle();

    }
}


std::string enqueue_check(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        const std::string& _testsuite_handle,
        Optional<unsigned long long> _logd_request_id)
{
    Logging::Context log("enqueue_check");

    std::string created_check_handle = request_check_enqueueing(
            _ctx,
            _contact_id,
            _testsuite_handle,
            _logd_request_id);

    if (_testsuite_handle == LibFred::TestsuiteHandle::AUTOMATIC ||
        _testsuite_handle == LibFred::TestsuiteHandle::THANK_YOU
        )
    {
        confirm_check_enqueueing(
                _ctx,
                uuid::from_string(created_check_handle),
                _logd_request_id);
    }

    return created_check_handle;
}


Optional<std::string> enqueue_check_if_no_other_exists(
        LibFred::OperationContext&         _ctx,
        unsigned long long _contact_id,
        const std::string&              _testsuite_handle,
        Optional<unsigned long long>    _logd_request_id)
{

    Database::Result existing_check_count_res = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT "
                    "COUNT( c_ch.id ) AS count_ "
                "FROM contact_check AS c_ch "
                    "JOIN contact_history AS c_h ON c_ch.contact_history_id = c_h.historyid "
                    "JOIN enum_contact_check_status AS enum_c_ch_s ON c_ch.enum_contact_check_status_id = enum_c_ch_s.id "
                "WHERE enum_c_ch_s.handle = ANY($1::varchar[]) "
                    "AND c_h.id = $2::bigint ",
            // clang-format on
            Database::query_param_list(
                    std::string(
                            "{")
                    + boost::join(
                            LibFred::ContactCheckStatus::get_not_yet_resolved(),
                            ",")
                    + "}")(_contact_id));

    if (static_cast<unsigned long long>(existing_check_count_res[0]["count_"]) == 0)
    {
        return enqueue_check(
                _ctx,
                _contact_id,
                _testsuite_handle,
                _logd_request_id);
    }

    return Optional<std::string>();
}


} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
