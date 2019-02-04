/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  admin contact verification exceptions
 */

#ifndef EXCEPTIONS_HH_22A4F0F06AE44C44936D609957771BA9
#define EXCEPTIONS_HH_22A4F0F06AE44C44936D609957771BA9

#include "libfred/opexception.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

    struct ExceptionUnknownContactId : virtual LibFred::OperationException {
        const char* what() const noexcept {return "unknown contact id";}
    };

    struct ExceptionUnknownTestsuiteHandle : virtual LibFred::OperationException {
        const char* what() const noexcept {return "unknown testsuite handle";}
    };

    struct ExceptionUnknownCheckHandle : virtual LibFred::OperationException {
        const char* what() const noexcept {return "unknown check handle";}
    };

    struct ExceptionUnknownCheckStatusHandle : virtual LibFred::OperationException {
        const char* what() const noexcept {return "unknown check status handle";}
    };

    struct ExceptionCheckNotUpdateable : virtual LibFred::OperationException {
        const char* what() const noexcept {return "check is not updateable - either already resolved or tests not yet finished";}
    };

    struct ExceptionUnknownTestHandle : virtual LibFred::OperationException {
        const char* what() const noexcept {return "unknown test handle";}
    };

    struct ExceptionUnknownCheckTestPair : virtual LibFred::OperationException {
        const char* what() const noexcept {return "unknown check_handle, test_handle pair";}
    };

    struct ExceptionUnknownTestStatusHandle : virtual LibFred::OperationException {
        const char* what() const noexcept {return "unknown test status handle";}
    };

    struct ExceptionIncorrectTestsuite : virtual LibFred::OperationException {
        const char* what() const noexcept {return "incorrect testsuite";}
    };

    struct ExceptionIncorrectCheckStatus : virtual LibFred::OperationException {
        const char* what() const noexcept {return "incorrect check status";}
    };

    struct ExceptionIncorrectContactStatus : virtual LibFred::OperationException {
        const char* what() const noexcept {return "incorrect contact status";}
    };

    struct ExceptionDomainsAlreadyDeleted : virtual LibFred::OperationException {
        const char* what() const noexcept {return "domains already deleted";}
    };

    struct ExceptionTestImplementationError : virtual LibFred::OperationException {
        const char* what() const noexcept {return "error in tests implementation";}
    };

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
