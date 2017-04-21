/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/epp_extended_error_localized.h"
#include "src/epp/epp_extended_error.h"
#include "src/epp/session_lang.h"
#include "src/epp/localization.h"
#include "src/fredlib/opcontext.h"

#include "src/epp/reason.h"
#include "src/epp/param.h"

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
