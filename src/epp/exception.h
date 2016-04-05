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

namespace Epp {

    struct Exception {
        virtual const char* what() const = 0;
        virtual ~Exception() {}
    };

    struct AuthErrorServerClosingConnection : Exception {   const char* what() const { return "authorization error: server is closing connection"; }    };
    struct AutorError                       : Exception {   const char* what() const { return "autor error"; }  }; /* TODO some better name, what() */
    struct InvalidHandle                    : Exception {   const char* what() const { return "invalid handle"; }   };
    struct InvalidSessionLang               : Exception {   const char* what() const { return "invalid session language";   }   };
    struct NonexistentHandle                : Exception {   const char* what() const { return "nonexistent handle"; }   };
    struct ObjectExists                     : Exception {   const char* what() const { return "object exists"; }    };
    struct ObjectNotEligibleForTransfer     : Exception {   const char* what() const { return "object not eligible for transfer";   }   };
    struct ObjectStatusProhibitingOperation : Exception {   const char* what() const { return "object status prohibiting operation";    }   };
    struct SsnTypeWithoutSsn                : Exception {   const char* what() const { return "ssntype without ssn"; }   };
    struct SsnWithoutSsnType                : Exception {   const char* what() const { return "ssn without ssntype"; }   };

    struct InvalidIdentTypeDbHandle         : Exception {   const char* what() const { return "invalid IdentType db handle";   }   };

    /* localized descriptions */
    struct LocalizedDescriptionException : Exception {
        virtual const char* what() const = 0;
    };

    struct UnknownLocalizationLanguage      : LocalizedDescriptionException {  const char* what() const { return "unknown localization language"; }   };
    struct MissingLocalizedDescription      : LocalizedDescriptionException {  const char* what() const { return "missing localized description"; }   };
    struct UnknownLocalizedDescriptionId    : LocalizedDescriptionException {  const char* what() const { return "unknown localized description id"; }   };
}

#endif
