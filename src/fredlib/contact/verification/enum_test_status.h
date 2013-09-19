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
 *  enum test status
 */

#ifndef CONTACT_VERIFICATION_TEST_STATUS_35891112386_
#define CONTACT_VERIFICATION_TEST_STATUS_35891112386_

namespace Fred
{
    /**
     * Available statuses for Contact test.
     * Should be in sync with enum_contact_test_status.name in db.
     */
    namespace ContactTestStatus
    {
        const std::string ENQUEUED      = "enqueued";
        const std::string RUNNING       = "running";
        const std::string MANUAL        = "manual";
        const std::string OK            = "ok";
        const std::string FAIL          = "fail";
    }
}
#endif // #include guard end

