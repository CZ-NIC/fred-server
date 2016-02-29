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
#include <string>

#include "src/fredlib/contact/transfer_contact.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/exception.h"

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/util.h"

BOOST_FIXTURE_TEST_SUITE(TestTransferContact, Test::has_contact_and_a_different_registrar)

BOOST_AUTO_TEST_CASE(test_transfer_ok)
{
    BOOST_CHECK_NO_THROW(
        Fred::TransferContact(contact.id, the_different_registrar.handle, contact.authinfopw).exec(ctx)
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
            post_op_res[0]["trdate"]
        ),
        static_cast<std::string>(
            post_op_res[0]["now_"]
        )
    );
}

BOOST_AUTO_TEST_CASE(test_unknown_registrar)
{
    Fred::TransferContact transfer(
        contact.id,
        "nonexistentregistrar", /* <= !!! */
        contact.authinfopw
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::UnknownRegistrar
    );
}

BOOST_AUTO_TEST_CASE(test_unknown_object)
{
    Fred::TransferContact transfer(
        42, /* <= !!! */
        the_different_registrar.handle,
        contact.authinfopw
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::UnknownContactId
    );
}

BOOST_AUTO_TEST_CASE(test_incorrect_authinfopw)
{
    Fred::TransferContact transfer(
        contact.id,
        the_different_registrar.handle,
        "IwouldBEverySURPRISEDifANYONEhasSUCHauthinfopw486413514543154144178743404566387036415415051"  /* <= !!! */
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::IncorrectAuthInfoPw
    );
}

BOOST_AUTO_TEST_CASE(test_registrar_is_already_sponsoring)
{
    Fred::TransferContact transfer(
        contact.id,
        registrar.handle, /* <= !!! */
        contact.authinfopw
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::NewRegistrarIsAlreadySponsoring
    );
}

BOOST_AUTO_TEST_SUITE_END()
