/*
 * Copyright (C) 2018-2022  CZ.NIC, z. s. p. o.
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
#ifndef CONTACT_OUTPUT_HH_423C3E44464D4CB2AF91BA42D101450F
#define CONTACT_OUTPUT_HH_423C3E44464D4CB2AF91BA42D101450F

#include "libfred/registrable_object/contact/info_contact_data.hh"
#include "src/backend/epp/contact/info_contact.hh"
#include "src/backend/epp/contact/contact_data_share_policy_rules.hh"
#include "src/backend/epp/password.hh"
#include "libfred/object_state/get_object_states.hh"

#include <vector>

namespace Epp {
namespace Contact {

InfoContactOutputData get_info_contact_output(
        LibFred::OperationContext& _ctx,
        LibFred::InfoContactData _data,
        const Password& _authinfopw,
        const ContactDataSharePolicyRules& _contact_data_share_policy_rules,
        const SessionData& _session_data,
        const std::vector<LibFred::ObjectStateData>& _object_state_data);

InfoContactOutputData get_info_contact_output(
        const LibFred::InfoContactData& _data,
        const std::vector<LibFred::ObjectStateData>& _object_state_data,
        bool _include_authinfo);

} // namespace Epp::Contact
} // namespace Epp

#endif
