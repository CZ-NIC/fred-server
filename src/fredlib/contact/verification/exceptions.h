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
 *  contact verification exceptions
 */

#ifndef CONTACT_VERIFICATION_EXCEPTIONS_5454654549888_
#define CONTACT_VERIFICATION_EXCEPTIONS_5454654549888_

#include "src/fredlib/opexception.h"

namespace Fred
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

    struct ExceptionUnknownTestHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown test handle";}
    };
    struct ExceptionTestNotInMyTestsuite : virtual Fred::OperationException {
        const char* what() const throw() {return "test is not in testsuite of this check";}
    };
    struct ExceptionCheckTestPairAlreadyExists : virtual Fred::OperationException {
        const char* what() const throw() {return "given check test pair already exists";}
    };

    struct ExceptionUnknownCheckStatusHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown check status handle";}
    };

    struct ExceptionUnknownCheckTestPair : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown check_handle, test_handle pair";}
    };
    struct ExceptionUnknownTestStatusHandle : virtual Fred::OperationException {
        const char* what() const throw() {return "unknown test status handle";}
    };
}

#endif
