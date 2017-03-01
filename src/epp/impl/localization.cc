#include "src/epp/impl/localization.h"

#include "src/epp/error.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/epp/session_lang.h"
#include "src/epp/impl/util.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/map_at.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace Epp {

std::string get_reason_description_localized_column_name(SessionLang::Enum _lang)
{
    switch (_lang)
    {
        case SessionLang::en:
            return "reason";

        case SessionLang::cs:
            return "reason_cs";
    }
    throw UnknownLocalizationLanguage();
}

std::string get_result_description_localized_column_name(SessionLang::Enum _lang)
{
    switch (_lang)
    {
        case SessionLang::en:
            return "status";

        case SessionLang::cs:
            return "status_cs";
    }
    throw UnknownLocalizationLanguage();
}

std::string get_reason_description_localized(
    Fred::OperationContext& _ctx,
    Reason::Enum _reason,
    SessionLang::Enum _lang)
{
    const std::string column_name = get_reason_description_localized_column_name(_lang);

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

    return static_cast< std::string >(res[0][0]);
}

namespace {

std::string get_success_state_localized_description(SessionLang::Enum _lang)
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
std::map<std::string, std::string> localize_object_states_deprecated(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _state_handles,
    const SessionLang::Enum _lang
) {
    std::map<std::string, std::string> handle_to_description;
    {
        const std::vector< Fred::ObjectStateDescription > all_state_descriptions =
            Fred::GetObjectStateDescriptions(SessionLang::to_db_handle(_lang)).exec(_ctx);
        BOOST_FOREACH(const Fred::ObjectStateDescription& state_description, all_state_descriptions) {
            handle_to_description.insert(std::make_pair(state_description.handle, state_description.description));
        }
    }

    std::map<std::string, std::string> result;
    BOOST_FOREACH(const std::string& handle, _state_handles) {
        /* XXX HACK: OK state */
        if (handle == "ok") {
            result["ok"] = get_success_state_localized_description(_lang);
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

ObjectStatesLocalized localize_object_states(
    Fred::OperationContext& _ctx,
    const std::set< Fred::Object_State::Enum >& _states,
    SessionLang::Enum _lang)
{
    typedef std::vector< Fred::ObjectStateDescription > ObjectStateDescriptions;
    const ObjectStateDescriptions all_state_descriptions =
        Fred::GetObjectStateDescriptions(SessionLang::to_db_handle(_lang)).exec(_ctx);

    ObjectStatesLocalized states;
    for (ObjectStateDescriptions::const_iterator state_ptr = all_state_descriptions.begin();
         state_ptr != all_state_descriptions.end(); ++state_ptr)
    {
        const Fred::Object_State::Enum state =
            Conversion::Enums::from_db_handle< Fred::Object_State >(state_ptr->handle);
        if (_states.find(state) != _states.end()) {
            states.descriptions[state] = state_ptr->description;
        }
    }
    states.success_state_localized_description = get_success_state_localized_description(_lang);
    return states;
}

/**
 * @returns untyped postgres array
 * Caller should cast it properly before using in query.
 */
std::string convert_values_to_pg_array(const std::set<unsigned>& _input) {
    std::vector<std::string> string_values;
    string_values.reserve(_input.size());

    BOOST_FOREACH(unsigned elem, _input) {
        string_values.push_back( boost::lexical_cast<std::string>(elem) );
    }

    return "{" + boost::algorithm::join(string_values, ", ") + "}";
}



}
