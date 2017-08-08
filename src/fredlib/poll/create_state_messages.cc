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

#include "src/fredlib/poll/create_state_messages.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/poll/message_type.h"

#include "boost/algorithm/string/join.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

namespace Fred {
namespace Poll {

CreateStateMessages::CreateStateMessages(const std::string& _except_list, int _limit) : limit_(_limit)
{
    boost::split(except_list_, _except_list, boost::is_any_of(","));
    for (std::vector<std::string>::const_iterator itr = except_list_.begin();
        itr != except_list_.end();
        ++itr)
    {
        if (!itr->empty())
        {
            Conversion::Enums::from_db_handle<Fred::Poll::MessageType>(*itr);
        }
    }
}

unsigned long long CreateStateMessages::exec(OperationContext& _ctx) const
{
    const Database::Result sql_query_result = _ctx.get_conn().exec_params(
             "WITH "
                  "pfilter(stateidname, registrytypename, msgtypename) AS "
                  "( "
                      "VALUES  "
                      "('expirationWarning', NULL, 'imp_expiration'), "
                      "('expired', NULL, 'expiration'), "
                      "('validationWarning1', NULL, 'imp_validation'), "
                      "('notValidated', NULL, 'validation'), "
                      "('outzoneUnguarded', NULL, 'outzone'), "
                      "('deleteCandidate', 'contact', 'idle_delete_contact'), "
                      "('deleteCandidate', 'nsset', 'idle_delete_nsset'), "
                      "('deleteCandidate', 'domain', 'idle_delete_domain'), "
                      "('deleteCandidate', 'keyset', 'idle_delete_keyset') "
                  "), "
                  "sfilter AS "
                  "( "
                      "SELECT stateidname, registrytypename, msgtypename "
                      "FROM pfilter "
                      "WHERE msgtypename != ALL($1::varchar[]) "
                  "), "
                  "tfilter AS "
                  "( "
                      "SELECT eos.id AS stateid, eot.id AS registrytype, mt.id AS msgtype "
                      "FROM sfilter f "
                      "JOIN enum_object_states eos ON eos.name=f.stateidname "
                      "LEFT JOIN enum_object_type eot ON eot.name=f.registrytypename "
                      "JOIN messagetype mt ON mt.name=f.msgtypename "
                  "), "
                  "tmp_table AS "
                  "( "
                      "SELECT nextval('message_id_seq') AS id, oh.clid AS reg, f.msgtype AS msgtype, os.id AS stateid "
                      "FROM object_registry ob "
                      "JOIN object_state os ON os.object_id=ob.id "
                      "JOIN object_history oh ON oh.historyid=os.ohid_from "
                      "JOIN tfilter f ON f.stateid=os.state_id AND (f.registrytype IS NULL OR f.registrytype=ob.type) "
                      "LEFT JOIN poll_statechange ps ON ps.stateid=os.id  "
                      "WHERE os.valid_to IS NULL AND ps.stateid IS NULL "
                      "ORDER BY os.id "
                      "LIMIT $2::int"
                  "), "
                  "we_dont_care AS "
                  "( "
                      "INSERT INTO message (id, clid, crdate, exdate, seen, msgtype) "
                      "SELECT id, reg, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP + INTERVAL '7days', false, msgtype "
                      "FROM tmp_table "
                      "ORDER BY stateid "
                  ") "
             "INSERT INTO poll_statechange (msgid, stateid) "
             "SELECT id, stateid "
             "FROM tmp_table "
             "ORDER BY stateid",
             Database::query_param_list
             (std::string("{") + boost::join(except_list_, ",") + std::string("}"))
             (limit_ > 0 ? limit_ : Database::QPNull));

    return sql_query_result.rows_affected();
}

} // namespace Fred::Poll
} // namespace Fred
