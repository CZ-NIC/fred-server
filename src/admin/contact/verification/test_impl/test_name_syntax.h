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
 *  contact verification test for name (syntax only)
 */

#ifndef CONTACT_VERIFICATION_TEST_NAME_SYNTAX_124898987441_
#define CONTACT_VERIFICATION_TEST_NAME_SYNTAX_124898987441_

#include "admin/contact/verification/test_impl/test_interface.h"

namespace Admin
{
    class ContactVerificationTestNameSyntax: public ContactVerificationTest {

        public:
            virtual ContactVerificationTest::T_run_result run(long _history_id) const;
            virtual std::string get_name() const { return "name_syntax"; }
    };
}


#endif // #include guard end
