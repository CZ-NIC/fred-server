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

#include "fredlib/contact/verification/create_check.h"

namespace Fred
{
    CreateContactCheck::CreateContactCheck(Pg::Integer _contact_history_id, Pg::Serial _testsuite_id, Pg::BigSerial _logd_request_id, Pg::Serial _status_id)
        : contact_history_id_(_contact_history_id),
          testsuite_id_(_testsuite_id),
          logd_request_id_(_logd_request_id),
          status_id_(_status_id)
    { }

    boost::posix_time::ptime CreateContactCheck::exec(OperationContext& _ctx, const std::string& _postgres_time_zone) {
        boost::posix_time::ptime timestamp;

        try {
            Database::Result insert_contact_check_res = _ctx.get_conn().exec_params(
                "INSERT INTO contact_check ("
                "   contact_history_id,"
                "   log_request_id,"
                "   enum_contact_testsuite_id,"
                "   enum_contact_check_status_id )"
                "VALUES ("
                "   $1::int,"
                "   $2::bigint,"
                "   $3::int,"
                "   $4::int )"
                "RETURNING create_time AT TIME ZONE 'UTC' AT TIME ZONE $5::text;",
                Database::query_param_list(contact_history_id_)(testsuite_id_)(logd_request_id_)(status_id_)(_postgres_time_zone));

            if (insert_contact_check_res.size() != 1) {
                BOOST_THROW_EXCEPTION(Fred::InternalError("contact_check creation failed"));
            }

            timestamp = boost::posix_time::time_from_string(std::string(insert_contact_check_res[0][0]));

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }

        return timestamp;
    }

    std::ostream& operator<<(std::ostream& os, const CreateContactCheck& i) {
        os << "#CreateContactCheck contact_history_id_: " << i.contact_history_id_
            << " testsuite_id_: " << i.testsuite_id_
            << " logd_request_id_: " << i.logd_request_id_
            << " status_id_: " << i.status_id_;

        return os;
    }

    std::string CreateContactCheck::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

} // namespace Fred
