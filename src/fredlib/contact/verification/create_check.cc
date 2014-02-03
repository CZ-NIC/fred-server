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
 *  create contact check
 */

#include "util/random_data_generator.h"

#include "src/fredlib/contact/verification/create_check.h"
#include "src/fredlib/contact/verification/create_test.h"
#include "src/fredlib/contact/verification/enum_check_status.h"

#include <utility>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

namespace Fred
{
    CreateContactCheck::CreateContactCheck(
        long long           _contact_id,
        const std::string& _testsuite_handle
    ) :
        contact_id_(_contact_id),
        testsuite_handle_(_testsuite_handle)
    { }

    CreateContactCheck::CreateContactCheck(
        long long           _contact_id,
        const std::string&  _testsuite_handle,
        Optional<long long> _logd_request_id
    ) :
        contact_id_(_contact_id),
        testsuite_handle_(_testsuite_handle),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                Nullable<long long>( _logd_request_id.get_value() )
                :
                Nullable<long long>()
        )
    { }

    CreateContactCheck& CreateContactCheck::set_logd_request_id(long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }

    std::string CreateContactCheck::exec(OperationContext& _ctx) {
        _ctx.get_log().debug("CreateContactCheck exec() started");
        _ctx.get_log().info(to_string());

        Fred::OperationContext ctx_unique;
        std::string unique_test_query =
                "SELECT handle "
                "   FROM contact_check "
                "   WHERE handle=$1::uuid ";
        // generate handle over and over until it is unique
        std::string handle;
        do {
            handle = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
        } while(
            ctx_unique.get_conn().exec_params(
                unique_test_query,
                Database::query_param_list(handle)
                )
            .size() != 0
        );

        // using solo select for easy checking of existence (subselect would be strange)
        Database::Result contact_history_res = _ctx.get_conn().exec_params(
            "SELECT obj_reg.historyid AS historyid_ "
            "   FROM object_registry AS obj_reg "
            "       JOIN enum_object_type AS e_o_t ON obj_reg.type = e_o_t.id "
            "   WHERE obj_reg.id=$1::integer "
            "       AND e_o_t.name = $2::varchar "
            "       AND obj_reg.erdate IS NULL; ",
            Database::query_param_list(contact_id_)("contact")
        );
        if(contact_history_res.size() != 1) {
            throw ExceptionUnknownContactId();
        }
        long contact_history_id = static_cast<long>(contact_history_res[0]["historyid_"]);

        Database::Result testsuite_res = _ctx.get_conn().exec_params(
            "SELECT id "
            "   FROM enum_contact_testsuite "
            "   WHERE handle=$1::varchar; ",
            Database::query_param_list(testsuite_handle_)
        );
        if(testsuite_res.size() != 1) {
            throw ExceptionUnknownTestsuiteHandle();
        }
        long testsuite_id = static_cast<long>(testsuite_res[0]["id"]);

        try {
            _ctx.get_conn().exec_params(
                "INSERT INTO contact_check ( "
                "   handle,"
                "   contact_history_id,"
                "   enum_contact_testsuite_id,"
                "   enum_contact_check_status_id,"
                "   logd_request_id"
                ")"
                "VALUES ("
                "   $1::uuid,"
                "   $2::int,"
                "   $3::int,"
                "   (SELECT id FROM enum_contact_check_status WHERE handle=$4::varchar),"
                "   $5::bigint"
                ");",
                Database::query_param_list
                    (handle)
                    (contact_history_id)
                    (testsuite_id)
                    (Fred::ContactCheckStatus::ENQUEUED)
                    (logd_request_id_)
            );
        } catch(const std::exception& _exc) {

            std::string what_string(_exc.what());

            if(what_string.find("fk_contact_check_contact_history_id") != std::string::npos) {
                throw ExceptionUnknownContactId();
            }

            if(what_string.find("contact_check_fk_Enum_contact_testsuite_id") != std::string::npos) {
                throw ExceptionUnknownTestsuiteHandle();
            }

            // problem was elsewhere so let it propagate
            throw;
        }

        _ctx.get_log().debug("CreateContactCheck executed succesfully");

        return handle;
    }

    std::string CreateContactCheck::to_string() const {
        using std::string;
        using std::make_pair;
        using boost::lexical_cast;

        return Util::format_operation_state(
            "CreateContactCheck",
            boost::assign::list_of
                (make_pair("contact_id",        lexical_cast<string>(contact_id_) ))
                (make_pair("testsuite_handle",  testsuite_handle_ ))
                (make_pair("logd_request_id",   logd_request_id_.print_quoted() ))
        );
    }

} // namespace Fred
