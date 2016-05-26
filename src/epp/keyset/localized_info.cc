#include "src/epp/keyset/localized_info.h"
#include "src/epp/action.h"
#include "src/epp/localization.h"

#include "util/log/context.h"

#include <boost/format.hpp>

namespace Epp {

LocalizedKeysetInfoResult keyset_info(
    const std::string &_keyset_handle,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string &_server_transaction_handle)
{
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
    Logging::Context logging_ctx3(_server_transaction_handle);
    Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::KeySetInfo)));

    try {
        Fred::OperationContextCreator ctx;
        const KeysetInfoData keyset_info_data = keyset_info(ctx, _keyset_handle, _registrar_id);
        LocalizedKeysetInfoData data;
        ctx.commit_transaction();
        return LocalizedKeysetInfoResult(data, create_localized_success_response(Response::ok, ctx, _lang));
    }
    catch (const NonexistentHandle &e) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(ctx, Response::object_not_exist, std::set< Error >(), _lang);
    }
    catch(const LocalizedFailResponse&) {
        throw;
    }
    catch(...) {
        Fred::OperationContextCreator ctx;
        throw create_localized_fail_response(ctx, Response::failed, std::set< Error >(), _lang);
    }
}

}//namespace Epp
