#include "src/epp/localization.h"

#include "src/epp/localized_response.h"
#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/reason.h"
#include "src/epp/response.h"
#include "src/epp/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "util/map_at.h"

#include <set>
#include <string>

#include <boost/foreach.hpp>

namespace Epp {

static std::string get_reason(
    Fred::OperationContext& _ctx,
    const Reason::Enum _reason,
    const SessionLang::Enum _lang
) {
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT "
            "reason AS reason_en_, "
            "reason_cs AS reason_cz_ "
        "FROM enum_reason "
        "WHERE id = $1::integer ",
        Database::query_param_list( to_description_db_id(_reason) )
    );

    if(res.size() != 1) {
        throw MissingLocalizedDescription();
    }

    if(_lang == SessionLang::en) {
        return static_cast<std::string>(res[0]["reason_en_"]);
    }
    if(_lang == SessionLang::cz) {
        return static_cast<std::string>(res[0]["reason_cz_"]);
    }

    throw UnknownLocalizationLanguage();
}

static std::string get_response_msg(
    Fred::OperationContext& _ctx,
    const Response::Enum _response,
    const SessionLang::Enum _lang
) {
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT "
            "status AS status_en_, "
            "status_cs AS status_cz_ "
        "FROM enum_error "
        "WHERE id = $1::integer ",
        Database::query_param_list( to_description_db_id(_response) )
    );

    if(res.size() != 1) {
        throw MissingLocalizedDescription();
    }

    if(_lang == SessionLang::en) {
        return static_cast<std::string>(res[0]["status_en_"]);
    }
    if(_lang == SessionLang::cz) {
        return static_cast<std::string>(res[0]["status_cz_"]);
    }

    throw UnknownLocalizationLanguage();
}

static std::set<LocalizedError> create_localized_errors(
    Fred::OperationContext& _ctx,
    const std::set<Error>& _errors,
    const SessionLang::Enum _lang
) {
    std::set<LocalizedError> result;

    BOOST_FOREACH(const Error& e, _errors) {
        result.insert(
            LocalizedError(
                e.param,
                e.position,
                get_reason(
                    _ctx,
                    e.reason,
                    _lang
                )
            )
        );
    }

    return result;
}

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
    const Response::Enum& _response,
    Fred::OperationContext& _ctx,
    const SessionLang::Enum _lang
) {
    return LocalizedSuccessResponse(
        _response,
        get_response_msg(_ctx, Response::ok, _lang)
    );
}

std::map<std::string, std::string> get_object_state_descriptions(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _state_handles,
    const SessionLang::Enum _lang
) {
    std::map<std::string, std::string> handle_to_description;
    {
        const std::vector<Fred::ObjectStateDescription> all_state_descriptions = Fred::GetObjectStateDescriptions( to_db_handle(_lang) ).exec(_ctx);
        BOOST_FOREACH(const Fred::ObjectStateDescription& state_description, all_state_descriptions) {
            handle_to_description.insert(std::make_pair(state_description.handle, state_description.description));
        }
    }

    std::map<std::string, std::string> result;
    BOOST_FOREACH(const std::string& handle, _state_handles) {
        /* XXX HACK: OK state */
        if( handle == "ok" ) {

            result["ok"] =
                _lang == SessionLang::en
                    ? "Object is without restrictions"
                    : _lang == SessionLang::cz
                        ? "Objekt je bez omezení"
                        : "";

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

}
