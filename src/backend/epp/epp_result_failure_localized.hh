/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef EPP_RESULT_FAILURE_LOCALIZED_HH_F77661AD879F4D99A8A8558E620A01EF
#define EPP_RESULT_FAILURE_LOCALIZED_HH_F77661AD879F4D99A8A8558E620A01EF

#include "src/backend/epp/epp_extended_error_localized.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/session_lang.hh"
#include "libfred/opcontext.hh"

#include <boost/optional.hpp>

#include <set>
#include <string>

namespace Epp {

class EppResultFailureLocalized
{
public:
    EppResultFailureLocalized(
            LibFred::OperationContext& _ctx,
            const EppResultFailure& _epp_result,
            SessionLang::Enum _session_lang);

    const char* c_str()const noexcept;

    EppResultCode::Failure epp_result_code()const;

    std::string epp_result_description()const;

    const boost::optional<std::set<EppExtendedErrorLocalized>>& extended_errors()const;
private:
    EppResultFailure epp_result_;
    std::string epp_result_description_;
    boost::optional<std::set<EppExtendedErrorLocalized> > extended_errors_;
};

}//namespace Epp

#endif
