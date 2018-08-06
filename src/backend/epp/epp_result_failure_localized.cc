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

#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/epp/epp_result_failure_localized.hh"

#include "src/backend/epp/epp_extended_error.hh"
#include "src/backend/epp/epp_extended_error_localized.hh"
#include "src/backend/epp/localization.hh"

#include "src/libfred/opcontext.hh"

#include <boost/optional.hpp>

#include <set>

namespace Epp {

EppResultFailureLocalized::EppResultFailureLocalized(
        LibFred::OperationContext& _ctx,
        const EppResultFailure& _epp_result,
        const SessionLang::Enum _session_lang)
    : epp_result_(_epp_result)
{
    epp_result_description_ = get_epp_result_description_localized<EppResultCode::Failure>(_ctx, epp_result_.epp_result_code(), _session_lang);

    const boost::optional<std::set<EppExtendedError> >& extended_errors = _epp_result.extended_errors();
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

const char* EppResultFailureLocalized::c_str()const noexcept
{
    return epp_result_description_.c_str();
}

EppResultCode::Failure EppResultFailureLocalized::epp_result_code()const
{
    return epp_result_.epp_result_code();
}

std::string EppResultFailureLocalized::epp_result_description()const
{
    return epp_result_description_;
}

const boost::optional<std::set<EppExtendedErrorLocalized>>& EppResultFailureLocalized::extended_errors()const
{
    return extended_errors_;
}

}//namespace Epp
