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
 *  contact verification tests interface
 */

#ifndef CONTACT_VERIFICATION_TEST_INTF_11637813419_
#define CONTACT_VERIFICATION_TEST_INTF_11637813419_

#include <string>

namespace Admin
{
    class ContactVerificationTest {
        public:
            /**
             * @return final status of the test
             */
            virtual std::string run(long _history_id) const = 0;
            virtual std::string get_name() const = 0;
            virtual ~ContactVerificationTest() {};
    };
}

#endif // #include guard end
