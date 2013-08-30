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
 *  @file
 *  update contact check
 */
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

#include "fredlib/contact/verification/update_check.h"


namespace Fred
{
    UpdateContactCheck::UpdateContactCheck(
        const std::string& _check_handle,
        const std::string& _status_name
    ) :
        check_handle_(_check_handle),
        status_name_(_status_name)
    { }

    UpdateContactCheck::UpdateContactCheck(
        const std::string&  _check_handle,
        const std::string&  _status_name,
        Optional<long long> _logd_request_id
    ) :
        check_handle_(_check_handle),
        status_name_(_status_name),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                Nullable<long long>( _logd_request_id.get_value() )
                :
                Nullable<long long>()
        )
    { }

    UpdateContactCheck& UpdateContactCheck::set_logd_request_id (long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }

    void UpdateContactCheck::exec (OperationContext& _ctx) {
        try {
            Database::Result update_contact_check_res = _ctx.get_conn().exec_params(
               "UPDATE contact_check SET ( "
               "    enum_contact_check_status_id,"
               "    logd_request_id"
               ") = ("
               "    (SELECT id FROM enum_contact_check_status WHERE name=$1::varchar),"
               "    $2::bigint"
               ")"
               "WHERE handle=$3::cont_chck_handle;",
               Database::query_param_list
                (status_name_)
                (logd_request_id_)
                (check_handle_)
            );

            if (update_contact_check_res.size() != 1) {
               BOOST_THROW_EXCEPTION(Fred::InternalError("contact_check update failed"));
            }
        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }
    }

    std::ostream& operator<<(std::ostream& os, const UpdateContactCheck& i) {
        os << "#UpdateContactCheck "
            << " handle_: "          << i.check_handle_
            << " logd_request_id_: " << i.logd_request_id_.print_quoted()
            << " status_name_: "     << i.status_name_;

        return os;
    }

    std::string UpdateContactCheck::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
} // namespace Fred
