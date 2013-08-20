/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file create_check.cc
 *  create contact check
 */

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/join.hpp>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/enum_check_status.h"

namespace Fred
{
    CreateContactCheck::CreateContactCheck(
        const std::string& _contact_handle,
        const std::string& _testsuite_name,
    ) :
        contact_handle_(_contact_handle),
        testsuite_name_(_testsuite_name)
    { }

    CreateContactCheck::CreateContactCheck(
        const std::string& _contact_handle,
        const std::string& _testsuite_name,
        Optional<long long> _logd_request_id
    ) :
        contact_handle_(_contact_handle),
        testsuite_name_(_testsuite_name),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                (_logd_request_id.get_value() )
                :
                Nullable<long long>()
        )
    { }

    CreateContactCheck& CreateContactCheck::set_logd_request_id(long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }

    CreateContactCheck& CreateContactCheck::unset_logd_request_id() {
        logd_request_id_ = Nullable<long long>();
        return *this;
    }

    std::string CreateContactCheck::exec(OperationContext& _ctx) {
        std::string handle;
        std::vector<std::string> values;
        std::vector<std::string> columns;
        Database::query_param_list params;

        columns.push_back("contact_history_id");
        values.push_back
                    ("(SELECT o_h.historyid"
                     "    FROM object_registry AS o_r"
                     "        LEFT JOIN object_history AS o_h USING(id)"
                     "        LEFT JOIN h AS ON o_h.historyid = h.id"
                     "    WHERE o_r.name=$1::varchar"
                     "        AND h.next IS NULL)");
        params(contact_handle_);

        columns.push_back("enum_contact_testsuite_id");
        values.push_back("(SELECT id FROM enum_contact_testsuite WHERE name=$2::varchar)");
        params(testsuite_name_);

        columns.push_back("enum_contact_check_status_id");
        values.push_back("(SELECT id FROM enum_contact_check_status WHERE name=$3::varchar)");
        params(Fred::ContactCheckStatus::ENQUEUED);

        columns.push_back("logd_request_id");
        values.push_back("$4::bigint");
        params(logd_request_id_);

        try {
            Database::Result insert_contact_check_res = _ctx.get_conn().exec_params(
                "INSERT INTO contact_check ( "
                    + boost::algorithm::join( columns, ", ") +
                ")"
                "VALUES ("
                    + boost::algorithm::join( values, ", ") +
                ")"
                "RETURNING handle;",
                params);

            if (insert_contact_check_res.size() != 1) {
                BOOST_THROW_EXCEPTION(Fred::InternalError("contact_check creation failed"));
            }

            handle = *(insert_contact_check_res.begin())["handle"];

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }

        return handle;
    }

    std::ostream& operator<<(std::ostream& os, const CreateContactCheck& i) {
        os << "#CreateContactCheck contact_handle_: " << i.contact_handle_
            << " testsuite_name_: " << i.testsuite_name_
            << " logd_request_id_: " << i.logd_request_id_.print_quoted();

        return os;
    }

    std::string CreateContactCheck::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

} // namespace Fred
