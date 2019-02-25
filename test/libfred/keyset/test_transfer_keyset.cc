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
#include <string>

#include "libfred/registrable_object/keyset/transfer_keyset.hh"
#include "libfred/object/transfer_object_exception.hh"
#include "libfred/exception.hh"

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/util.hh"

BOOST_FIXTURE_TEST_SUITE(TestTransferKeyset, Test::has_keyset_and_a_different_registrar)

BOOST_AUTO_TEST_CASE(test_transfer_ok_data)
{
    const unsigned long long logd_request_id = 369874125;

    const unsigned long long post_transfer_history_id = ::LibFred::TransferKeyset(keyset.id, the_different_registrar.handle, keyset.authinfopw, logd_request_id).exec(ctx);

    const std::string timezone = "Europe/Prague";

    const ::LibFred::InfoKeysetOutput post_transfer_keyset_metadata = ::LibFred::InfoKeysetById(keyset.id).exec(ctx);
    const ::LibFred::InfoKeysetData& post_transfer_keyset_data = post_transfer_keyset_metadata.info_keyset_data;

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoRegistrarByHandle(post_transfer_keyset_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id,
        the_different_registrar.id
    );

    BOOST_CHECK_EQUAL(
        post_transfer_keyset_data.transfer_time,
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
        post_transfer_keyset_data.historyid
    );

    BOOST_CHECK_EQUAL(
        logd_request_id,
        post_transfer_keyset_metadata.logd_request_id
    );

    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>(
            ctx.get_conn().exec_params(
                "SELECT MAX(historyid) AS current_hid_ FROM keyset_history WHERE id = $1::integer",
                Database::query_param_list(keyset.id)
            )[0]["current_hid_"]
        ),
        post_transfer_history_id
    );

    BOOST_CHECK_EQUAL(
        post_transfer_keyset_data,
        ::LibFred::InfoKeysetHistoryById(keyset.id).exec(ctx).at(0).info_keyset_data
    );
}

BOOST_AUTO_TEST_CASE(test_transfer_ok_by_keyset_authinfo)
{
    ::LibFred::TransferKeyset transfer(
        keyset.id, the_different_registrar.handle, keyset.authinfopw, Nullable<unsigned long long>()
    );

    BOOST_CHECK_NO_THROW( transfer.exec(ctx) );
}

BOOST_AUTO_TEST_CASE(test_transfer_ok_by_tech_contact)
{
    ::LibFred::TransferKeyset transfer(
        keyset.id, the_different_registrar.handle, tech_contact1.authinfopw, Nullable<unsigned long long>()
    );

    BOOST_CHECK_NO_THROW( transfer.exec(ctx) );
}

BOOST_AUTO_TEST_CASE(test_unknown_registrar)
{
    ::LibFred::TransferKeyset transfer(
        keyset.id,
        "nonexistentregistrar", /* <= !!! */
        keyset.authinfopw,
        Nullable<unsigned long long>()
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::UnknownRegistrar
    );
}

BOOST_AUTO_TEST_CASE(test_unknown_object)
{
    ::LibFred::TransferKeyset transfer(
        Test::get_nonexistent_object_id(ctx), /* <= !!! */
        the_different_registrar.handle,
        keyset.authinfopw,
        Nullable<unsigned long long>()
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::UnknownKeysetId
    );
}

BOOST_AUTO_TEST_CASE(test_incorrect_authinfopw)
{
    ::LibFred::TransferKeyset transfer(
        keyset.id,
        the_different_registrar.handle,
        "IwouldBEverySURPRISEDifANYONEhasSUCHauthinfopw486413514543154144178743404566387036415415051", /* <= !!! */
        Nullable<unsigned long long>()
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::IncorrectAuthInfoPw
    );
}

struct has_keyset_and_a_different_registrar_and_a_different_contact : Test::has_keyset_and_a_different_registrar {

    ::LibFred::InfoContactData different_contact;

    has_keyset_and_a_different_registrar_and_a_different_contact() {
        const std::string different_contact_handle = "THE-DIFFERENT-ONE";
        ::LibFred::CreateContact(different_contact_handle, registrar.handle).exec(ctx);
        different_contact = ::LibFred::InfoContactByHandle(different_contact_handle).exec(ctx).info_contact_data;
    }
};

BOOST_FIXTURE_TEST_CASE(test_incorrect_authinfopw_other_contact, has_keyset_and_a_different_registrar_and_a_different_contact)
{
    ::LibFred::TransferKeyset transfer(
        keyset.id,
        the_different_registrar.handle,
        different_contact.handle, /* <= !!! */
        Nullable<unsigned long long>()
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::IncorrectAuthInfoPw
    );
}

BOOST_AUTO_TEST_CASE(test_registrar_is_already_sponsoring)
{
    ::LibFred::TransferKeyset transfer(
        keyset.id,
        registrar.handle, /* <= !!! */
        keyset.authinfopw,
        Nullable<unsigned long long>()
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::NewRegistrarIsAlreadySponsoring
    );
}

BOOST_AUTO_TEST_SUITE_END()
