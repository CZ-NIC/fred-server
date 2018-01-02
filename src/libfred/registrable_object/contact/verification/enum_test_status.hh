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

#ifndef ENUM_TEST_STATUS_HH_A1CB8D8900624E6BB006A754EBE98CD0
#define ENUM_TEST_STATUS_HH_A1CB8D8900624E6BB006A754EBE98CD0

namespace LibFred
{
    /**
     * Available statuses for Contact test.
     * Should be in sync with enum_contact_test_status.handle in db.
     */
    namespace ContactTestStatus
    {
        const std::string ENQUEUED      = "enqueued";
        const std::string RUNNING       = "running";
        const std::string SKIPPED       = "skipped";
        const std::string ERROR         = "error";
        const std::string MANUAL        = "manual";
        const std::string OK            = "ok";
        const std::string FAIL          = "fail";
    }
}
#endif // #include guard end

