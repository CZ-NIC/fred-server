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

#include "src/fredlib/contact/verification/create_check.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/db_settings.h"
#include "util/db/nullable.h"
#include "util/random_data_generator.h"

#include "tests/fredlib/contact/verification/setup_utils.h"

#include "src/admin/contact/verification/test_impl/test_interface.h"


class DummyTestReturning: public Admin::ContactVerificationTest {
        std::string name;
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
                } catch (Database::ResultFailed& ) {
                    continue;
                }
                break;
            }
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
            Database::Result res;
            // prevent name collisions
            while(true) {
                try {
                    Fred::OperationContext ctx;

                    name_ = "DUMMY_THROWING_TEST_" + RandomDataGenerator().xnumstring(15);
                    description_ = name_ + "_DESCRIPTION";
                    res = ctx.get_conn().exec(
                        "INSERT INTO enum_contact_test "
                        "   (name, description) "
                        "   VALUES ('" + name_ + "', '" + description_ + "') "
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

        ContactVerificationTest::T_run_result run(long _history_id) const {
            throw std::runtime_error("not exactly a feature");
        }
        std::string get_name() const { return name_; }
};

#endif // #include guard end
