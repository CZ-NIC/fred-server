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
#include "src/fredlib/contact/verification/create_test.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>

namespace Fred
{
    CreateContactTest::CreateContactTest(
        const std::string& _check_handle,
        const std::string& _test_handle
    ) :
        check_handle_(_check_handle),
        test_handle_(_test_handle)
    { }

    CreateContactTest::CreateContactTest(
        const std::string&              _check_handle,
        const std::string&              _test_handle,
        Optional<unsigned long long>    _logd_request_id
    ) :
        check_handle_(_check_handle),
        test_handle_(_test_handle),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                Nullable<unsigned long long>( _logd_request_id.get_value() )
                :
                Nullable<unsigned long long>()
        )
    { }

    CreateContactTest& CreateContactTest::set_logd_request_id(unsigned long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }

    void CreateContactTest::exec(OperationContext& _ctx) {
        _ctx.get_log().debug("CreateContactTest exec() started");
        _ctx.get_log().info(to_string());

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
        unsigned long long check_id = static_cast<unsigned long long>(check_res[0]["id"]);

        // get test is in testsuite of this check
        Database::Result testinsuite_res = _ctx.get_conn().exec_params(
            "SELECT c_t.id "
            "   FROM enum_contact_test AS c_t "
            "       JOIN contact_testsuite_map AS c_t_m ON c_t.id = c_t_m.enum_contact_test_id "
            "       JOIN contact_check AS c_c ON c_t_m.enum_contact_testsuite_id = c_c.enum_contact_testsuite_id "
            "   WHERE c_t.handle=$1::varchar "
            "       AND c_c.handle=$2::uuid; ",
            Database::query_param_list(test_handle_)(check_handle_)
        );
        if(testinsuite_res.size() != 1) {
            // is the test really unknown? ... (see ...or below)
            Database::Result test_res = _ctx.get_conn().exec_params(
                "SELECT id "
                "   FROM enum_contact_test "
                "   WHERE handle=$1::varchar; ",
                Database::query_param_list(test_handle_)
            );
            if(test_res.size() != 1) {
                throw ExceptionUnknownTestHandle();
            }

            // ...or was it that i just don't have it in my suite?
            throw ExceptionTestNotInMyTestsuite();
        }


        Database::Result test_res = _ctx.get_conn().exec_params(
            "SELECT id "
            "   FROM enum_contact_test "
            "   WHERE handle=$1::varchar; ",
            Database::query_param_list(test_handle_)
        );
        if(test_res.size() != 1) {
            throw ExceptionUnknownTestHandle();
        }
        unsigned long long test_id = static_cast<unsigned long long>(test_res[0]["id"]);

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
                "   (SELECT id FROM enum_contact_test_status WHERE handle=$3::varchar),"
                "   $4::bigint"
                ");",
                Database::query_param_list
                    (check_id)
                    (test_id)
                    (Fred::ContactTestStatus::ENQUEUED)
                    (logd_request_id_)
            );

        } catch(const std::exception& _exc) {

            std::string what_string(_exc.what());

            if(what_string.find("contact_test_result_fk_Contact_check_id") != std::string::npos) {
                throw ExceptionUnknownCheckHandle();
            }

            if(what_string.find("contact_test_result_fk_Enum_contact_test_id") != std::string::npos) {
                throw ExceptionUnknownTestHandle();
            }

            if(what_string.find("idx_contact_test_result_unique_check_test_pair") != std::string::npos) {
                throw ExceptionCheckTestPairAlreadyExists();
            }

            // problem was elsewhere so let it propagate
            throw;
        }

        _ctx.get_log().debug("CreateContactTest executed successfully");
    }

    std::string CreateContactTest::to_string() const {
        using std::make_pair;

        return Util::format_operation_state(
            "CreateContactTest",
            boost::assign::list_of
                (make_pair("check_handle",      check_handle_ ))
                (make_pair("test_handle",       test_handle_ ))
                (make_pair("logd_request_id",   logd_request_id_.print_quoted() ))
        );
    }

} // namespace Fred
