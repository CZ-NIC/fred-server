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

#include "src/libfred/poll/create_low_credit_messages.hh"
#include "src/libfred/opexception.hh"
#include "src/util/db/query_param.hh"

namespace LibFred {
namespace Poll {

unsigned long long CreateLowCreditMessages::exec(OperationContext& _ctx) const
{
    const Database::Result sql_query_result = _ctx.get_conn().exec(
        "WITH "
            "tmp_message AS "
            "("
                "SELECT m.clid, pc.zone, MAX(m.crdate) AS crdate "
                "FROM message m JOIN poll_credit pc ON m.id=pc.msgid "
                "GROUP BY m.clid, pc.zone "
            "), "
            "tmp_invoice AS "
            "("
                "SELECT i.registrar_id, i.zone_id, MAX(i.crdate) AS crdate "
                "FROM invoice i "
                "JOIN invoice_prefix ip ON i.invoice_prefix_id = ip.id AND ip.typ=0 "
                "GROUP BY i.registrar_id, i.zone_id "
            "), "
            "tmp_poll_credit AS "
            "("
                "SELECT nextval('message_id_seq') AS msgid, "
                       "rc.zone_id AS zoneid, "
                       "rc.registrar_id AS reg, "
                       "rc.credit AS credititself, "
                       "l.credlimit AS creditlimit "
                "FROM registrar_credit rc "
                "JOIN poll_credit_zone_limit l ON rc.zone_id = l.zone "
                "LEFT JOIN tmp_message ON tmp_message.clid=rc.registrar_id AND tmp_message.zone=rc.zone_id "
                "LEFT JOIN tmp_invoice ON tmp_invoice.registrar_id=rc.registrar_id AND tmp_invoice.zone_id=rc.zone_id "
                "WHERE rc.credit < l.credlimit AND (tmp_message.crdate IS NULL OR tmp_message.crdate < tmp_invoice.crdate) "
            "), "
            "we_dont_care_because_we_dont_actually_use_the_name AS "
            "("
                "INSERT INTO message (id, clid, crdate, exdate, seen, msgtype) "
                "SELECT msgid, reg, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP + INTERVAL '7days', false, 1 "
                "FROM tmp_poll_credit "
            ")"
        "INSERT INTO poll_credit (msgid, zone, credlimit, credit) "
        "SELECT msgid, zoneid, creditlimit, credititself "
        "FROM tmp_poll_credit");

    return sql_query_result.rows_affected();
}

} // namespace LibFred::Poll
} // namespace LibFred

