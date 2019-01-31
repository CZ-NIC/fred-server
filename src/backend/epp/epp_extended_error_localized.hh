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

#ifndef EPP_EXTENDED_ERROR_LOCALIZED_HH_6C4FA59C8B8346CC9ECC6DFB7658F149
#define EPP_EXTENDED_ERROR_LOCALIZED_HH_6C4FA59C8B8346CC9ECC6DFB7658F149

#include "src/backend/epp/epp_extended_error.hh"
#include "src/backend/epp/session_data.hh"

#include "src/backend/epp/reason.hh"
#include "src/backend/epp/param.hh"

#include "libfred/opcontext.hh"

#include <string>

namespace Epp {

class EppExtendedErrorLocalized
{

public:

    EppExtendedErrorLocalized(
            LibFred::OperationContext& _ctx,
            const EppExtendedError& _epp_extended_error,
            const SessionLang::Enum _lang);

    Param::Enum param() const {
        return epp_extended_error_.param();
    }

    unsigned short position() const {
        return epp_extended_error_.position();
    }

    std::string reason_description() const {
        return reason_description_;
    }

    // only intended for std::set usage - ordering definition is irrelevant
    friend bool operator < (const Epp::EppExtendedErrorLocalized& lhs, const Epp::EppExtendedErrorLocalized& rhs);

private:

    EppExtendedError epp_extended_error_;

    std::string reason_description_;

};

} // namespace Epp

#endif
