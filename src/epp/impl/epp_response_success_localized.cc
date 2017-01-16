#include "src/epp/impl/epp_response_success_localized.h"

#include "src/epp/impl/epp_response_success.h"
#include "src/epp/impl/epp_result_success.h"
#include "src/epp/impl/epp_result_success_localized.h"
#include "src/epp/impl/localization.h"
#include "src/epp/impl/session_data.h"

#include "src/fredlib/opcontext.h"

#include <vector>

namespace Epp {

EppResponseSuccessLocalized::EppResponseSuccessLocalized(
        Fred::OperationContext& _ctx,
        const EppResponseSuccess& _epp_response,
        const Epp::SessionLang::Enum& _session_lang)
    : epp_response_(_epp_response),
      epp_result_(EppResultSuccessLocalized(_ctx, _epp_response.epp_result(), _session_lang))
{ }

} // namespace Epp

