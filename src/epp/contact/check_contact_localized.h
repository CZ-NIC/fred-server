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

/**
 *  @file
 */

#ifndef CHECK_CONTACT_LOCALIZED_H_1AE05A8205724400A94CCB90F0E0060F
#define CHECK_CONTACT_LOCALIZED_H_1AE05A8205724400A94CCB90F0E0060F

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/contact/impl/contact_handle_registration_obstruction.h"

#include <boost/optional.hpp>

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Contact {

struct ContactHandleLocalizedRegistrationObstruction
{
    const ContactHandleRegistrationObstruction::Enum state;
    const std::string description;

    ContactHandleLocalizedRegistrationObstruction(
        const ContactHandleRegistrationObstruction::Enum _state,
        const std::string& _description
    ) :
       state(_state),
       description(_description)
    { }
};

struct CheckContactLocalizedResponse
{
    const LocalizedSuccessResponse ok_response;
    const std::map<std::string, boost::optional<ContactHandleLocalizedRegistrationObstruction> > contact_statuses;

    CheckContactLocalizedResponse(
        const LocalizedSuccessResponse& _ok_response,
        const std::map<std::string, boost::optional<ContactHandleLocalizedRegistrationObstruction> >& _contact_statuses
    ) :
        ok_response(_ok_response),
        contact_statuses(_contact_statuses)
    { }
};

CheckContactLocalizedResponse check_contact_localized(
        const std::set<std::string>& _contact_handles,
        unsigned long long _registrar_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle);

} // namespace Epp::Contact
} // namespace Epp

#endif
