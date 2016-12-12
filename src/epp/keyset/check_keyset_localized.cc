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
#include "util/log/context.h"

#include <set>

namespace Epp {
namespace Keyset {
namespace Localized {

namespace {

typedef std::map< std::string, Nullable< KeysetHandleRegistrationObstructin::Enum > > RawResults;

CheckKeysetLocalizedResponse::Results localize_check_keyset_results(
    Fred::OperationContext& _ctx,
    const std::map< std::string, Nullable< KeysetHandleRegistrationObstructin::Enum > >& _keyset_check_results,
    SessionLang::Enum _lang)
{
    typedef std::map< std::string, Nullable< KeysetHandleRegistrationObstructin::Enum > > RawResult;
    typedef std::map< Reason::Enum, std::string > ReasonDescription;
    ReasonDescription reason_description;
    {
        Database::query_param_list params;
        std::string set_of_reason_ids;
        {
            std::set< Reason::Enum > enum_reason_ids;
            for (RawResult::const_iterator result_ptr = _keyset_check_results.begin();
                 result_ptr != _keyset_check_results.end(); ++result_ptr)
            {
                if (!result_ptr->second.isnull()) {
                    const Reason::Enum reason = to_reason(result_ptr->second.get_value());
                    if (enum_reason_ids.insert(reason).second) {
                        if (!set_of_reason_ids.empty()) {
                            set_of_reason_ids += ",";
                        }
                        set_of_reason_ids += "$" + params.add(reason) + "::INTEGER";
                    }
                }
            }
        }
        if (!params.empty()) {
            std::string column_name;
            switch (_lang)
            {
                case SessionLang::en:
                    column_name = "reason";
                    break;
                case SessionLang::cs:
                    column_name = "reason_cs";
                    break;
            }
            if (column_name.empty()) {
                throw UnknownLocalizationLanguage();
            }
            const Database::Result db_res = _ctx.get_conn().exec_params(
                "SELECT id," + column_name + " "
                "FROM enum_reason "
                "WHERE id IN (" + set_of_reason_ids + ")", params);
            if (db_res.size() <= 0) {
                throw MissingLocalizedDescription();
            }
            for (::size_t idx = 0; idx < db_res.size(); ++idx) {
                const Reason::Enum reason = from_description_db_id< Reason >(static_cast< unsigned >(db_res[idx][0]));
                const std::string description = static_cast< std::string >(db_res[idx][1]);
                reason_description[reason] = description;
            }
        }
    }
    CheckKeysetLocalizedResponse::Results localized_result;
    for (RawResults::const_iterator result_ptr = _keyset_check_results.begin();
         result_ptr != _keyset_check_results.end(); ++result_ptr)
    {
        Nullable< CheckKeysetLocalizedResponse::Result > result;
        if (!result_ptr->second.isnull()) {
            CheckKeysetLocalizedResponse::Result data;
            data.state = result_ptr->second.get_value();
            const Reason::Enum reason = to_reason(data.state);
            data.description = reason_description[reason];
            result = data;
        }
        localized_result[result_ptr->first] = result;
    }
    return localized_result;
}

}//namespace Epp::Keyset::{anonymous}

CheckKeysetLocalizedResponse check_keyset_localized(
    const std::set< std::string >& _keyset_handles,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle)
{
    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::CheckKeyset)));

        Fred::OperationContextCreator ctx;

        const std::map< std::string, Nullable< Keyset::KeysetHandleRegistrationObstructin::Enum > > check_keyset_results =
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
