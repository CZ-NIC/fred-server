/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef CHECK_CONTACT_LOCALIZED_H_1AE05A8205724400A94CCB90F0E0060F
#define CHECK_CONTACT_LOCALIZED_H_1AE05A8205724400A94CCB90F0E0060F

#include "src/backend/epp/contact/check_contact_config_data.hh"
#include "src/backend/epp/contact/check_contact_localized_response.hh"
#include "src/backend/epp/session_data.hh"

#include <set>
#include <string>

namespace Epp {
namespace Contact {

CheckContactLocalizedResponse check_contact_localized(
        const std::set<std::string>& _contact_handles,
        const CheckContactConfigData& _check_contact_config_data,
        const SessionData& _session_data);


} // namespace Epp::Contact
} // namespace Epp

#endif
