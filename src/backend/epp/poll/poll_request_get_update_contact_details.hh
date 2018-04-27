/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef POLL_REQUEST_GET_UPDATE_CONTACT_DETAILS_HH_4783DA914F9A425D98BA1FEAD4F7787B
#define POLL_REQUEST_GET_UPDATE_CONTACT_DETAILS_HH_4783DA914F9A425D98BA1FEAD4F7787B

#include "src/libfred/opcontext.hh"

#include "src/backend/epp/contact/info_contact.hh"

namespace Epp {
namespace Poll {

struct PollRequestUpdateContactOutputData
{
    Epp::Contact::InfoContactOutputData old_data;
    Epp::Contact::InfoContactOutputData new_data;
};

PollRequestUpdateContactOutputData poll_request_get_update_contact_details(
        LibFred::OperationContext& _ctx,
        unsigned long long _message_id,
        unsigned long long _registrar_id);


} // namespace Epp::Poll
} // namespace Epp

#endif
