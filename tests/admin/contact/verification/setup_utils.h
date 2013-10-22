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
 *  setup utils for integration tests
 */

#ifndef TESTS_ADMIN_CONTACT_VERIFICATION_SETUP_UTILS_H_102876878
#define TESTS_ADMIN_CONTACT_VERIFICATION_SETUP_UTILS_H_102876878

#include <vector>
#include <utility>
#include <string>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/info_check.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "random_data_generator.h"

#include "admin/contact/verification/test_impl/test_interface.h"

namespace AdminTests {

    class DummyTestReturning: public Admin::ContactVerificationTest {
            std::string name;
            std::string description;
            long id;
            std::string return_status;

        public:
            DummyTestReturning (const std::string _return_status)
            : return_status(_return_status)
            {
                name = "DUMMY_TEST_" + return_status + "_" + RandomDataGenerator().xnumstring(15);
                description = name + "_DESCRIPTION";

                Fred::OperationContext ctx;
                id = static_cast<long>(
                     ctx.get_conn().exec(
                         "INSERT INTO enum_contact_test "
                         "   (name, description) "
                         "   VALUES ('" + name + "', '" + description + "') "
                         "   RETURNING id;"
                     )[0]["id"]
                );
                ctx.commit_transaction();
            }
            ContactVerificationTest::T_run_result run(long _history_id) const { return std::make_pair(return_status, return_status); }
            std::string get_name() const { return name; }
    };

    /* Jack the Thrower */
    class DummyThrowingTest: public Admin::ContactVerificationTest {
        std::string name_;
        std::string description_;
        long id_;

        public:
            DummyThrowingTest () {
                Fred::OperationContext ctx;

                name_ = "DUMMY_THROWING_TEST_" + RandomDataGenerator().xnumstring(15);
                description_ = name_ + "_DESCRIPTION";
                Database::Result res = ctx.get_conn().exec(
                    "INSERT INTO enum_contact_test "
                    "   (name, description) "
                    "   VALUES ('" + name_ + "', '" + description_ + "') "
                    "   RETURNING id AS id_; ");

                if(res.size()==0) {
                    throw std::runtime_error("failed to create dummy throwing test");
                }
                ctx.commit_transaction();

                id_ = static_cast<long>(res[0]["id_"]);
            }

            ContactVerificationTest::T_run_result run(long _history_id) const {
                throw std::runtime_error("not exactly a feature");
            }
            std::string get_name() const { return name_; }
    };


    struct setup_testsuite {
        std::string testsuite_name;

        setup_testsuite() {
            testsuite_name = "TESTSUITE_" + RandomDataGenerator().xnumstring(15);
            Fred::OperationContext ctx;
            Database::Result res = ctx.get_conn().exec(
                "INSERT INTO enum_contact_testsuite "
                "   (name, description)"
                "   VALUES ('"+ testsuite_name +"', 'description some text')"
                "   RETURNING id;"
            );
            if(res.size()==0) {
                throw std::runtime_error("failed to create testsuite");
            }
            ctx.commit_transaction();
        }

    };

    void setup_testdef_in_testsuite (const std::string testdef_name, const std::string testsuite_name );

    struct setup_check {
        std::string check_handle_;
        std::string contact_handle_;

        setup_check(const std::string& testsuite_name) {
            // registrar
            Fred::OperationContext ctx1;
            Database::Result registrar_res = ctx1.get_conn().exec("SELECT handle FROM registrar LIMIT 1;");

            if(registrar_res.size() == 0) {
                throw std::exception();
            };

            std::string registrar_handle = static_cast<std::string>(registrar_res[0]["handle"]);

            // contact
            contact_handle_ = "CREATE_CNT_CHECK_" + RandomDataGenerator().xnumstring(15);
            Fred::CreateContact create_contact(contact_handle_, registrar_handle);
            Fred::OperationContext ctx2;
            create_contact.exec(ctx2);
            ctx2.commit_transaction();

            // check
            Fred::CreateContactCheck create_check(contact_handle_, testsuite_name);
            Fred::OperationContext ctx3;
            check_handle_ = create_check.exec(ctx3);
            ctx3.commit_transaction();
        }
    };

}

#endif // #include guard end
