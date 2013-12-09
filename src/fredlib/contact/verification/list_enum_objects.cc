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
 *  functions for listing of various auxiliary enumerated objects realted to admin contact verification
 */

#include "fredlib/contact/verification/list_enum_objects.h"
#include "fredlib/opcontext.h"
#include "fredlib/opexception.h"

namespace Fred
{
    std::vector<test_result_status> list_test_result_statuses(const std::string& lang ) {
        try {
            Fred::OperationContext ctx;

            // TODO lang
            Database::Result names_res = ctx.get_conn().exec(
                "SELECT "
                "   name, description "
                "   FROM enum_contact_test_status ");

            std::vector<test_result_status> result;

            result.reserve(names_res.size());

            for(Database::Result::Iterator it = names_res.begin(); it != names_res.end(); ++it) {
               result.push_back(
                   test_result_status(
                       static_cast<std::string>( (*it)["name"] ),
                       static_cast<std::string>( (*it)["name"] ), // TODO tady casem preklad
                       static_cast<std::string>( (*it)["description"] )
                   )
               );
            }

            return result;

        } catch (...) {
            throw Fred::InternalError("failed to get test statuses");
        }
    }

    std::vector<check_status> list_check_statuses(const std::string& lang ) {
        try {
            Fred::OperationContext ctx;

            // TODO lang
            Database::Result names_res = ctx.get_conn().exec(
                "SELECT "
                "   name, description "
                "   FROM enum_contact_check_status ");

            std::vector<check_status> result;

            result.reserve(names_res.size());

            for(Database::Result::Iterator it = names_res.begin(); it != names_res.end(); ++it) {
                result.push_back(
                    check_status(
                        static_cast<std::string>( (*it)["name"] ),
                        static_cast<std::string>( (*it)["name"] ), // TODO tady casem preklad
                        static_cast<std::string>( (*it)["description"] )
                    )
                );
            }

            return result;

        } catch (...) {
            throw Fred::InternalError("failed to get check statuses");
        }
    }

    std::vector<test_definition> list_test_definitions(const std::string& lang, const std::string& testsuite_name ) {
        try {
            Fred::OperationContext ctx;

            // TODO lang
            std::string query =
                "SELECT "
                "   enum_c_t.name           AS name_, "
                "   enum_c_t.description    AS description_"
                "   FROM enum_contact_test AS enum_c_t ";
            Database::Result names_res;
            if(testsuite_name.empty()) {
                names_res = ctx.get_conn().exec(query);
            } else {
                query +=
                        "   JOIN contact_testsuite_map AS c_t_m ON c_t_m.enum_contact_test_id = enum_c_t.id "
                        "   JOIN enum_contact_testsuite AS enum_c_tst ON c_t_m.enum_contact_testsuite_id = enum_c_tst.id "
                        // TODO tady se asi bude menit sloupec ve where
                        "   WHERE enum_c_tst.name = $1::varchar";
                names_res = ctx.get_conn().exec_params(
                    query,
                    Database::query_param_list(testsuite_name)
                );
            }

            std::vector<test_definition> result;

            result.reserve(names_res.size());

            for(Database::Result::Iterator it = names_res.begin(); it != names_res.end(); ++it) {
                result.push_back(
                    test_definition(
                        static_cast<std::string>( (*it)["name_"] ),
                        static_cast<std::string>( (*it)["name_"] ), // TODO tady casem preklad
                        static_cast<std::string>( (*it)["description_"] )
                    )
                );
            }

            return result;

        } catch (...) {
            throw Fred::InternalError("failed to get test definitions");
        }
    }

    std::vector<testsuite_definition> list_testsuite_definitions(const std::string& lang ) {
        try {
            Fred::OperationContext ctx;

            // TODO lang
            Database::Result names_res = ctx.get_conn().exec(
                "SELECT "
                "   name, description "
                "   FROM enum_contact_testsuite ");

            std::vector<testsuite_definition> result;

            result.reserve(names_res.size());

            for(Database::Result::Iterator it = names_res.begin(); it != names_res.end(); ++it) {
                result.push_back(
                    testsuite_definition(
                        static_cast<std::string>( (*it)["name"] ),
                        static_cast<std::string>( (*it)["name"] ), // TODO tady casem preklad
                        static_cast<std::string>( (*it)["description"] )
                    )
                );
                result.back().tests = list_test_definitions(lang, static_cast<std::string>( (*it)["name"] ));
            }

            return result;

        } catch (...) {
            throw Fred::InternalError("failed to get testsuite definitions");
        }
    }

}
