#include "src/epp/keyset/update_keyset_localized.h"
#include "src/epp/keyset/update_keyset.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/parameter_errors.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"

#include "util/log/context.h"

namespace Epp {
namespace Keyset {
namespace Localized {

namespace {

typedef bool Presents;

Presents insert_scalar_parameter_error_if_presents(const ParameterErrors &_src,
                                                   Param::Enum _param,
                                                   Reason::Enum _reason,
                                                   std::set< Error > &_dst)
{
    if (_src.has_scalar_parameter_error(_param, _reason)) {
        _dst.insert(Error::of_scalar_parameter(_param, _reason));
        return true;
    }
    return false;
}

Presents insert_vector_parameter_error_if_presents(const ParameterErrors &_src,
                                                   Param::Enum _param,
                                                   Reason::Enum _reason,
                                                   std::set< Error > &_dst)
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

LocalizedSuccessResponse update_keyset_localized(
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts_add,
    const std::vector< std::string > &_tech_contacts_rem,
    const std::vector< Keyset::DsRecord > &_ds_records_add,
    const std::vector< Keyset::DsRecord > &_ds_records_rem,
    const std::vector< Keyset::DnsKey > &_dns_keys_add,
    const std::vector< Keyset::DnsKey > &_dns_keys_rem,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    bool _epp_notification_disabled,
    const std::string &_dont_notify_client_transaction_handles_with_this_prefix)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::UpdateKeyset)));

        Fred::OperationContextCreator ctx;

        const UpdateKeysetResult result = update_keyset(ctx, _keyset_handle,
                                                             _auth_info_pw,
                                                             _tech_contacts_add,
                                                             _tech_contacts_rem,
                                                             _ds_records_add,
                                                             _ds_records_rem,
                                                             _dns_keys_add,
                                                             _dns_keys_rem,
                                                             _registrar_id,
                                                             _logd_request_id);

        const LocalizedSuccessResponse localized_result =
            create_localized_success_response(ctx, Response::ok, _lang);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::updated,
            result.update_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
            _epp_notification_disabled,
            _dont_notify_client_transaction_handles_with_this_prefix);

        return localized_result;
    }
    catch (const AuthErrorServerClosingConnection&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::authentication_error_server_closing_connection,
            std::set< Error >(),
            _lang);
    }
    catch (const ObjectStatusProhibitsOperation&) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::status_prohibits_operation,
            std::set< Error >(),
            _lang);
    }
    catch (const ParameterErrors &e) {
        Fred::OperationContextCreator ctx;
        std::set< Error > errors;

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_handle, Reason::keyset_notexist, errors)) {
            throw create_localized_fail_response(ctx, Response::object_not_exist, errors, _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::registrar_autor, Reason::unauthorized_registrar, errors)) {
            throw create_localized_fail_response(ctx, Response::authorization_error, errors, _lang);
        }

        if (e.has_scalar_parameter_error(Param::keyset_dsrecord, Reason::dsrecord_limit)  ||
            e.has_scalar_parameter_error(Param::keyset_tech,     Reason::techadmin_limit) ||
            e.has_scalar_parameter_error(Param::keyset_dnskey,   Reason::dnskey_limit))
        {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, std::set< Error >(), _lang);
        }

        insert_vector_parameter_error_if_presents(e, Param::keyset_tech_add,   Reason::technical_contact_not_registered, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_tech_add,   Reason::duplicated_contact, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_tech_add,   Reason::technical_contact_already_assigned, errors);
        if (!errors.empty()) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }

        insert_vector_parameter_error_if_presents(e, Param::keyset_tech_rem,   Reason::technical_contact_not_registered, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_tech_rem,   Reason::duplicated_contact, errors);
        if (!errors.empty()) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }

        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_add, Reason::duplicated_dnskey, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_add, Reason::dnskey_bad_flags, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_add, Reason::dnskey_bad_protocol, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_add, Reason::dnskey_bad_alg, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_add, Reason::dnskey_bad_key_char, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_add, Reason::dnskey_bad_key_len, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_add, Reason::dnskey_exist, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_rem, Reason::duplicated_dnskey, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey_rem, Reason::dnskey_notexist, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_dnskey,   Reason::no_dnskey_dsrecord, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }
        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_tech_rem, Reason::can_not_remove_tech, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_value_policy_error, errors, _lang);
        }

        throw create_localized_fail_response(ctx, Response::failed, e.get_set_of_error(), _lang);
    }
    catch (const LocalizedFailResponse&) {
        throw;
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(
            ctx,
            Response::failed,
            std::set< Error >(),
            _lang);
    }
}

} // namespace Epp::Keyset::Localized
} // namespace Epp::Keyset
} // namespace Epp
