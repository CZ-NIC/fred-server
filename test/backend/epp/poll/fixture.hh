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

#ifndef FIXTURE_H_392BF773C44544998E917DB3040CB769
#define FIXTURE_H_392BF773C44544998E917DB3040CB769

#include "src/libfred/opcontext.hh"

namespace Test {

void mark_all_messages_as_seen(::LibFred::OperationContext& _ctx);

unsigned long long get_number_of_unseen_poll_messages(::LibFred::OperationContext& _ctx);

struct MessageDetail
{
    unsigned long long message_id;
    unsigned long long registrar_id;
};

MessageDetail get_message_ids(::LibFred::OperationContext& _ctx);

} // namespace Test

#endif
