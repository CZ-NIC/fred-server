#include "src/epp/keyset/localized_info.h"
#include "src/epp/keyset/info.h"
#include "src/epp/action.h"
#include "src/epp/localization.h"

#include "util/log/context.h"

#include <boost/format.hpp>

namespace Epp {

LocalizedKeysetInfoResult localized_keyset_info(
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
        data.handle = keyset_info_data.handle;
        data.roid = keyset_info_data.roid;
        data.sponsoring_registrar_handle = keyset_info_data.sponsoring_registrar_handle;
        data.creating_registrar_handle = keyset_info_data.creating_registrar_handle;
        data.last_update_registrar_handle = keyset_info_data.last_update_registrar_handle;
        data.states_description = get_localized_object_state(ctx, keyset_info_data.states, _lang);
        data.crdate = keyset_info_data.crdate;
        data.last_update = keyset_info_data.last_update;
        data.last_transfer = keyset_info_data.last_transfer;
        data.auth_info_pw = keyset_info_data.auth_info_pw;
        data.ds_records = keyset_info_data.ds_records;
        data.dns_keys = keyset_info_data.dns_keys;
        data.tech_contacts = keyset_info_data.tech_contacts;
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
