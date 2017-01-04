#include "src/epp/impl/epp_response_failure_localized.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_failure_localized.h"
#include "src/epp/impl/localization.h"

#include "src/fredlib/opcontext.h"

#include <vector>

namespace Epp {

EppResponseFailureLocalized::EppResponseFailureLocalized(
        Fred::OperationContext& _ctx,
        const EppResponseFailure& _epp_response,
        const Epp::SessionLang::Enum& _session_lang)
    : epp_response_(_epp_response)
{

    const std::vector<EppResultFailure>& epp_results = _epp_response.epp_results();

    for (std::vector<EppResultFailure>::const_iterator epp_result = epp_results.begin();
         epp_result != epp_results.end();
         ++epp_result)
    {
        epp_results_.push_back(EppResultFailureLocalized(_ctx, *epp_result, _session_lang));
    }
}

} // namespace Epp
