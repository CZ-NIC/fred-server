#include "src/epp/impl/epp_result_success.h"
#include "src/epp/impl/epp_result_success_localized.h"

#include "src/epp/impl/epp_extended_error.h"
#include "src/epp/impl/epp_extended_error_localized.h"
#include "src/epp/impl/localization.h"

#include "src/fredlib/opcontext.h"

#include <boost/optional.hpp>

#include <set>

namespace Epp {

EppResultSuccessLocalized::EppResultSuccessLocalized(
        Fred::OperationContext& _ctx,
        const EppResultSuccess& _epp_result,
        const SessionLang::Enum _session_lang)
    : epp_result_(_epp_result)
{
    epp_result_description_ = get_epp_result_description_localized<EppResultCode::Success>(_ctx, epp_result_.epp_result_code(), _session_lang);
}

} // namespace Epp
