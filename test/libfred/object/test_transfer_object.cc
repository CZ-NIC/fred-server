/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <boost/test/unit_test.hpp>

#include "libfred/object/generated_authinfo_password.hh"
#include "libfred/object/transfer_object.hh"
#include "libfred/object/transfer_object_exception.hh"

#include "test/setup/fixtures.hh"
#include "test/libfred/util.hh"

BOOST_AUTO_TEST_SUITE(TestTransferObject)

BOOST_FIXTURE_TEST_CASE(test_transfer, Test::has_contact_and_a_different_registrar)
{
    const ::LibFred::GeneratedAuthInfoPassword new_authinfo("abcdefgh");

    const unsigned long long logd_request_id = 123456;

    const unsigned long long post_transfer_history_id = ::LibFred::transfer_object(
        ctx,
        contact.id,
        the_different_registrar.handle,
        new_authinfo,
        logd_request_id
    );

    const std::string timezone = "Europe/Prague";

    const ::LibFred::InfoContactOutput post_transfer_contact_metadata = ::LibFred::InfoContactById(contact.id).exec(ctx, timezone);
    const ::LibFred::InfoContactData& post_transfer_contact_data = post_transfer_contact_metadata.info_contact_data;

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoRegistrarByHandle(post_transfer_contact_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id,
        the_different_registrar.id
    );

    BOOST_CHECK_EQUAL(
        post_transfer_contact_data.authinfopw,
        new_authinfo.password_
    );

    BOOST_CHECK_EQUAL(
        post_transfer_contact_data.transfer_time,
        boost::posix_time::time_from_string(
            static_cast<std::string>(
                ctx.get_conn().exec_params(
                    "SELECT now()::TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text",
                    Database::query_param_list(timezone)
                )[0][0]
            )
        )
    );

    BOOST_CHECK_EQUAL(
        post_transfer_history_id,
        post_transfer_contact_data.historyid
    );

    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>(
            ctx.get_conn().exec_params(
                "SELECT MAX(historyid) AS current_hid_ FROM object_history WHERE id = $1::integer",
                Database::query_param_list(contact.id)
            )[0]["current_hid_"]
        ),
        post_transfer_history_id
    );

    BOOST_CHECK_EQUAL(
        logd_request_id,
        post_transfer_contact_metadata.logd_request_id
    );
}

BOOST_FIXTURE_TEST_CASE(test_unknown_registrar, Test::has_contact_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        ::LibFred::transfer_object(ctx, contact.id, "nonexistentregistrar" /* <= !!! */, ::LibFred::GeneratedAuthInfoPassword("abcdefgh")),
        ::LibFred::UnknownRegistrar
    );
}

BOOST_FIXTURE_TEST_CASE(test_unknown_object, Test::has_contact_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        ::LibFred::transfer_object(ctx, Test::get_nonexistent_object_id(ctx) /* <= !!! */, the_different_registrar.handle, ::LibFred::GeneratedAuthInfoPassword("abcdefgh")),
        ::LibFred::UnknownObjectId
    );
}

BOOST_FIXTURE_TEST_CASE(test_registrar_is_already_sponsoring, Test::has_contact_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        ::LibFred::transfer_object(ctx, contact.id, registrar.handle, ::LibFred::GeneratedAuthInfoPassword("abcdefgh")),
        ::LibFred::NewRegistrarIsAlreadySponsoring
    );
}

BOOST_AUTO_TEST_SUITE_END();
