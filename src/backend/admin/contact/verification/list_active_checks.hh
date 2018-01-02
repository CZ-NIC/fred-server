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
 *  return checks ready to be processed and resolved by user
 */

#ifndef LIST_ACTIVE_CHECKS_HH_B5E8B3E8E6FD445D9E6DEB4EBDFEBACB
#define LIST_ACTIVE_CHECKS_HH_B5E8B3E8E6FD445D9E6DEB4EBDFEBACB

#include <libfred/admin_contact_verification.hh>

namespace Admin {

    /**
     * Lists checks awaiting resolution - e. g. certain statuses.
     *
     * @param _testsuite handle     return checks only with specified testsuite
     * @return information about checks
     */
    std::vector<LibFred::ListChecksItem> list_active_checks(const Optional<std::string>& _testsuite_handle);
}


#endif
