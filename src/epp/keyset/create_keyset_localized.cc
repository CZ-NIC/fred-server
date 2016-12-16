#include "src/epp/keyset/create_keyset_localized.h"
#include "src/epp/keyset/create_keyset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/parameter_errors.h"
#include "src/epp/impl/response.h"

#include "util/log/context.h"

#include <boost/format.hpp>
#include <boost/format/free_funcs.hpp>

#include <string>

namespace Epp {
namespace Keyset {
namespace Localized {

namespace {

typedef bool Presents;

Presents insert_scalar_parameter_error_if_presents(
        const ParameterErrors& _src,
        Param::Enum _param,
        Reason::Enum _reason,
        std::set<Error>& _dst)
{
    if (_src.has_scalar_parameter_error(_param, _reason)) {
        _dst.insert(Error::of_scalar_parameter(_param, _reason));
        return true;
    }
    return false;
}

Presents insert_vector_parameter_error_if_presents(
        const ParameterErrors& _src,
        Param::Enum _param,
        Reason::Enum _reason,
        std::set<Error>& _dst)
{
    if (_src.has_vector_parameter_error(_param, _reason)) {
        const ParameterErrors::Where where = _src.get_vector_parameter_error(_param, _reason);
        for (ParameterErrors::Where::Indexes::const_iterator idx_ptr = where.indexes.begin();
             idx_ptr != where.indexes.end(); ++idx_ptr)
        {
            _dst.insert(Error::of_vector_parameter(_param, *idx_ptr, _reason));
        }
        return true;
    }
    return false;
}

}//namespace Epp::Keyset::Localized::{anonymous}

ResponseOfCreate create_keyset_localized(
        const std::string& _keyset_handle,
        const Optional<std::string>& _auth_info_pw,
        const std::vector<std::string>& _tech_contacts,
        const std::vector<Keyset::DsRecord>& _ds_records,
        const std::vector<Keyset::DnsKey>& _dns_keys,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(boost::str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(boost::str(boost::format("action-%1%") % static_cast<unsigned>(Action::CreateKeyset)));

        Fred::OperationContextCreator ctx;

        const CreateKeysetResult result =
            create_keyset(
                    ctx, _keyset_handle,
                    _auth_info_pw,
                    _tech_contacts,
                    _ds_records,
                    _dns_keys,
                    _registrar_id,
                    _logd_request_id);

        const ResponseOfCreate localized_result(
                create_localized_success_response(
                        ctx,
                        Response::ok,
                        _lang),
                result.crdate);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
                Notification::created,
                result.create_history_id,
                _registrar_id,
                _server_transaction_handle,
                _client_transaction_handle,
                _epp_notification_disabled,
                _dont_notify_client_transaction_handles_with_this_prefix);

        return localized_result;
    }
    catch (const AuthErrorServerClosingConnection&)
    {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang);
    }
    catch (const ParameterErrors& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        std::set<Error> errors;

        if (e.has_scalar_parameter_error(Param::keyset_tech, Reason::technical_contact_not_registered) ||
            e.has_scalar_parameter_error(Param::keyset_dnskey, Reason::no_dnskey))
        {
            throw create_localized_fail_response(
                    exception_localization_ctx,
                    Response::parameter_missing,
                    std::set<Error>(),
                    _lang);
        }

        if (e.has_scalar_parameter_error(Param::keyset_dsrecord, Reason::dsrecord_limit)  ||
            e.has_scalar_parameter_error(Param::keyset_tech,     Reason::techadmin_limit) ||
            e.has_scalar_parameter_error(Param::keyset_dnskey,   Reason::dnskey_limit))
        {
            throw create_localized_fail_response(
                    exception_localization_ctx,
                    Response::parameter_value_policy_error,
                    std::set<Error>(),
                    _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_handle, Reason::bad_format_keyset_handle, errors)) {
            throw create_localized_fail_response(
                    exception_localization_ctx,
                    Response::parameter_value_syntax_error,
                    errors,
                    _lang);
        }

        if (e.has_scalar_parameter_error(Param::keyset_handle, Reason::existing)) {
            throw create_localized_fail_response(
                    exception_localization_ctx,
                    Response::object_exist,
                    std::set<Error>(),
                    _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_handle, Reason::protected_period, errors)) {
            throw create_localized_fail_response(
                    exception_localization_ctx,
                    Response::parameter_value_policy_error,
                    errors,
                    _lang);
        }

        insert_vector_parameter_error_if_presents(e, Param::keyset_tech, Reason::technical_contact_not_registered, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_tech, Reason::duplicated_contact, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::duplicated_dnskey, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_flags, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_protocol, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_alg, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_key_char, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_key_len, errors);
        if (!errors.empty()) {
            throw create_localized_fail_response(
                    exception_localization_ctx,
                    Response::parameter_value_policy_error,
                    errors,
                    _lang);
        }

        throw create_localized_fail_response(exception_localization_ctx, Response::failed, e.get_set_of_error(), _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("create_keyset_localized failure: ") + e.what());
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in create_keyset_localized function");
        throw create_localized_fail_response(
                exception_localization_ctx,
                Response::failed,
                std::set<Error>(),
                _lang);
    }
}

} // namespace Epp::Keyset::Localized
} // namespace Epp::Keyset
} // namespace Epp
