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
 *  create contact test
 */
#include <boost/algorithm/string/join.hpp>

#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/enum_test_status.h"

namespace Fred
{
    CreateContactTest::CreateContactTest(
        const std::string& _check_handle,
        const std::string& _test_name
    ) :
        check_handle_(_check_handle),
        test_name_(_test_name)
    { }

    CreateContactTest::CreateContactTest(
        const std::string&  _check_handle,
        const std::string&  _test_name,
        Optional<long long> _logd_request_id
    ) :
        check_handle_(_check_handle),
        test_name_(_test_name),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                Nullable<long long>( _logd_request_id.get_value() )
                :
                Nullable<long long>()
        )
    { }

    CreateContactTest& CreateContactTest::set_logd_request_id(long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }

    void CreateContactTest::exec(OperationContext& _ctx) {
        // using solo select for easy checking of existence (subselect would be strange)
        Database::Result check_res = _ctx.get_conn().exec_params(
            "SELECT id "
            "   FROM contact_check "
            "   WHERE handle=$1::uuid "
            "   FOR SHARE;",
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
            "   WHERE name=$1::varchar"
            "   FOR SHARE;",
            Database::query_param_list(test_name_)
        );
        if(test_res.size() != 1) {
            throw ExceptionUnknownTestName();
        }
        long test_id = static_cast<long>(test_res[0]["id"]);

        try {
            _ctx.get_conn().exec_params(
                "INSERT INTO contact_test_result ( "
                "   contact_check_id,"
                "   enum_contact_test_id,"
                "   enum_contact_test_status_id,"
                "   logd_request_id"
                ")"
                "VALUES ("
                "   $1::bigint,"
                "   $2::int,"
                "   (SELECT id FROM enum_contact_test_status WHERE name=$3::varchar),"
                "   $4::bigint"
                ");",
                Database::query_param_list
                    (check_id)
                    (test_id)
                    (Fred::ContactTestStatus::RUNNING)
                    (logd_request_id_)
            );

        } catch(const std::exception& _exc) {

            std::string what_string(_exc.what());

            if(what_string.find("contact_test_result_fk_Contact_check_id") != std::string::npos) {
                throw ExceptionUnknownCheckHandle();
            }

            if(what_string.find("contact_test_result_fk_Enum_contact_test_id") != std::string::npos) {
                throw ExceptionUnknownTestName();
            }

            // problem was elsewhere so let it propagate
            throw;
        }
    }

    std::ostream& operator<<(std::ostream& os, const CreateContactTest& i) {
        os << "#CreateContactTest"
            << " check_handle_: "    << i.check_handle_
            << " test_name_: "       << i.test_name_
            << " logd_request_id_: " << i.logd_request_id_.print_quoted();

        return os;
    }

    std::string CreateContactTest::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

} // namespace Fred
