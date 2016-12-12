#include "src/epp/impl/localization.h"

#include "src/epp/impl/response_localized.h"
#include "src/epp/error.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "util/map_at.h"

#include <set>
#include <string>
#include <stdexcept>

#include <boost/foreach.hpp>

namespace Epp {

namespace {

std::string get_reason(
    Fred::OperationContext &_ctx,
    Reason::Enum _reason,
    SessionLang::Enum _lang)
{
    const std::string column_name = (_lang == SessionLang::cs) ? "reason_cs"
                                                               : "reason";
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT " + column_name + " "
        "FROM enum_reason "
        "WHERE id=$1::integer",
        Database::query_param_list(to_description_db_id(_reason)));

    if (res.size() < 1) {
        throw MissingLocalizedDescription();
    }

    if (1 < res.size()) {
        throw std::runtime_error("0 or 1 row expected");
    }

    switch (_lang)
    {
        case SessionLang::en:
        case SessionLang::cs:
            return static_cast< std::string >(res[0][0]);
    }

    throw UnknownLocalizationLanguage();
}

std::string get_response_msg(
    Fred::OperationContext &_ctx,
    Response::Enum _response,
    SessionLang::Enum _lang)
{
    const std::string column_name = (_lang == SessionLang::cs) ? "status_cs"
                                                               : "status";
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT " + column_name + " "
        "FROM enum_error "
        "WHERE id=$1::integer",
        Database::query_param_list(to_description_db_id(_response)));

    if (res.size() < 1) {
        throw MissingLocalizedDescription();
    }

    if (1 < res.size()) {
        throw std::runtime_error("0 or 1 row expected");
    }

    switch (_lang)
    {
        case SessionLang::en:
        case SessionLang::cs:
            return static_cast< std::string >(res[0][0]);
    }

    throw UnknownLocalizationLanguage();
}

std::set< LocalizedError > create_localized_errors(
    Fred::OperationContext &_ctx,
    const std::set< Error > &_errors,
    SessionLang::Enum _lang)
{
    std::set< LocalizedError > result;

    for (std::set< Error >::const_iterator error_ptr = _errors.begin(); error_ptr != _errors.end(); ++error_ptr)
    {
        const std::string reason_description = get_reason(_ctx, error_ptr->reason, _lang);
        result.insert(LocalizedError(error_ptr->param, error_ptr->position, reason_description));
    }

    return result;
}

}//namespace Epp::{anonymous}

LocalizedFailResponse create_localized_fail_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    const std::set<Error>& _errors,
    const SessionLang::Enum _lang
) {
    return LocalizedFailResponse(
        _response,
        get_response_msg(_ctx, _response, _lang),
        create_localized_errors(_ctx, _errors, _lang)
    );
}

LocalizedFailResponse create_localized_fail_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    const Error& _error,
    const SessionLang::Enum _lang
) {
    std::set<Error> errors;
    errors.insert(_error);

    return create_localized_fail_response(_ctx, _response, errors, _lang);
}

LocalizedSuccessResponse create_localized_success_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    const SessionLang::Enum _lang
) {
    return LocalizedSuccessResponse(
        _response,
        get_response_msg(_ctx, Response::ok, _lang)
    );
}

namespace {

std::string get_ok_state_description(SessionLang::Enum _lang)
{
    switch (_lang)
    {
        case SessionLang::en:
            return "Object is without restrictions";
        case SessionLang::cs:
            return "Objekt je bez omezení";
    }
    return std::string();
}

}
std::map<std::string, std::string> get_object_state_descriptions(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _state_handles,
    const SessionLang::Enum _lang
) {
    std::map<std::string, std::string> handle_to_description;
    {
        const std::vector< Fred::ObjectStateDescription > all_state_descriptions =
            Fred::GetObjectStateDescriptions(Conversion::Enums::to_db_handle(_lang)).exec(_ctx);
        BOOST_FOREACH(const Fred::ObjectStateDescription& state_description, all_state_descriptions) {
            handle_to_description.insert(std::make_pair(state_description.handle, state_description.description));
        }
    }

    std::map<std::string, std::string> result;
    BOOST_FOREACH(const std::string& handle, _state_handles) {
        /* XXX HACK: OK state */
        if (handle == "ok") {
            result["ok"] = get_ok_state_description(_lang);
            continue;
        }

        try {
            result[handle] = map_at(handle_to_description, handle);

        } catch(const std::out_of_range&) {
            /* intentionally ignoring missing descriptions */
        }
    }

    return result;
}

LocalizedStates get_localized_object_state(
    Fred::OperationContext &_ctx,
    const std::set< Fred::Object_State::Enum > &_states,
    SessionLang::Enum _lang)
{
    typedef std::vector< Fred::ObjectStateDescription > ObjectStateDescriptions;
    const ObjectStateDescriptions all_state_descriptions =
        Fred::GetObjectStateDescriptions(Conversion::Enums::to_db_handle(_lang)).exec(_ctx);

    LocalizedStates states;
    for (ObjectStateDescriptions::const_iterator state_ptr = all_state_descriptions.begin();
         state_ptr != all_state_descriptions.end(); ++state_ptr)
    {
        const Fred::Object_State::Enum state =
            Conversion::Enums::from_db_handle< Fred::Object_State >(state_ptr->handle);
        if (_states.find(state) != _states.end()) {
            states.descriptions[state] = state_ptr->description;
        }
    }
    states.ok_state_description = get_ok_state_description(_lang);
    return states;
}

}
