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

#include "src/backend/epp/epp_response_success_localized.hh"

#include "src/backend/epp/epp_response_success.hh"
#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/epp_result_success_localized.hh"
#include "src/backend/epp/localization.hh"
#include "src/backend/epp/session_data.hh"

#include "src/libfred/opcontext.hh"

#include <vector>

namespace Epp {

EppResponseSuccessLocalized::EppResponseSuccessLocalized(
        LibFred::OperationContext& _ctx,
        const EppResponseSuccess& _epp_response,
        const Epp::SessionLang::Enum _session_lang)
    : epp_response_(_epp_response),
      epp_result_(EppResultSuccessLocalized(_ctx, _epp_response.epp_result(), _session_lang))
{ }

} // namespace Epp

