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

#include "tests/admin/contact/verification/setup_utils.h"
#include "fredlib/contact/verification/enum_testsuite_name.h"

namespace AdminTests {

    void setup_testdef_in_testsuite (Fred::OperationContext& _ctx, const std::string testdef_name, const std::string testsuite_name ) {
        if(
            _ctx.get_conn().exec(
                "INSERT INTO contact_testsuite_map "
                "   (enum_contact_test_id, enum_contact_testsuite_id) "
                "   VALUES ("
                "       (SELECT id FROM enum_contact_test WHERE name='"+ testdef_name +"' ), "
                "       (SELECT id FROM enum_contact_testsuite WHERE name='"+ testsuite_name +"') "
                "   ) "
                "   RETURNING enum_contact_test_id;"
            ).size() != 1
        ) {
            throw std::exception();
        }
    }
}
