/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef POLL_ACKNOWLEDGEMENT_HH_B99FF2A5A2874E81AA71B1ED5AAC03FC
#define POLL_ACKNOWLEDGEMENT_HH_B99FF2A5A2874E81AA71B1ED5AAC03FC

#include "libfred/opcontext.hh"

namespace Epp {
namespace Poll {

struct PollAcknowledgementOutputData
{
    unsigned long long number_of_unseen_messages;
    unsigned long long oldest_unseen_message_id;
};

PollAcknowledgementOutputData poll_acknowledgement(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
