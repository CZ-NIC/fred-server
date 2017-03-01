#include "src/epp/impl/epp_extended_error_localized.h"
#include "src/epp/impl/epp_extended_error.h"
#include "src/epp/session_lang.h"
#include "src/epp/impl/localization.h"
#include "src/fredlib/opcontext.h"

#include "src/epp/impl/reason.h"
#include "src/epp/impl/param.h"

namespace Epp {

EppExtendedErrorLocalized::EppExtendedErrorLocalized(
        Fred::OperationContext& _ctx,
        const EppExtendedError& _epp_extended_error,
        const SessionLang::Enum _lang)
    : epp_extended_error_(_epp_extended_error)
{
    reason_description_ = get_reason_description_localized(_ctx, epp_extended_error_.reason(), _lang);
}

bool operator < (const Epp::EppExtendedErrorLocalized& _lhs, const Epp::EppExtendedErrorLocalized& _rhs)
{
    return _lhs.epp_extended_error_< _rhs.epp_extended_error_;
}


} // namespace Epp
