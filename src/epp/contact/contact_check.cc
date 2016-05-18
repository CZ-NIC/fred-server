#include "src/epp/contact/contact_check.h"

#include "src/epp/action.h"
#include "src/epp/contact/contact_check_impl.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"
#include "util/log/context.h"
#include "util/map_at.h"

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

static std::set<unsigned> convert_to_description_db_ids(const std::set<ContactHandleRegistrationObstruction::Enum>& _obstructions) {
    std::set<unsigned> states_ids;

    for(
        std::set<ContactHandleRegistrationObstruction::Enum>::const_iterator it = _obstructions.begin();
        it != _obstructions.end();
        ++it
    ) {
        states_ids.insert( to_description_db_id(*it) );
    }

    return states_ids;
}

static std::map<ContactHandleRegistrationObstruction::Enum, std::string> get_localized_description_of_obstructions(
    Fred::OperationContext& _ctx,
    const std::set<ContactHandleRegistrationObstruction::Enum>& _obstructions,
    const SessionLang::Enum _lang
) {

    const std::string column_name =
        _lang == SessionLang::en
        ?   "reason"
        :   _lang == SessionLang::cz
            ?   "reason_cz"
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

    if(db_res.size() < 1) {
        throw MissingLocalizedDescription();
    }

    std::map<ContactHandleRegistrationObstruction::Enum, std::string> result;

    for(unsigned long i = 0; i < db_res.size(); ++i) {
        result.insert(
            std::make_pair(
                from_description_db_id<ContactHandleRegistrationObstruction>( static_cast<unsigned>(db_res[i]["id_"]) ),
                static_cast<std::string>(db_res[i]["reason_txt_"])
            )
        );
    }

    return result;
}

static std::map<std::string, LazyNullable<LocalizedContactHandleRegistrationObstruction> > create_localized_check_response(
    Fred::OperationContext& _ctx,
    const std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> >& _check_results,
    const SessionLang::Enum _lang
) {
    const std::map<ContactHandleRegistrationObstruction::Enum, std::string> localized_description_of_obstructions = get_localized_description_of_obstructions(
        _ctx,
        ContactHandleRegistrationObstruction::get_all_values(),
        _lang
    );

    std::map<std::string, LazyNullable<LocalizedContactHandleRegistrationObstruction> > result;

    for(std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> >::const_iterator contact_it = _check_results.begin();
        contact_it != _check_results.end();
        ++contact_it
    ) {
        try {
            result.insert(
                std::make_pair(
                    contact_it->first,
                    contact_it->second.isnull()
                    ?   LazyNullable<LocalizedContactHandleRegistrationObstruction>()
                    :   LazyNullable<LocalizedContactHandleRegistrationObstruction>(
                            LocalizedContactHandleRegistrationObstruction(
                                contact_it->second.get_value(),
                                map_at(localized_description_of_obstructions, contact_it->second.get_value())
                            )
                        )
                )
            );
        } catch(const std::out_of_range&) {
            throw MissingLocalizedDescription();
        }
    }

    return result;
}

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

        const std::map<std::string, Nullable<ContactHandleRegistrationObstruction::Enum> > impl_result = contact_check_impl(ctx, _contact_handles);

        return LocalizedCheckContactResponse(
            create_localized_success_response(
                Response::ok,
                ctx,
                _lang
            ),
            create_localized_check_response( ctx, impl_result, _lang )
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

}
