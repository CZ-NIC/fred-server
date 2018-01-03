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

#ifndef EPP_RESULT_FAILURE_LOCALIZED_HH_F77661AD879F4D99A8A8558E620A01EF
#define EPP_RESULT_FAILURE_LOCALIZED_HH_F77661AD879F4D99A8A8558E620A01EF

#include "src/backend/epp/epp_extended_error_localized.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/session_lang.hh"
#include "src/libfred/opcontext.hh"

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

    virtual const char* c_str() const noexcept {
        return epp_result_description_.c_str();
    }

    EppResultCode::Failure epp_result_code() const {
        return epp_result_.epp_result_code();
    }

    std::string epp_result_description() const {
        return epp_result_description_;
    }

    const boost::optional<std::set<EppExtendedErrorLocalized> >& extended_errors() const
    {
        return extended_errors_;
    }

private:

    EppResultFailure epp_result_;

    std::string epp_result_description_;

    boost::optional<std::set<EppExtendedErrorLocalized> > extended_errors_;

};

} // namespace Epp

#endif
