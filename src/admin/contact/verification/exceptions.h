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

#ifndef ADMIN_CONTACT_VERIFICATION_EXCEPTIONS_323255421029_
#define ADMIN_CONTACT_VERIFICATION_EXCEPTIONS_323255421029_

#include "src/fredlib/opexception.h"

namespace Admin
{
    struct ExceptionUnknownContactId : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown contact id";}
    };

    struct ExceptionUnknownTestsuiteHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown testsuite handle";}
    };

    struct ExceptionUnknownCheckHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown check handle";}
    };

    struct ExceptionUnknownCheckStatusHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown check status handle";}
    };

    struct ExceptionCheckNotUpdateable : virtual Fred::OperationException {
        const char* what() const throw() {return "check is not updateable - either already resolved or tests not yet finished";}
    };

    struct ExceptionUnknownTestHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown test handle";}
    };

    struct ExceptionUnknownCheckTestPair : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown check_handle, test_handle pair";}
    };

    struct ExceptionUnknownTestStatusHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown test status handle";}
    };

    struct ExceptionIncorrectTestsuite : virtual Fred::OperationException {
        const char* what() const throw() {return "incorrect testsuite";}
    };

    struct ExceptionIncorrectCheckStatus : virtual Fred::OperationException {
        const char* what() const throw() {return "incorrect check status";}
    };

    struct ExceptionIncorrectContactStatus : virtual Fred::OperationException {
        const char* what() const throw() {return "incorrect contact status";}
    };

    struct ExceptionDomainsAlreadyDeleted : virtual Fred::OperationException {
        const char* what() const throw() {return "domains already deleted";}
    };
}

#endif
