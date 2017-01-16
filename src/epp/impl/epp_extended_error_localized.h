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

#ifndef EPP_EXTENDED_ERROR_LOCALIZED_H_537A3FC8B8834EB3B7C1EECC479E746E
#define EPP_EXTENDED_ERROR_LOCALIZED_H_537A3FC8B8834EB3B7C1EECC479E746E

#include "src/epp/impl/epp_extended_error.h"
#include "src/epp/impl/session_data.h"

#include "src/epp/impl/reason.h"
#include "src/epp/impl/param.h"

#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {

class EppExtendedErrorLocalized
{

public:

    EppExtendedErrorLocalized(
            Fred::OperationContext& _ctx,
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

    const EppExtendedError& epp_extended_error_;

    std::string reason_description_;

};

} // namespace Epp

#endif
