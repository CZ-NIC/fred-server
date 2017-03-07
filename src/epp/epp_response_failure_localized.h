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

#ifndef EPP_RESPONSE_FAILURE_LOCALIZED_H_2206E3966B8346719CBDBBFD6A2DC0EB
#define EPP_RESPONSE_FAILURE_LOCALIZED_H_2206E3966B8346719CBDBBFD6A2DC0EB

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_failure_localized.h"
#include "src/epp/session_data.h"
#include "src/fredlib/opcontext.h"

#include <vector>

namespace Epp {

class EppResponseFailureLocalized
{
public:

    EppResponseFailureLocalized(
            Fred::OperationContext& _ctx,
            const EppResponseFailure& _epp_response,
            Epp::SessionLang::Enum _session_lang);

    virtual ~EppResponseFailureLocalized() throw()
    { }

    const std::vector<EppResultFailureLocalized>& epp_results() const {
        return epp_results_;
    }

    const EppResultFailureLocalized& epp_result() const {
        return epp_results_.front();
    }

private:

    EppResponseFailure epp_response_;
    std::vector<EppResultFailureLocalized> epp_results_;
};

} // namespace Epp

#endif
