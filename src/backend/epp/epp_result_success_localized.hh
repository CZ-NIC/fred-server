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

#ifndef EPP_RESULT_SUCCESS_LOCALIZED_HH_D5B711D9123A4844BD2942786CC20589
#define EPP_RESULT_SUCCESS_LOCALIZED_HH_D5B711D9123A4844BD2942786CC20589

#include "src/backend/epp/epp_result_success.hh"
#include "src/backend/epp/session_lang.hh"
#include "src/libfred/opcontext.hh"

#include <boost/optional.hpp>

#include <set>
#include <string>

namespace Epp {

class EppResultSuccessLocalized
{
public:
    EppResultSuccessLocalized(
            LibFred::OperationContext& _ctx,
            const EppResultSuccess& _epp_result,
            SessionLang::Enum _session_lang);

    const char* c_str()const noexcept;

    EppResultCode::Success epp_result_code()const;

    std::string epp_result_description()const;
private:
    EppResultSuccess epp_result_;
    std::string epp_result_description_;
};

}//namespace Epp

#endif
