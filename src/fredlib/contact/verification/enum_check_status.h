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
 *  enum check status
 */

#ifndef CONTACT_VERIFICATION_CHECK_STATUS_35892232386_
#define CONTACT_VERIFICATION_CHECK_STATUS_35892232386_

#include <boost/assign/list_of.hpp>

namespace Fred
{
    /**
     * Available statuses for Contact check.
     * Should be in sync with enum_contact_check_status.handle in db.
     */
    namespace ContactCheckStatus
    {
        const std::string ENQUEUE_REQ           = "enqueue_req";
        const std::string ENQUEUED              = "enqueued";
        const std::string RUNNING               = "running";
        const std::string AUTO_TO_BE_DECIDED    = "auto_to_be_decided";
        const std::string AUTO_OK               = "auto_ok";
        const std::string AUTO_FAIL             = "auto_fail";
        const std::string OK                    = "ok";
        const std::string FAIL_REQ              = "fail_req";
        const std::string FAIL                  = "fail";
        const std::string INVALIDATED           = "invalidated";

        /**
         * Check statuses sorted into semantical groups
         */

        inline std::vector<std::string> get_resolution_awaiting() {
            return boost::assign::list_of
                (AUTO_OK)
                (AUTO_FAIL)
                (AUTO_TO_BE_DECIDED)
                (FAIL_REQ);
        }

        inline std::vector<std::string> get_not_yet_resolved() {
            return boost::assign::list_of
                (ENQUEUE_REQ)
                (ENQUEUED)
                (RUNNING)
                (AUTO_OK)
                (AUTO_FAIL)
                (AUTO_TO_BE_DECIDED)
                (FAIL_REQ);
        }

        inline std::vector<std::string> get_possible_resolutions() {
            return boost::assign::list_of
                (OK)
                (FAIL_REQ)
                (FAIL)
                (INVALIDATED);
        }

        inline std::vector<std::string> get_all() {
            return boost::assign::list_of
                (ENQUEUE_REQ)
                (ENQUEUED)
                (RUNNING)
                (AUTO_OK)
                (AUTO_FAIL)
                (AUTO_TO_BE_DECIDED)
                (OK)
                (FAIL_REQ)
                (FAIL)
                (INVALIDATED);
        }
    }
}
#endif // #include guard end

