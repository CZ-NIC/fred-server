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

#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/epp_result_success_localized.hh"

#include "src/backend/epp/epp_extended_error.hh"
#include "src/backend/epp/epp_extended_error_localized.hh"
#include "src/backend/epp/localization.hh"

#include "src/libfred/opcontext.hh"

#include <boost/optional.hpp>

#include <set>

namespace Epp {

EppResultSuccessLocalized::EppResultSuccessLocalized(
        LibFred::OperationContext& _ctx,
        const EppResultSuccess& _epp_result,
        const SessionLang::Enum _session_lang)
    : epp_result_(_epp_result),
      epp_result_description_(
              get_epp_result_description_localized<EppResultCode::Success>(_ctx, epp_result_.epp_result_code(), _session_lang))
{ }

const char* EppResultSuccessLocalized::c_str()const noexcept
{
    return epp_result_description_.c_str();
}

EppResultCode::Success EppResultSuccessLocalized::epp_result_code()const
{
    return epp_result_.epp_result_code();
}

std::string EppResultSuccessLocalized::epp_result_description()const
{
    return epp_result_description_;
}

} // namespace Epp
