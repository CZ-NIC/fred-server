/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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


#include "src/epp/nsset/nsset_check.h"

#include "src/epp/action.h"
#include "src/epp/nsset/nsset_check_impl.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"
#include "util/log/context.h"
#include "util/map_at.h"
#include "util/util.h"

#include <set>
#include <utility>

#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace Epp {

/**
 * @returns untyped postgres array
 * Caller should cast it properly before using in query.
 */
static std::string convert_to_pg_array(const std::set<unsigned>& _input) {
    std::vector<std::string> string_values;
    string_values.reserve(_input.size());

    BOOST_FOREACH(unsigned elem, _input) {
        string_values.push_back( boost::lexical_cast<std::string>(elem) );
    }

    return "{" + boost::algorithm::join(string_values, ", ") + "}";
}

static std::set<unsigned> convert_to_description_db_ids(const std::set<NssetHandleRegistrationObstruction::Enum>& _obstructions) {
    std::set<unsigned> states_ids;

    for(
        std::set<NssetHandleRegistrationObstruction::Enum>::const_iterator it = _obstructions.begin();
        it != _obstructions.end();
        ++it
    ) {
        states_ids.insert( to_description_db_id(NssetHandleRegistrationObstruction::to_reason(*it) ));
    }

    return states_ids;
}

static std::map<NssetHandleRegistrationObstruction::Enum, std::string> get_localized_description_of_obstructions(
    Fred::OperationContext& _ctx,
    const std::set<NssetHandleRegistrationObstruction::Enum>& _obstructions,
    const SessionLang::Enum _lang
) {
    std::map<unsigned, NssetHandleRegistrationObstruction::Enum> reason_id_obstruction_map;
    for(std::set<NssetHandleRegistrationObstruction::Enum>::const_iterator it = _obstructions.begin();
        it != _obstructions.end();
        ++it
    ) {
        reason_id_obstruction_map.insert(
            std::make_pair(to_description_db_id(
                NssetHandleRegistrationObstruction::to_reason(*it)), *it));
    }

    const std::string column_name =
        _lang == SessionLang::en
        ?   "reason"
        :   _lang == SessionLang::cs
            ?   "reason_cs"
            :   throw UnknownLocalizationLanguage();

    const Database::Result db_res = _ctx.get_conn().exec_params(
        "SELECT "
            "id AS id_, " +
            column_name + " AS reason_txt_ "
        "FROM enum_reason "
        "WHERE id = ANY( $1::integer[] ) ",
        Database::query_param_list(
            convert_to_pg_array( convert_to_description_db_ids(_obstructions) )
        )
    );

    if(db_res.size() < _obstructions.size()) {
        throw MissingLocalizedDescription();
    }

    std::map<NssetHandleRegistrationObstruction::Enum, std::string> result;

    for(unsigned long i = 0; i < db_res.size(); ++i) {
        result.insert(
            std::make_pair(
                map_at(reason_id_obstruction_map, static_cast<unsigned>(db_res[i]["id_"])),
                static_cast<std::string>(db_res[i]["reason_txt_"]))
        );
    }

    return result;
}

static std::map<std::string, boost::optional<LocalizedNssetHandleRegistrationObstruction> > create_localized_check_response(
    Fred::OperationContext& _ctx,
    const std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> >& _check_results,
    const SessionLang::Enum _lang
) {
    const std::map<NssetHandleRegistrationObstruction::Enum, std::string> localized_description_of_obstructions = get_localized_description_of_obstructions(
            _ctx, static_cast< const std::set<NssetHandleRegistrationObstruction::Enum>& >(
                Util::set_of<NssetHandleRegistrationObstruction::Enum>
                    (NssetHandleRegistrationObstruction::invalid_handle)
                    (NssetHandleRegistrationObstruction::protected_handle)
                    (NssetHandleRegistrationObstruction::registered_handle)),
            _lang
        );

    std::map<std::string, boost::optional<LocalizedNssetHandleRegistrationObstruction> > result;
    for(std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> >::const_iterator nsset_it = _check_results.begin();
        nsset_it != _check_results.end(); ++nsset_it)
    {
        result.insert(std::make_pair(nsset_it->first,
            nsset_it->second.isnull()
                ?   boost::optional<LocalizedNssetHandleRegistrationObstruction>()
                :   boost::optional<LocalizedNssetHandleRegistrationObstruction>(
                        LocalizedNssetHandleRegistrationObstruction(
                            nsset_it->second.get_value(),
                            map_at(localized_description_of_obstructions, nsset_it->second.get_value())
                        )
                    )
            )
        );
    }
    return result;
}

LocalizedCheckNssetResponse nsset_check(
    const std::set<std::string>& _nsset_handles,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
) {
    Logging::Context logging_ctx("rifd");
    Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
    Logging::Context logging_ctx3(_server_transaction_handle);
    Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::NSsetCheck) ) );

    if( _registrar_id == 0 ) {
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

    const std::map<std::string, Nullable<NssetHandleRegistrationObstruction::Enum> > impl_result = nsset_check_impl(ctx, _nsset_handles);

    return LocalizedCheckNssetResponse(
       create_localized_success_response(
           Response::ok,
           ctx,
           _lang
       ),
       create_localized_check_response( ctx, impl_result, _lang )
    );
}

}
