/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  enum object states related to admin contact verification
 *  There can be another testsuites as well but content of this enum is mandatory and should be always present in database.
 */

#ifndef ENUM_HH_5A2AA128050749E194A29DEF54A4DF92
#define ENUM_HH_5A2AA128050749E194A29DEF54A4DF92

#include <boost/assign/list_of.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {
namespace ContactStates {

const std::string CONTACT_IN_MANUAL_VERIFICATION        = "contactInManualVerification";
const std::string CONTACT_PASSED_MANUAL_VERIFICATION    = "contactPassedManualVerification";
const std::string CONTACT_FAILED_MANUAL_VERIFICATION    = "contactFailedManualVerification";

inline std::vector<std::string> get_all()
{
    return boost::assign::list_of(CONTACT_IN_MANUAL_VERIFICATION)(CONTACT_PASSED_MANUAL_VERIFICATION)(
            CONTACT_FAILED_MANUAL_VERIFICATION);
}


inline std::vector<std::string> get_final()
{
    return boost::assign::list_of(CONTACT_PASSED_MANUAL_VERIFICATION)(CONTACT_FAILED_MANUAL_VERIFICATION);
}


} // namespace Fred::Backend::Admin::Contact::Verification::ObjectStates
} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif // #include guard end
