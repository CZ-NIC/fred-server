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

#ifndef EPP_RESULT_SUCCESS_LOCALIZED_H_2B0481DFEC544E119EC86D0C318C9920
#define EPP_RESULT_SUCCESS_LOCALIZED_H_2B0481DFEC544E119EC86D0C318C9920

#include "src/epp/impl/epp_result_success.h"
#include "src/epp/impl/session_lang.h"

#include "src/fredlib/opcontext.h"

#include <boost/optional.hpp>

#include <set>

namespace Epp {

class EppResultSuccessLocalized
{

public:

    EppResultSuccessLocalized(
            Fred::OperationContext& _ctx,
            const EppResultSuccess& _epp_result,
            const SessionLang::Enum& _session_lang);

    virtual const char* c_str() const throw() {
        return epp_result_description_.c_str();
    }

    EppResultCode::Success epp_result_code() const {
        return epp_result_.epp_result_code();
    }

    std::string epp_result_description() const {
        return epp_result_description_;
    }

private:

    EppResultSuccess epp_result_;

    std::string epp_result_description_;

};

} // namespace Epp

#endif
