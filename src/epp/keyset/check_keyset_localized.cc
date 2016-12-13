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

#include "src/epp/keyset/check_keyset.h"
#include "src/epp/keyset/check_keyset_localized.h"

#include "src/epp/impl/action.h"
#include "src/epp/impl/exception.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/response.h"
#include "src/epp/impl/util.h"
#include "src/epp/keyset/impl/keyset_handle_registration_obstruction.h"
#include "src/fredlib/opcontext.h"
#include "util/log/context.h"

#include <map>
#include <set>
#include <stdexcept>

namespace Epp {
namespace Keyset {
namespace Localized {

namespace {

typedef std::map< std::string, Nullable< KeysetHandleRegistrationObstruction::Enum > > RawResults;

std::set<unsigned> convert_to_description_db_ids(const RawResults& _keyset_check_results) {
    std::set<unsigned> states_ids;
    for (RawResults::const_iterator result_ptr = _keyset_check_results.begin();
            result_ptr != _keyset_check_results.end();
            ++result_ptr)
    {
        if (!result_ptr->second.isnull()) {
            states_ids.insert(to_description_db_id(KeysetHandleRegistrationObstruction::to_reason(result_ptr->second.get_value())));
        }
    }

    return states_ids;
}

CheckKeysetLocalizedResponse::Results localize_check_keyset_results(
    Fred::OperationContext& _ctx,
    const std::map< std::string, Nullable< KeysetHandleRegistrationObstruction::Enum > >& _keyset_check_results,
    const SessionLang::Enum _lang)
{
    typedef std::map< KeysetHandleRegistrationObstruction::Enum, std::string > ReasonDescription;

    // get_localized_description_of_obstruction
    const ReasonDescription reasons_descriptions =
        get_reasons_descriptions<KeysetHandleRegistrationObstruction>(_ctx, convert_to_description_db_ids(_keyset_check_results), _lang);
    if (reasons_descriptions.size() <= 0) {
        throw MissingLocalizedDescription();
    }

    CheckKeysetLocalizedResponse::Results localized_result;
    for (RawResults::const_iterator result_ptr = _keyset_check_results.begin();
         result_ptr != _keyset_check_results.end(); ++result_ptr)
    {
        Nullable< CheckKeysetLocalizedResponse::Result > result;
        if (!result_ptr->second.isnull()) {
            CheckKeysetLocalizedResponse::Result data;
            data.state = result_ptr->second.get_value();
            data.description = reasons_descriptions.at(data.state);
            result = data;
        }
        localized_result[result_ptr->first] = result;
    }
    return localized_result;
}

}//namespace Epp::Keyset::{anonymous}

CheckKeysetLocalizedResponse check_keyset_localized(
    const std::set< std::string >& _keyset_handles,
    const unsigned long long _registrar_id,
    const SessionLang::Enum _lang,
    const std::string& _server_transaction_handle)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::CheckKeyset)));

        Fred::OperationContextCreator ctx;

        const std::map< std::string, Nullable< Keyset::KeysetHandleRegistrationObstruction::Enum > > check_keyset_results =
            check_keyset(
                    ctx,
                    _keyset_handles,
                    _registrar_id);

        const LocalizedSuccessResponse ok_response =
            create_localized_success_response(
                    ctx,
                    Response::ok,
                    _lang);

        const CheckKeysetLocalizedResponse::Results localized_check_keyset_results =
            localize_check_keyset_results(
                    ctx,
                    check_keyset_results,
                    _lang);

        return CheckKeysetLocalizedResponse(
                ok_response,
                localized_check_keyset_results);
    }
    catch (const AuthErrorServerClosingConnection&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang);
    }
    catch (const std::exception& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info(std::string("check_keyset_localized failure: ") + e.what());
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::failed,
            std::set< Error >(),
            _lang);
    }
    catch (...) {
        Fred::OperationContextCreator exception_localization_ctx;
        exception_localization_ctx.get_log().info("unexpected exception in check_keyset_localized function");
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::failed,
            std::set< Error >(),
            _lang);
    }
}

} // namespace Epp::Keyset::Localized
} // namespace Epp::Keyset
} // namespace Epp
