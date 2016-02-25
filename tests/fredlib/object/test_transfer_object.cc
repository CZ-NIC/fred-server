/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include <boost/test/unit_test.hpp>

#include "src/fredlib/object/generated_authinfo_password.h"
#include "src/fredlib/object/transfer_object.h"
#include "src/fredlib/object/transfer_object_exception.h"

#include "tests/setup/fixtures.h"
#include "tests/fredlib/util.h"

BOOST_AUTO_TEST_SUITE(TestTransferObject)

BOOST_FIXTURE_TEST_CASE(test_transfer, Test::has_contact_and_a_different_registrar)
{
    const Fred::GeneratedAuthInfoPassword new_authinfo("abcdefgh");

    BOOST_CHECK_NO_THROW(
        Fred::transfer_object(ctx, contact.id, the_different_registrar.handle, new_authinfo)
    );

    const Database::Result post_op_res =
        ctx.get_conn().exec_params(
            "SELECT "
                "clid, "
                "trdate, "
                "authinfopw, "
                "now()::timestamp as now_ "
            "FROM object "
            "WHERE id = $1::integer",
            Database::query_param_list(contact.id)
        );

    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>(
            post_op_res[0]["clid"]
        ),
        the_different_registrar.id
    );

    BOOST_CHECK_EQUAL(
        static_cast<std::string>(
            post_op_res[0]["authinfopw"]
        ),
        new_authinfo.password_
    );

    BOOST_CHECK_EQUAL(
        static_cast<std::string>(
            post_op_res[0]["trdate"]
        ),
        static_cast<std::string>(
            post_op_res[0]["now_"]
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_unknown_registrar, Test::has_contact_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        Fred::transfer_object(ctx, contact.id, "nonexistentregistrar" /* <= !!! */, Fred::GeneratedAuthInfoPassword("abcdefgh")),
        Fred::ExceptionUnknownRegistrar
    );
}

BOOST_FIXTURE_TEST_CASE(test_unknown_object, Test::has_contact_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        Fred::transfer_object(ctx, 42 /* <= !!! */, the_different_registrar.handle, Fred::GeneratedAuthInfoPassword("abcdefgh")),
        Fred::ExceptionUnknownObjectId
    );
}

BOOST_FIXTURE_TEST_CASE(test_registrar_is_already_sponsoring, Test::has_contact_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        Fred::transfer_object(ctx, registrar.id /* <= !!! */, the_different_registrar.handle, Fred::GeneratedAuthInfoPassword("abcdefgh")),
        Fred::ExceptionUnknownObjectId
    );
}

BOOST_AUTO_TEST_SUITE_END();
