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
 *  enum object states related to admin contact verification
 *  There can be another testsuites as well but content of this enum is mandatory and should be always present in database.
 */

#ifndef CONTACT_VERIFICATION_RELATED_OBJECT_STATES_21547565320_
#define CONTACT_VERIFICATION_RELATED_OBJECT_STATES_21547565320_

#include <vector>
#include <string>
#include <boost/assign/list_of.hpp>

namespace Admin
{
namespace AdminContactVerificationObjectStates
{
    const std::string CONTACT_IN_MANUAL_VERIFICATION        = "contactInManualVerification";
    const std::string CONTACT_PASSED_MANUAL_VERIFICATION    = "contactPassedManualVerification";
    const std::string CONTACT_FAILED_MANUAL_VERIFICATION    = "contactFailedManualVerification";

    inline std::vector<std::string> get_all() {
        return boost::assign::list_of
            (CONTACT_IN_MANUAL_VERIFICATION)
            (CONTACT_PASSED_MANUAL_VERIFICATION)
            (CONTACT_FAILED_MANUAL_VERIFICATION);
    }

    inline std::vector<std::string> get_final() {
        return boost::assign::list_of
            (CONTACT_PASSED_MANUAL_VERIFICATION)
            (CONTACT_FAILED_MANUAL_VERIFICATION);
    }
}
}
#endif // #include guard end

