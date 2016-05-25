#include "src/epp/keyset/localized_info.h"
#include "src/epp/action.h"

#include "util/log/context.h"

#include <boost/format.hpp>

namespace Epp {

LocalizedKeysetInfoResult keyset_info(
    const std::string &_keyset_handle,
    unsigned long long _registrar_id,
    SessionLang::Enum _object_state_description_lang,
    const std::string &_server_transaction_handle)
{
    Logging::Context logging_ctx1("rifd");
    Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
    Logging::Context logging_ctx3(_server_transaction_handle);
    Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast< unsigned >(Action::KeySetInfo)));

    /* since no changes are comitted this transaction is reused for everything */

}

}//namespace Epp
