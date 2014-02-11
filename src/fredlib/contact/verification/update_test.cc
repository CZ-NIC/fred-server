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

#include "src/fredlib/contact/verification/update_test.h"

#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>

namespace Fred
{
   UpdateContactTest::UpdateContactTest(
        const std::string& _check_handle,
        const std::string& _test_name,
        const std::string& _status_handle
    ) :
        check_handle_(_check_handle),
        test_handle_(_test_name),
        status_handle_(_status_handle)
    { }

    UpdateContactTest::UpdateContactTest(
        const std::string&    _check_handle,
        const std::string&    _test_handle,
        const std::string&    _status_handle,
        Optional<long long>   _logd_request_id,
        Optional<std::string> _error_msg
    ) :
        check_handle_(_check_handle),
        test_handle_(_test_handle),
        status_handle_(_status_handle),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                Nullable<long long>( _logd_request_id.get_value() )
                :
                Nullable<long long>()
        ),
        error_msg_(
            ( _error_msg.isset() )
                ?
                Nullable<std::string>( _error_msg.get_value() )
                :
                Nullable<std::string>()
        )
    { }

    UpdateContactTest& UpdateContactTest::set_logd_request_id (long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }

    UpdateContactTest& UpdateContactTest::set_error_msg (const std::string& _error_msg) {
        error_msg_ = _error_msg;
        return *this;
    }

    void UpdateContactTest::exec (OperationContext& _ctx) {
        _ctx.get_log().debug("UpdateContactTest exec() started");
        _ctx.get_log().info(to_string());

        // using solo select for easy checking of existence (subselect would be strange)
        Database::Result status_res = _ctx.get_conn().exec_params(
            "SELECT id "
            "   FROM enum_contact_test_status "
            "   WHERE handle=$1::varchar; ",
            Database::query_param_list(status_handle_)
        );
        if(status_res.size() != 1) {
            throw ExceptionUnknownTestStatusHandle();
        }
        long status_id = static_cast<long>(status_res[0]["id"]);

        // using solo select for easy checking of existence (subselect would be strange)
        Database::Result check_res = _ctx.get_conn().exec_params(
            "SELECT id "
            "   FROM contact_check "
            "   WHERE handle=$1::uuid; ",
            Database::query_param_list(check_handle_)
        );
        if(check_res.size() != 1) {
            throw ExceptionUnknownCheckHandle();
        }
        long check_id = static_cast<long>(check_res[0]["id"]);

        // using solo select for easy checking of existence (subselect would be strange)
        Database::Result test_res = _ctx.get_conn().exec_params(
            "SELECT id "
            "   FROM enum_contact_test "
            "   WHERE handle=$1::varchar; ",
            Database::query_param_list(test_handle_)
        );
        if(test_res.size() != 1) {
            throw ExceptionUnknownTestHandle();
        }
        long test_id = static_cast<long>(test_res[0]["id"]);

        try {
            Database::Result update_contact_test_res = _ctx.get_conn().exec_params(
                "UPDATE contact_test_result SET ( "
                "    enum_contact_test_status_id, "
                "    logd_request_id, "
                "    error_msg "
                ") = ( "
                "    $1::int, "
                "    $2::bigint, "
                "    $3::varchar "
                ") "
                "WHERE contact_check_id=$4::bigint "
                "   AND enum_contact_test_id=$5::int "
                "RETURNING id;",
                Database::query_param_list
                    (status_id)
                    (logd_request_id_)
                    (error_msg_)
                    (check_id)
                    (test_id)
            );

            if (update_contact_test_res.size() != 1) {
                // is specified record existing at all?
                if( _ctx.get_conn().exec_params(
                       "SELECT id "
                       "    FROM contact_test_result "
                       "    WHERE contact_check_id=$1::bigint "
                       "        AND enum_contact_test_id=$2::int;",
                       Database::query_param_list(check_id)(test_id)
                    ).size() != 1
                ) {
                    throw ExceptionUnknownCheckTestPair();
                }
                // ok, it exists but we have problem non-the-less...
                BOOST_THROW_EXCEPTION(Fred::InternalError("contact_test update failed"));
            }
        } catch(const std::exception& _exc) {

            std::string what_string(_exc.what());

            if(what_string.find("contact_test_result_fk_Contact_check_id") != std::string::npos) {
                throw ExceptionUnknownCheckHandle();
            }

            if(what_string.find("contact_test_result_fk_Enum_contact_test_id") != std::string::npos) {
                throw ExceptionUnknownTestHandle();
            }

            if(what_string.find("contact_test_result_history_fk_Enum_contact_test_status_id") != std::string::npos) {
                throw ExceptionUnknownTestStatusHandle();
            }

            // problem was elsewhere so let it propagate
            throw;
        }

        _ctx.get_log().debug("UpdateContactTest executed succesfully");
    }

    std::string UpdateContactTest::to_string() const {
        using std::make_pair;

        return Util::format_operation_state(
            "ListContactChecks",
            boost::assign::list_of
                (make_pair("check_handle",      check_handle_ ))
                (make_pair("test_handle",       test_handle_ ))
                (make_pair("status_handle",     status_handle_ ))
                (make_pair("logd_request_id",   logd_request_id_.print_quoted() ))
                (make_pair("error_msg",         error_msg_.print_quoted() ))
        );
    }
} // namespace Fred
