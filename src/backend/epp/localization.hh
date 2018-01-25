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

#ifndef LOCALIZATION_HH_0C448DBDBB9043D39BD1BE736A402EB1
#define LOCALIZATION_HH_0C448DBDBB9043D39BD1BE736A402EB1

#include "src/backend/epp/contact/status_value.hh"
#include "src/backend/epp/domain/status_value.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/keyset/status_value.hh"
#include "src/backend/epp/nsset/status_value.hh"
#include "src/backend/epp/object_states_localized.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/session_lang.hh"
#include "src/libfred/object_state/get_object_state_descriptions.hh"
#include "src/util/enum_conversion.hh"

#include "src/libfred/opcontext.hh"

#include <map>
#include <set>
#include <string>

namespace Epp {

std::string get_reason_description_localized_column_name(SessionLang::Enum _lang);
std::string get_result_description_localized_column_name(SessionLang::Enum _lang);

std::string get_reason_description_localized(
        LibFred::OperationContext& _ctx,
        Reason::Enum _reason,
        SessionLang::Enum _lang);

template <typename T>
std::string get_epp_result_description_localized(
        LibFred::OperationContext& _ctx,
        T _epp_result_code,
        SessionLang::Enum _session_lang)
{
    const std::string column_name = get_result_description_localized_column_name(_session_lang);

    const Database::Result res = _ctx.get_conn().exec_params(
            "SELECT " + column_name + " "
            "FROM enum_error "
            "WHERE id=$1::integer",
            Database::query_param_list(EppResultCode::to_description_db_id(_epp_result_code)));

    if (res.size() < 1) {
        throw MissingLocalizedDescription();
    }

    if (1 < res.size()) {
        throw std::runtime_error("0 or 1 row expected");
    }

    return static_cast<std::string>(res[0][0]);
}

namespace {

inline std::string get_success_state_localized_description(SessionLang::Enum _lang)
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

template <typename T>
ObjectStatesLocalized<T> localize_object_states(
        LibFred::OperationContext& _ctx,
        const std::set<typename T::Enum>& _status_values,
        SessionLang::Enum _lang)
{
    ObjectStatesLocalized<T> status_values_localized;
    for (typename std::set<typename T::Enum>::const_iterator status_value = _status_values.begin();
            status_value != _status_values.end();
            ++status_value)
    {
        const Database::Result res = _ctx.get_conn().exec_params(
            "SELECT eosd.description "
            "FROM enum_object_states_desc eosd "
                "JOIN enum_object_states eos ON eosd.state_id = eos.id "
                    "AND eos.external = TRUE "
            "WHERE eos.name = $1::text "
            "AND UPPER(eosd.lang) = UPPER($2::text)",
            Database::query_param_list(Conversion::Enums::to_db_handle(Conversion::Enums::to_fred_object_state(*status_value)))
            (SessionLang::to_db_handle(_lang)));

        if (res.size() < 1) {
            throw MissingLocalizedDescription();
        }

        if (res.size() > 1) {
            throw std::runtime_error("0 or 1 row expected");
        }

        status_values_localized.descriptions[*status_value] = static_cast<std::string>(res[0][0]);
    }
    status_values_localized.success_state_localized_description = get_success_state_localized_description(_lang);

    return status_values_localized;
}

/**
 * @returns untyped postgres array
 * Caller should cast it properly before using in query.
 */
std::string convert_values_to_pg_array(const std::set<unsigned>& _input);

template <typename T>
std::set<unsigned> convert_reasons_to_reasons_descriptions_db_ids(const std::map<std::string, Nullable<typename T::Enum> >& _reasons) {
    std::set<unsigned> reasons_descriptions_db_ids;
    for (typename std::map<std::string, Nullable<typename T::Enum> >::const_iterator result_ptr = _reasons.begin();
         result_ptr != _reasons.end();
         ++result_ptr)
    {
        if (!result_ptr->second.isnull()) {
            reasons_descriptions_db_ids.insert(to_description_db_id(T::to_reason(result_ptr->second.get_value())));
        }
    }

    return reasons_descriptions_db_ids;
}

/**
* @brief Gives localized reasons descriptions for provided reasons IDs
*
* @tparam T  requested reason type
* @param _ctx
* @param _reasons_ids  IDs of reasons which will be localized
* @param _lang
*
* @return  localized reasons descriptions
*/
template <typename T>
std::map<typename T::Enum, std::string> get_reasons_descriptions_localized(
        const LibFred::OperationContext& _ctx,
        const std::set<unsigned>& _reasons_ids,
        const SessionLang::Enum _lang)
{
    std::map<typename T::Enum, std::string> reasons_descriptions_localized;
    if (!_reasons_ids.empty()) {
        const std::string reason_description_column_name = get_reason_description_localized_column_name(_lang);
        const Database::Result db_result = _ctx.get_conn().exec_params(
                "SELECT "
                    "id AS reason_id, " +
                    reason_description_column_name + " AS reason_description_localized "
                "FROM enum_reason "
                "WHERE id = ANY( $1::integer[] ) ",
                Database::query_param_list(
                    convert_values_to_pg_array(_reasons_ids)
                ));

        if (db_result.size() < _reasons_ids.size()) {
            throw MissingLocalizedDescription();
        }

        for (::size_t i = 0; i < db_result.size(); ++i) {

            const typename T::Enum reason =
                    T::from_reason(
                            from_description_db_id<Reason>(
                                    static_cast<unsigned>(db_result[i]["reason_id"])));

            const std::string reason_description_localized =
                    static_cast<std::string>(db_result[i]["reason_description_localized"]);

            reasons_descriptions_localized[reason] = reason_description_localized;

        }
    }
    return reasons_descriptions_localized;
}

/**
* @brief Decorate each check results with localized reason description.
*
* @tparam F  from type (not localized check result type)
* @tparam T  to type (localized check result type)
* @tparam O  makes T either Optional or boost::optional
* @param _ctx
* @param _check_results
* @param _lang
*
* @returns  localized check results
*/
template <typename F, typename T, template <class> class O>
std::map<std::string, O<T> > localize_check_results(
        LibFred::OperationContext& _ctx,
        const std::map<std::string, Nullable<typename F::Enum> >& _check_results,
        const SessionLang::Enum _lang)
{
    // get_localized_descriptions
    const std::map<typename F::Enum, std::string> reasons_descriptions_localized =
            get_reasons_descriptions_localized<F>(
                    _ctx,
                    convert_reasons_to_reasons_descriptions_db_ids<F>(_check_results),
                    _lang);

    // for each check result
    std::map<std::string, O<T> > localized_results;
    for (typename std::map<std::string, Nullable<typename F::Enum> >::const_iterator check_result = _check_results.begin();
         check_result != _check_results.end();
         ++check_result)
    {
        // keep handle
        const std::string handle = check_result->first;

        // but reason
        Nullable<typename F::Enum>
        const reason = check_result->second;

        // ...decorate with reason_description_localized
        const O<T> reason_description_localized =
                reason.isnull()
                        ? O<T>()
                        : O<T>(T(
                                  reason.get_value(),
                                  reasons_descriptions_localized.at(reason.get_value())));

        localized_results.insert(std::make_pair(handle, reason_description_localized));
    }

    return localized_results;
}

} // namespace Epp

#endif