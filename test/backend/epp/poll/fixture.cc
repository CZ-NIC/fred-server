/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/epp/poll/fixture.hh"

#include <boost/test/unit_test.hpp>

namespace Test {

void mark_all_messages_as_seen(::LibFred::OperationContext& _ctx)
{
    _ctx.get_conn().exec("UPDATE message SET seen=true");
}

unsigned long long get_number_of_unseen_poll_messages(::LibFred::OperationContext& _ctx)
{
    const Database::Result sql_query_result = _ctx.get_conn().exec("SELECT COUNT(id) FROM message WHERE NOT seen");
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    return static_cast<unsigned long long>(sql_query_result[0][0]);
}

MessageDetail get_message_ids(::LibFred::OperationContext& _ctx)
{
    MessageDetail ret;

    const Database::Result sql_query_result =
        _ctx.get_conn().exec("SELECT id, clid FROM message WHERE NOT seen ORDER BY id DESC LIMIT 1");
    BOOST_REQUIRE_EQUAL(sql_query_result.size(), 1);
    ret.message_id = static_cast<unsigned long long>(sql_query_result[0][0]);
    ret.registrar_id = static_cast<unsigned long long>(sql_query_result[0][1]);

    return ret;
}

} // namespace Test
