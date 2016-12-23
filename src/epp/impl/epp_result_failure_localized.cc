#include "src/epp/impl/epp_result_failure.h"
#include "src/epp/impl/epp_result_failure_localized.h"

#include "src/epp/impl/epp_extended_error.h"
#include "src/epp/impl/epp_extended_error_localized.h"
#include "src/epp/impl/localization.h"

#include "src/fredlib/opcontext.h"

#include <boost/optional.hpp>

#include <set>

namespace Epp {

EppResultFailureLocalized::EppResultFailureLocalized(
        Fred::OperationContext& _ctx,
        const EppResultFailure& _epp_result_failure,
        const SessionLang::Enum& _session_lang)
    : epp_result_failure_(_epp_result_failure)
{
    epp_result_description_ = get_epp_result_description_localized(_ctx, epp_result_failure_.epp_result_code(), _session_lang);

    const boost::optional<std::set<EppExtendedError> >& extended_errors = _epp_result_failure.extended_errors();
    if (extended_errors) {
        extended_errors_ = std::set<EppExtendedErrorLocalized>();
        for (std::set<EppExtendedError>::const_iterator extended_error = extended_errors->begin();
             extended_error != extended_errors->end();
             ++extended_error)
        {
            extended_errors_->insert(EppExtendedErrorLocalized(_ctx, *extended_error, _session_lang));
        }
    }
}

} // namespace Epp
