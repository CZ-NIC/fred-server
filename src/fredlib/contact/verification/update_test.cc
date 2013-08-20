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
 *  update contact test
 */

#include "fredlib/contact/verification/update_test.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

namespace Fred
{
   UpdateContactTest::UpdateContactTest(
        const std::string& _check_handle,
        const std::string& _test_name,
        const std::string& _status_name
    ) :
        check_handle_(_check_handle),
        test_name_(_test_name),
        status_name_(_status_name)
    { }

    UpdateContactTest::UpdateContactTest(
        const std::string&    _check_handle,
        const std::string&    _test_name,
        const std::string&    _status_name,
        Optional<long long>   _logd_request_id,
        Optional<std::string> _error_msg
    ) :
        check_handle_(_check_handle),
        test_name_(_test_name),
        status_name_(_status_name),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                (_logd_request_id.get_value() )
                :
                Nullable<long long>()
        ),
        error_msg_(
            ( _error_msg.isset() )
                ?
                (_error_msg.get_value() )
                :
                Nullable<long long>()
        )
    { }

    UpdateContactTest& UpdateContactTest::set_logd_request_id (long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }
    UpdateContactTest& UpdateContactTest::unset_logd_request_id () {
        logd_request_id_ = Nullable<long long>();
        return *this;
    }

    UpdateContactTest& UpdateContactTest::set_error_msg (const std::string& _error_msg) {
        error_msg_ = _error_msg;
        return *this;
    }
    UpdateContactTest& UpdateContactTest::unset_error_msg () {
        error_msg_ = Nullable<std::string>();
        return *this;
    }

    void UpdateContactTest::exec (OperationContext& _ctx) {

        std::vector<std::string> columns = boost::assign::list_of
            ("enum_contact_test_status_id");

        std::vector<std::string> values = boost::assign::list_of
            ("(SELECT id FROM enum_contact_test_status WHERE name=$1::varchar)");

        Database::query_param_list params(status_name_);

        // optional values
        columns.push_back("logd_request_id");
        values.push_back("$2::bigint)");
        if( logd_request_id_.isnull() == false ) {
            params(logd_request_id_ );
        } else {
            params(Database::NullQueryParam);
        }

        columns.push_back("error_msg");
        values.push_back("$3::bigint)");
        params(error_msg_);

        try {
            Database::Result update_contact_check_res = _ctx.get_conn().exec_params(
               "UPDATE contact_test_result SET ( "
                   + boost::algorithm::join( columns, ", ") +
               ") = ("
                   + boost::algorithm::join( values, ", ") +
               ")"
               "WHERE contact_check_id="
               "    (SELECT id FROM contact_check WHERE handle=$4::cont_chck_handle)"
               "AND enum_contact_test_id="
               "    (SELECT id FROM enum_contact_test WHERE name=$5::varchar)",
               params(check_handle_)(test_name_) );

            if (update_contact_check_res.size() != 1) {
               BOOST_THROW_EXCEPTION(Fred::InternalError("contact_test update failed"));
            }
        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info( to_string() );
            throw;
        }
    }

    std::ostream& operator<<(std::ostream& os, const UpdateContactTest& i) {
        os << "#UpdateContactTest "
            << " check_handle_: "    << i.check_handle_
            << " test_name_: "       << i.test_name_
            << " status_name_: "     << i.status_name_
            << " logd_request_id_: " << i.logd_request_id_.print_quoted()
            << " error_msg_: "       << i.error_msg_.print_quoted();

        return os;
    }

    std::string UpdateContactTest::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
} // namespace Fred
