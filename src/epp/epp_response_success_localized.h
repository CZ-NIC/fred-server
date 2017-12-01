/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EPP_RESPONSE_SUCCESS_LOCALIZED_H_CABCE7C3B4504499B9CBAB066076F463
#define EPP_RESPONSE_SUCCESS_LOCALIZED_H_CABCE7C3B4504499B9CBAB066076F463

#include "src/epp/epp_response_success.h"
#include "src/epp/epp_result_success_localized.h"
#include "src/epp/session_data.h"
#include "src/fredlib/opcontext.h"

#include <vector>

namespace Epp {

class EppResponseSuccessLocalized
{
public:

    EppResponseSuccessLocalized(
            Fred::OperationContext& _ctx,
            const EppResponseSuccess& _epp_response,
            Epp::SessionLang::Enum _session_lang);

    virtual ~EppResponseSuccessLocalized()
    { }

    const EppResultSuccessLocalized& epp_result() const {
        return epp_result_;
    }

private:

    EppResponseSuccess epp_response_;
    EppResultSuccessLocalized epp_result_;
};

} // namespace Epp

#endif
