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

#include <fredlib/contact.h>
#include <fredlib/admin_contact_verification.h>

#include "src/fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "util/random_data_generator.h"

#include "tests/fredlib/contact/verification/setup_utils.h"
#include "src/admin/contact/verification/test_impl/test_interface.h"


class DummyTestReturning: public Admin::ContactVerification::Test {
        std::string handle;
        std::string description;
        long id;
        std::string return_status;

    public:
        DummyTestReturning (const std::string _return_status)
        : return_status(_return_status)
        {
            // prevent name collisions
            while(true) {
                try {
                    handle = "DUMMY_TEST_" + return_status + "_" + RandomDataGenerator().xnumstring(15);
                    description = handle + "_DESCRIPTION";

                    Fred::OperationContext ctx;
                    id = static_cast<long>(
                         ctx.get_conn().exec(
                             "INSERT INTO enum_contact_test "
                             "   (id, handle) "
                             "   VALUES ("+RandomDataGenerator().xnumstring(9)+", '" + handle + "') "
                             "   RETURNING id;"
                         )[0]["id"]
                    );
                    ctx.commit_transaction();
                } catch (Database::ResultFailed& ) {
                    continue;
                }
                break;
            }
        }
        T_run_result run(unsigned long long _history_id) const {
            return make_result(return_status, return_status);
        }
        static std::string registration_name() { return "DummyTestReturning"; }

        std::string get_handle() const { return handle; }
};

/* Jack the Thrower */
class DummyThrowingTest: public Admin::ContactVerification::Test {
    std::string handle_;
    std::string description_;
    long id_;

    public:
        DummyThrowingTest () {
            Database::Result res;
            // prevent name collisions
            while(true) {
                try {
                    Fred::OperationContext ctx;

                    handle_ = "DUMMY_THROWING_TEST_" + RandomDataGenerator().xnumstring(15);
                    description_ = handle_ + "_DESCRIPTION";
                    res = ctx.get_conn().exec(
                        "INSERT INTO enum_contact_test "
                        "   (id, handle) "
                        "   VALUES ("+RandomDataGenerator().xnumstring(9)+", '" + handle_ + "') "
                        "   RETURNING id AS id_; ");

                    if(res.size()==0) {
                        throw std::runtime_error("failed to create dummy throwing test");
                    }
                    ctx.commit_transaction();
                } catch (Fred::OperationException& ) {
                    continue;
                }
                break;
            }

            id_ = static_cast<long>(res[0]["id_"]);
        }

        T_run_result run(unsigned long long _history_id) const {
            throw std::runtime_error("not exactly a feature");
        }
        static std::string registration_name() { return "DummyThrowingTest"; }

        std::string get_handle() const { return handle_; }
};

#endif // #include guard end
