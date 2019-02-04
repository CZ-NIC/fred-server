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

#ifndef SETUP_UTILS_HH_EE8CE4C846E54B10BF252B1C0A414EAF
#define SETUP_UTILS_HH_EE8CE4C846E54B10BF252B1C0A414EAF

#include "test/libfred/contact/verification/setup_utils.hh"

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"
#include "libfred/db_settings.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "libfred/registrable_object/contact/verification/exceptions.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/list_checks.hh"
#include "libfred/registrable_object/contact/verification/list_enum_objects.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"
#include "libfred/registrable_object/contact/verification/update_test.hh"
#include "util/db/nullable.hh"
#include "util/random_data_generator.hh"

#include <string>
#include <utility>
#include <vector>

class DummyTestReturning: public Fred::Backend::Admin::Contact::Verification::Test {
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

                    ::LibFred::OperationContextCreator ctx;
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
        TestRunResult run(unsigned long long _history_id) const {
            return TestRunResult(return_status, return_status);
        }
        static std::string registration_name() { return "DummyTestReturning"; }

        std::string get_handle() const { return handle; }
};

/* Jack the Thrower */
class DummyThrowingTest: public Fred::Backend::Admin::Contact::Verification::Test {
    std::string handle_;
    std::string description_;
    long id_;

    public:
        DummyThrowingTest () {
            Database::Result res;
            // prevent name collisions
            while(true) {
                try {
                    ::LibFred::OperationContextCreator ctx;

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
                } catch (::LibFred::OperationException& ) {
                    continue;
                }
                break;
            }

            id_ = static_cast<long>(res[0]["id_"]);
        }

        TestRunResult run(unsigned long long _history_id) const {
            throw std::runtime_error("not exactly a feature");
        }
        static std::string registration_name() { return "DummyThrowingTest"; }

        std::string get_handle() const { return handle_; }
};

#endif
