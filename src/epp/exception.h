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

/**
 *  @file
 */

#ifndef EPP_EXCEPTION_H_1397645381064
#define EPP_EXCEPTION_H_1397645381064

#include <exception>

namespace Epp {

    struct Exception:std::exception { };

    struct AuthErrorServerClosingConnection   :Exception { const char* what() const throw() { return "authentication error: server is closing connection"; } };
    struct AuthorizationError                 :Exception { const char* what() const throw() { return "authorization error"; } };
    struct AuthorizationInformationError      :Exception { const char* what() const throw() { return "authorization information error"; } };
    struct InvalidSessionLang                 :Exception { const char* what() const throw() { return "invalid session language"; } };
    struct NonexistentHandle                  :Exception { const char* what() const throw() { return "nonexistent handle"; } };
    struct InvalidHandle                      :Exception { const char* what() const throw() { return "invalid handle"; } };
    struct ObjectExists                       :Exception { const char* what() const throw() { return "object exists"; } };
    struct ObjectDoesNotExist                 :Exception { const char* what() const throw() { return "object does not exist"; } };
    struct ObjectNotEligibleForTransfer       :Exception { const char* what() const throw() { return "object not eligible for transfer"; } };
    struct ObjectStatusProhibitsOperation     :Exception { const char* what() const throw() { return "object status prohibits operation"; } };
    struct ObjectAssociationProhibitsOperation:Exception { const char* what() const throw() { return "object association prohibits operation"; } };
    struct SsnTypeWithoutSsn                  :Exception { const char* what() const throw() { return "ssntype without ssn"; } };
    struct SsnWithoutSsnType                  :Exception { const char* what() const throw() { return "ssn without ssntype"; } };

    struct InvalidIdentTypeDbHandle           :Exception { const char* what() const throw() { return "invalid IdentType db handle"; } };
    struct InvalidResponseValue               :Exception { const char* what() const throw() { return "invalid Response::Enum value"; } };
    struct InvalidReasonValue                 :Exception { const char* what() const throw() { return "invalid Reason::Enum value"; } };

    struct RequiredParameterMissing : Exception { const char* what() const throw() { return "Required parameter missing"; } };
    struct BillingFailure : Exception { const char* what() const throw() { return "Billing failure"; } };



    /* localized descriptions */
    struct LocalizedDescriptionException:Exception { };

    struct UnknownLocalizationLanguage        :LocalizedDescriptionException { const char* what() const throw() { return "unknown localization language"; } };
    struct MissingLocalizedDescription        :LocalizedDescriptionException { const char* what() const throw() { return "missing localized description"; } };
    struct UnknownLocalizedDescriptionId      :LocalizedDescriptionException { const char* what() const throw() { return "unknown localized description id"; } };

}

#endif
