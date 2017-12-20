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

#ifndef EXCEPTION_H_B43EF188F27B433A95725369EF9D1FAB
#define EXCEPTION_H_B43EF188F27B433A95725369EF9D1FAB

#include <exception>

namespace Epp {

    struct Exception : std::exception { };

    struct InvalidSessionLang            : Exception { const char* what() const noexcept { return "invalid session language"; } };
    struct InvalidEppResultCodeValue     : Exception { const char* what() const noexcept { return "invalid EppResultCode::(Success|Failure) value"; } };
    struct InvalidReasonValue            : Exception { const char* what() const noexcept { return "invalid Reason::Enum value"; } };

    struct BillingFailure                : Exception { const char* what() const noexcept { return "Billing failure"; } };

    // localized descriptions
    struct LocalizedDescriptionException : Exception { };
    struct UnknownLocalizationLanguage   : LocalizedDescriptionException { const char* what() const noexcept { return "unknown localization language"; } };
    struct MissingLocalizedDescription   : LocalizedDescriptionException { const char* what() const noexcept { return "missing localized description"; } };
    struct UnknownLocalizedDescriptionId : LocalizedDescriptionException { const char* what() const noexcept { return "unknown localized description id"; } };

}

#endif
