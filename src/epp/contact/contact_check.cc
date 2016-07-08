#include "src/epp/contact/contact_check.h"

#include "src/epp/action.h"
#include "src/epp/contact/contact_check_impl.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"
#include "util/log/context.h"

#include <set>
#include <stdexcept>

namespace Epp {

namespace {

typedef std::map< ContactHandleRegistrationObstruction::Enum, std::string > ObstructionToDescription;
typedef std::map< std::string, Nullable< ContactHandleRegistrationObstruction::Enum > > HandleToObstruction;
typedef std::map< std::string, boost::optional< LocalizedContactHandleRegistrationObstruction > > HandleToLocalizedObstruction;
typedef std::map< unsigned, ContactHandleRegistrationObstruction::Enum > DescriptionIdToObstruction;

std::string get_column_for_language(SessionLang::Enum _lang)
{
    switch (_lang)
    {
        case SessionLang::en: return "reason";
        case SessionLang::cs: return "reason_cs";
    }
    throw UnknownLocalizationLanguage();
}

ObstructionToDescription get_localized_description_of_obstructions(
    Fred::OperationContext &_ctx,
    const DescriptionIdToObstruction &_obstructions,
    SessionLang::Enum _lang)
{
    ObstructionToDescription result;
    if (_obstructions.empty()) {
        return result;
    }

    Database::query_param_list params;
    std::string query = "SELECT id," + get_column_for_language(_lang) + " "
                        "FROM enum_reason "
                        "WHERE id IN (";
    for (DescriptionIdToObstruction::const_iterator obstruction_ptr = _obstructions.begin();
         obstruction_ptr != _obstructions.end();
         ++obstruction_ptr)
    {
        if (obstruction_ptr != _obstructions.begin()) {
            query += ",";
        }
        query += ("$" + params.add(obstruction_ptr->first) + "::INTEGER");
    }
    query += ")";
    const Database::Result db_res = _ctx.get_conn().exec_params(query, params);
    if (db_res.size() < _obstructions.size()) {
        throw MissingLocalizedDescription();
    }

    for (std::size_t idx = 0; idx < db_res.size(); ++idx) {
        const unsigned description_id = static_cast< unsigned >(db_res[idx][0]);
        const DescriptionIdToObstruction::const_iterator obstruction_ptr =
            _obstructions.find(description_id);
        if (obstruction_ptr == _obstructions.end()) {
            throw UnknownLocalizedDescriptionId();
        }
        const ContactHandleRegistrationObstruction::Enum obstruction = obstruction_ptr->second;
        const std::string description = static_cast< std::string >(db_res[idx][1]);
        result.insert(std::make_pair(obstruction, description));
    }

    return result;
}

boost::optional< LocalizedContactHandleRegistrationObstruction > get_localized_obstruction(
    const Nullable< ContactHandleRegistrationObstruction::Enum > &_check_result,
    const ObstructionToDescription &_localized_descriptions)
{
    if (_check_result.isnull()) {
        return boost::optional< LocalizedContactHandleRegistrationObstruction >();
    }
    const ContactHandleRegistrationObstruction::Enum obstruction = _check_result.get_value();
    const ObstructionToDescription::const_iterator localized_description_ptr =
        _localized_descriptions.find(obstruction);
    if (localized_description_ptr == _localized_descriptions.end()) {
        throw MissingLocalizedDescription();
    }
    const std::string description = localized_description_ptr->second;
    return LocalizedContactHandleRegistrationObstruction(obstruction, description);
}

HandleToLocalizedObstruction create_localized_check_response(
    Fred::OperationContext &_ctx,
    const HandleToObstruction &_check_results,
    SessionLang::Enum _lang)
{
    DescriptionIdToObstruction used_obstructions;
    for (HandleToObstruction::const_iterator check_ptr = _check_results.begin();
         check_ptr != _check_results.end();
         ++check_ptr)
    {
        if (!check_ptr->second.isnull()) {
            const ContactHandleRegistrationObstruction::Enum obstruction = check_ptr->second.get_value();
            const Reason::Enum reason = ContactHandleRegistrationObstruction::to_reason(obstruction);
            const unsigned description_id = to_description_db_id(reason);
            used_obstructions[description_id] = obstruction;
        }
    }
    const ObstructionToDescription localized_descriptions =
        get_localized_description_of_obstructions(_ctx, used_obstructions, _lang);

    HandleToLocalizedObstruction result;

    for (HandleToObstruction::const_iterator check_ptr = _check_results.begin();
         check_ptr != _check_results.end();
         ++check_ptr)
    {
        const std::string handle = check_ptr->first;
        result.insert(std::make_pair(handle, get_localized_obstruction(check_ptr->second, localized_descriptions)));
    }

    return result;
}

}//namespace Epp::{anonymous}

LocalizedCheckContactResponse contact_check(
    const std::set<std::string>& _contact_handles,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
) {
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::ContactCheck) ) );

        if (_registrar_id == 0) {
            Fred::OperationContextCreator exception_localization_ctx;
            throw create_localized_fail_response(
                exception_localization_ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang
            );
        }

        /* different than other methods - implementation is not throwing any specific exceptions so there's no need for exception translation */
        Fred::OperationContextCreator ctx;

        const HandleToObstruction impl_result = contact_check_impl(ctx, _contact_handles);
        const HandleToLocalizedObstruction localized_check_response =
            create_localized_check_response(ctx, impl_result, _lang);
        const LocalizedSuccessResponse ok_response =
            create_localized_success_response(Response::ok, ctx, _lang);

        return LocalizedCheckContactResponse(ok_response, localized_check_response);

    }
    catch (const LocalizedFailResponse&) {
        throw;
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::failed,
            std::set<Error>(),
            _lang);
    }
}

}
