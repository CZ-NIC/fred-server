/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 */

#ifndef LOCALIZATION_H_6BA4B6A62A5D41F0A69D20DE5B507D2E
#define LOCALIZATION_H_6BA4B6A62A5D41F0A69D20DE5B507D2E

#include "src/epp/error.h"
#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/object_states_localized.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/session_lang.h"

#include "src/fredlib/opcontext.h"

#include <set>

namespace Epp {

LocalizedSuccessResponse create_localized_success_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    SessionLang::Enum _lang
);

LocalizedFailResponse create_localized_fail_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    const std::set<Error>& _errors,
    SessionLang::Enum _lang
);

LocalizedFailResponse create_localized_fail_response(
    Fred::OperationContext& _ctx,
    const Response::Enum& _response,
    const Error& _error,
    SessionLang::Enum _lang
);

std::map<std::string, std::string> localize_object_states_deprecated(
    Fred::OperationContext& _ctx,
    const std::set<std::string>& _state_handles,
    SessionLang::Enum _lang
);

ObjectStatesLocalized localize_object_states(
        Fred::OperationContext& _ctx,
        const std::set<Fred::Object_State::Enum>& _states,
        SessionLang::Enum _lang);

std::string get_db_table_enum_reason_column_name_for_language(SessionLang::Enum _lang);
std::string get_db_table_enum_error_column_name_for_language(SessionLang::Enum _lang);

std::string convert_values_to_pg_array(const std::set<unsigned>& _input);

/**
 * @returns untyped postgres array
 * Caller should cast it properly before using in query.
 */
template <typename T>
std::set<unsigned> convert_values_to_description_db_ids(const std::map<std::string, Nullable<typename T::Enum> >& _check_results) {
    std::set<unsigned> states_ids;
    for (typename std::map<std::string, Nullable<typename T::Enum> >::const_iterator result_ptr = _check_results.begin();
         result_ptr != _check_results.end();
         ++result_ptr)
    {
        if (!result_ptr->second.isnull()) {
            states_ids.insert(to_description_db_id(T::to_reason(result_ptr->second.get_value())));
        }
    }

    return states_ids;
}

template <typename T>
std::map<typename T::Enum, std::string> get_reasons_descriptions(const Fred::OperationContext& _ctx, const std::set<unsigned>& _reasons_ids, const SessionLang::Enum _lang) {
    std::map<typename T::Enum, std::string> result;
    if(!_reasons_ids.empty()) {
        const std::string column_name = get_db_table_enum_reason_column_name_for_language(_lang);
        const Database::Result db_result = _ctx.get_conn().exec_params(
                "SELECT "
                    "id AS reason_id, " +
                    column_name + " AS reason_description "
                "FROM enum_reason "
                "WHERE id = ANY( $1::integer[] ) ",
                Database::query_param_list(
                    convert_values_to_pg_array(_reasons_ids)
                ));

        if (db_result.size() < _reasons_ids.size()) {
            throw MissingLocalizedDescription();
        }

        for (::size_t i = 0; i < db_result.size(); ++i) {
            const typename T::Enum reason = T::from_reason(from_description_db_id<Reason>(static_cast<unsigned>(db_result[i]["reason_id"])));
            const std::string description = static_cast<std::string>(db_result[i]["reason_description"]);
            result[reason] = description;
        }
    }
    return result;
}

template <typename F, typename T, template <class> class O>
std::map<std::string, O<T> > localize_check_results(
        Fred::OperationContext& _ctx,
        const std::map<std::string, Nullable<typename F::Enum> >& _check_results,
        const SessionLang::Enum _lang)
{
    // get_localized_descriptions_of_obstruction
    const std::map<typename F::Enum, std::string> reasons_descriptions =
            get_reasons_descriptions<F>(
                    _ctx,
                    convert_values_to_description_db_ids<F>(_check_results),
                    _lang);

    // for each check result
    std::map<std::string, O<T> > localized_result;
    for (typename std::map<std::string, Nullable<typename F::Enum> >::const_iterator check_ptr = _check_results.begin();
         check_ptr != _check_results.end();
         ++check_ptr)
    {
        // keep handle
        const std::string handle = check_ptr->first;

        // but obstruction::Enum
        Nullable<typename F::Enum>
        obstruction = check_ptr->second;

        // convert to localized_obstruction (not Enum)
        O<T>
        localized_obstruction = obstruction.isnull()
            ? O<T>()
            : O<T>(
                T(
                    obstruction.get_value(),
                    reasons_descriptions.at(obstruction.get_value())
                )
              );

        localized_result.insert(std::make_pair(handle, localized_obstruction));
    }

    return localized_result;
}

} // namespace Epp

#endif
