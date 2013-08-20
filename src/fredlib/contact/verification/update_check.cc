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
 *  @file update_check.cc
 *  update contact check
 */

#include "fredlib/contact/verification/update_check.h"
#include <boost/lexical_cast.hpp>

namespace Fred
{
    UpdateContactCheck::UpdateContactCheck(
        const std::string& _handle,
        const std::string& _status_name
    ) :
        handle_(_handle),
        status_name_(_status_name)
    { }

    UpdateContactCheck::UpdateContactCheck(
        const std::string&              _handle,
        const std::string&              _status_name,
        Optional<long long> _logd_request_id
    ) :
        handle_(_handle),
        status_name_(_status_name),
        logd_request_id_(_logd_request_id)
    { }

    UpdateContactCheck& UpdateContactCheck::set_logd_request_id (long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }
    UpdateContactCheck& UpdateContactCheck::unset_logd_request_id () {
        logd_request_id_ = Optional<long long>();
        return *this;
    }

    void UpdateContactCheck::exec (OperationContext& _ctx) {

        std::vector<std::string> columns;
        std::vector<std::string> values;
        Database::query_param_list params;

        // setting the first mandatory parameter - handle for WHERE condition
        //   - doing it here so I don't have to think which $number it is
        params(handle_);

        columns.push_back("enum_contact_check_status_id");
        values.push_back("(SELECT id FROM enum_contact_check_status WHERE name=$2::varchar)");
        params(status_name_);

        // optional values
        columns.push_back("logd_request_id");
        values.push_back("$3::bigint)");
        if( logd_request_id_.isset() ) {
            params(logd_request_id_.get_value());
        } else {
            params(Database::NullQueryParam);
        }

        try {
            Database::Result update_contact_check_res = _ctx.get_conn().exec_params(
               "UPDATE contact_check SET ( "
                   + boost::algorithm::join( columns, ", ") +
               ") = ("
                   + boost::algorithm::join( values, ", ") +
               ")"
               "WHERE handle=$1::cont_chck_handle;",
               params);

            if (update_contact_check_res.size() != 1) {
               BOOST_THROW_EXCEPTION(Fred::InternalError("contact_check update failed"));
            }
        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }
    }

    std::ostream& operator<<(std::ostream& os, const UpdateContactCheck& i) {
        os << "#UpdateContactCheck handle_: " << i.handle_
            << " logd_request_id_: " << i.logd_request_id_.print_quoted()
            << " status_name_: " << i.status_name_;

        return os;
    }

    std::string UpdateContactCheck::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
} // namespace Fred
