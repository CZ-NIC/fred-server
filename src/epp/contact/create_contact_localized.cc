#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/create_contact_localized.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/contact/create_contact.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/exception_aggregate_param_errors.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"

#include "util/log/context.h"

namespace Epp {
namespace Contact {

namespace {

std::string convert(const boost::optional< Nullable< std::string > > &src)
{
    return ContactChange::does_value_mean< ContactChange::Value::to_set >(src) ? ContactChange::get_value(src)
                                                                               : std::string();
}

std::string convert(const boost::optional< std::string > &src)
{
    return ContactChange::does_value_mean< ContactChange::Value::to_set >(src) ? ContactChange::get_value(src)
                                                                               : std::string();
}

std::vector< std::string > convert(const std::vector< boost::optional< Nullable< std::string > > > &src)
{
    std::vector< std::string > result;
    result.reserve(src.size());
    typedef std::vector< boost::optional< Nullable< std::string > > > VectorOfChangeData;
    for (VectorOfChangeData::const_iterator data_ptr = src.begin(); data_ptr != src.end(); ++data_ptr) {
        result.push_back(convert(*data_ptr));
    }
    return result;
}

}//namespace Epp::{anonymous}

CreateContactInputData::CreateContactInputData(const ContactChange &src)
:   name(convert(src.name)),
    organization(convert(src.organization)),
    streets(convert(src.streets)),
    city(convert(src.city)),
    state_or_province(convert(src.state_or_province)),
    postal_code(convert(src.postal_code)),
    country_code(convert(src.country_code)),
    telephone(convert(src.telephone)),
    fax(convert(src.fax)),
    email(convert(src.email)),
    notify_email(convert(src.notify_email)),
    VAT(convert(src.vat)),
    ident(convert(src.ident)),
    identtype(src.ident_type),
    authinfo(
        ((src.auth_info_pw ? *src.auth_info_pw : Nullable<std::string>()).isnull() )
            ? boost::optional<std::string>()
            : boost::optional<std::string>(convert(src.auth_info_pw))),
    disclose(src.disclose)
{
    if (disclose.is_initialized()) {
        disclose->check_validity();
    }
}

CreateContactLocalizedResponse create_contact_localized(
    const std::string &_contact_handle,
    const CreateContactInputData &_data,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle,
    const std::string &_client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string &_dont_notify_client_transaction_handles_with_this_prefix)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>(Action::CreateContact) ) );

        Fred::OperationContextCreator ctx;

        const CreateContactResult impl_result(create_contact(ctx,
                                                             _contact_handle,
                                                             _data,
                                                             _registrar_id,
                                                             _logd_request_id));

        const CreateContactLocalizedResponse localized_result(
            create_localized_success_response(Response::ok, ctx, _lang),
            impl_result.crdate);

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::created,
            impl_result.create_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
            _epp_notification_disabled,
            _dont_notify_client_transaction_handles_with_this_prefix);

        return localized_result;

    } catch(const AuthErrorServerClosingConnection& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );

    } catch(const ObjectExists& e) {

        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::object_exist,
            std::set<Error>(),
            _lang
        );

    } catch(const InvalidHandle&) {

        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_value_syntax_error,
            Error::of_scalar_parameter(Param::contact_handle, Reason::bad_format_contact_handle),
            _lang
        );

    } catch(const AggregatedParamErrors& e) {

        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_value_policy_error,
            e.get(),
            _lang
        );

    } catch(const LocalizedFailResponse&) {
        throw;

    } catch(...) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::failed,
            std::set<Error>(),
            _lang
        );
    }
}

} // namespace Epp::Contact
} // namespace Epp
