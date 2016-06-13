#include "src/epp/keyset/localized_update.h"
#include "src/epp/keyset/update.h"

#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/exception.h"
#include "src/epp/parameter_errors.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"

#include "util/log/context.h"

namespace Epp {
namespace KeySet {
namespace Localized {

namespace {

typedef bool Presents;

Presents insert_scalar_parameter_error_if_presents(const ParameterErrors &_src,
                                                   Param::Enum _param,
                                                   Reason::Enum _reason,
                                                   std::set< Error > &_dst)
{
    if (_src.has_scalar_parameter_error(_param, _reason)) {
        _dst.insert(scalar_parameter_failure(_param, _reason));
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
            _dst.insert(vector_parameter_failure(_param, *idx_ptr, _reason));
        }
        return true;
    }
    return false;
}

}//namespace Epp::KeySet::Localized::{anonymous}

ResponseOfUpdate update(
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts,
    const std::vector< KeySet::DsRecord > &_ds_records,
    const std::vector< KeySet::DnsKey > &_dns_keys,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const std::string &_dont_notify_client_transaction_handles_with_this_prefix)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::KeySetCreate)));

        Fred::OperationContextCreator ctx;

        const KeysetUpdateResult result = keyset_update(ctx, _keyset_handle,
                                                             _auth_info_pw,
                                                             _tech_contacts,
                                                             _ds_records,
                                                             _dns_keys,
                                                             _registrar_id,
                                                             _logd_request_id);

        const ResponseOfUpdate localized_result(
            create_localized_success_response(Response::ok, ctx, _lang),
            result.update);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::created,
            result.update_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
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
    catch (const ParameterErrors &e) {
        Fred::OperationContextCreator ctx;
        std::set< Error > errors;

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_tech, Reason::tech_notexist, errors)) {
            insert_scalar_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::no_dnskey, errors);
            throw create_localized_fail_response(ctx, Response::parameter_missing, errors, _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_tech, Reason::techadmin_limit, errors)) {
            insert_scalar_parameter_error_if_presents(e, Param::keyset_dsrecord, Reason::dsrecord_limit, errors);
            insert_scalar_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_limit, errors);
            throw create_localized_fail_response(ctx, Response::parameter_range_error, errors, _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::no_dnskey, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_missing, errors, _lang);
        }

        insert_scalar_parameter_error_if_presents(e, Param::keyset_dsrecord, Reason::dsrecord_limit, errors);
        insert_scalar_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_limit, errors);
        if (!errors.empty()) {
            throw create_localized_fail_response(ctx, Response::parameter_range_error, errors, _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_handle, Reason::bad_format_keyset_handle, errors)) {
            insert_scalar_parameter_error_if_presents(e, Param::keyset_handle, Reason::protected_period, errors);
            throw create_localized_fail_response(ctx, Response::parameter_error, errors, _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_handle, Reason::existing, errors)) {
            errors.clear();
            throw create_localized_fail_response(ctx, Response::object_exist, errors, _lang);
        }

        if (insert_scalar_parameter_error_if_presents(e, Param::keyset_handle, Reason::protected_period, errors)) {
            throw create_localized_fail_response(ctx, Response::parameter_error, errors, _lang);
        }

        insert_vector_parameter_error_if_presents(e, Param::keyset_tech, Reason::tech_notexist, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_tech, Reason::duplicity_contact, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::duplicity_dnskey, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_flags, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_protocol, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_alg, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_key_char, errors);
        insert_vector_parameter_error_if_presents(e, Param::keyset_dnskey, Reason::dnskey_bad_key_len, errors);
        if (!errors.empty()) {
            throw create_localized_fail_response(ctx, Response::parameter_error, errors, _lang);
        }

        throw;
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

}//namespace Epp::KeySet::Localized
}//namespace Epp::KeySet
}//namespace Epp
