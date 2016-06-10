#include "src/epp/keyset/check.h"
#include "src/epp/keyset/localized_check.h"

#include "src/epp/action.h"
#include "src/epp/exception.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"
#include "util/log/context.h"

#include <set>

namespace Epp {
namespace KeySet {
namespace Localized {

namespace {

typedef std::map< std::string, Nullable< HandleCheckResult::Enum > > RawResults;

HandlesCheck::Results localize_check_results(
    Fred::OperationContext &_ctx,
    const std::map< std::string, Nullable< HandleCheckResult::Enum > > &_keyset_check_results,
    SessionLang::Enum _lang)
{
    typedef std::map< std::string, Nullable< HandleCheckResult::Enum > > RawResult;
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
                case SessionLang::cz:
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
    HandlesCheck::Results localized_result;
    for (RawResults::const_iterator result_ptr = _keyset_check_results.begin();
         result_ptr != _keyset_check_results.end(); ++result_ptr)
    {
        Nullable< HandlesCheck::Result > result;
        if (!result_ptr->second.isnull()) {
            HandlesCheck::Result data;
            data.state = result_ptr->second.get_value();
            const Reason::Enum reason = to_reason(data.state);
            data.description = reason_description[reason];
            result = data;
        }
        localized_result[result_ptr->first] = result;
    }
    return localized_result;
}

}//namespace Epp::KeySet::{anonymous}

HandlesCheck check(
    const std::set< std::string > &_keyset_handles,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle)
{
    Logging::Context logging_ctx("rifd");
    Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
    Logging::Context logging_ctx3(_server_transaction_handle);
    Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::KeySetCheck)));

    try {
        Fred::OperationContextCreator ctx;

        static const unsigned long long invalid_registrar_id = 0;
        if (_registrar_id == invalid_registrar_id) {
            throw create_localized_fail_response(
                ctx,
                Response::authentication_error_server_closing_connection,
                std::set<Error>(),
                _lang);
        }

        const std::map< std::string, Nullable< KeySet::HandleCheckResult::Enum > > keyset_check_results =
            keyset_check(ctx, _keyset_handles);

        return HandlesCheck(create_localized_success_response(Response::ok, ctx, _lang),
                            localize_check_results(ctx, keyset_check_results, _lang));
    }
    catch (const LocalizedFailResponse&) {
        throw;
    }
    catch (const std::exception &e) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info(std::string("get_localized_check failure: ") + e.what());
        throw create_localized_fail_response(ctx, Response::failed, std::set< Error >(), _lang);
    }
    catch (...) {
        Fred::OperationContextCreator ctx;
        ctx.get_log().info("unexpected exception in get_localized_check function");
        throw create_localized_fail_response(ctx, Response::failed, std::set< Error >(), _lang);
    }
}

}//namespace Epp::KeySet::Localized
}//namespace Epp::KeySet
}//namespace Epp
