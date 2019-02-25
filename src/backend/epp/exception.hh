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
#ifndef EXCEPTION_HH_1DAB5A61E496418EB7F05E608086AB70
#define EXCEPTION_HH_1DAB5A61E496418EB7F05E608086AB70

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
