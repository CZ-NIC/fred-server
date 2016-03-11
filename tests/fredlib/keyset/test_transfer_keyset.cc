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

#include "src/fredlib/keyset/transfer_keyset.h"
#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/exception.h"

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/util.h"

BOOST_FIXTURE_TEST_SUITE(TestTransferKeyset, Test::has_keyset_and_a_different_registrar)

BOOST_AUTO_TEST_CASE(test_transfer_ok_data)
{
    const unsigned long long logd_request_id = 369874125;

    const unsigned long long post_transfer_history_id = Fred::TransferKeyset(keyset.id, the_different_registrar.handle, keyset.authinfopw, logd_request_id).exec(ctx);

    const std::string timezone = "Europe/Prague";

    const Fred::InfoKeysetOutput post_transfer_keyset_metadata = Fred::InfoKeysetById(keyset.id).exec(ctx);
    const Fred::InfoKeysetData& post_transfer_keyset_data = post_transfer_keyset_metadata.info_keyset_data;

    BOOST_CHECK_EQUAL(
        Fred::InfoRegistrarByHandle(post_transfer_keyset_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id,
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
        Fred::InfoKeysetHistoryById(keyset.id).exec(ctx).at(0).info_keyset_data
    );
}

BOOST_AUTO_TEST_CASE(test_transfer_ok_by_keyset_authinfo)
{
    Fred::TransferKeyset transfer(
        keyset.id, the_different_registrar.handle, keyset.authinfopw, Nullable<unsigned long long>()
    );

    BOOST_CHECK_NO_THROW( transfer.exec(ctx) );
}

BOOST_AUTO_TEST_CASE(test_transfer_ok_by_tech_contact)
{
    Fred::TransferKeyset transfer(
        keyset.id, the_different_registrar.handle, tech_contact1.authinfopw, Nullable<unsigned long long>()
    );

    BOOST_CHECK_NO_THROW( transfer.exec(ctx) );
}

BOOST_AUTO_TEST_CASE(test_unknown_registrar)
{
    Fred::TransferKeyset transfer(
        keyset.id,
        "nonexistentregistrar", /* <= !!! */
        keyset.authinfopw,
        Nullable<unsigned long long>()
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::UnknownRegistrar
    );
}

BOOST_AUTO_TEST_CASE(test_unknown_object)
{
    Fred::TransferKeyset transfer(
        Test::get_nonexistent_object_id(ctx), /* <= !!! */
        the_different_registrar.handle,
        keyset.authinfopw,
        Nullable<unsigned long long>()
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::UnknownKeysetId
    );
}

BOOST_AUTO_TEST_CASE(test_incorrect_authinfopw)
{
    Fred::TransferKeyset transfer(
        keyset.id,
        the_different_registrar.handle,
        "IwouldBEverySURPRISEDifANYONEhasSUCHauthinfopw486413514543154144178743404566387036415415051", /* <= !!! */
        Nullable<unsigned long long>()
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::IncorrectAuthInfoPw
    );
}

struct has_keyset_and_a_different_registrar_and_a_different_contact : Test::has_keyset_and_a_different_registrar {

    Fred::InfoContactData different_contact;

    has_keyset_and_a_different_registrar_and_a_different_contact() {
        const std::string different_contact_handle = "THE_DIFFERENT_ONE";
        Fred::CreateContact(different_contact_handle, registrar.handle).exec(ctx);
        different_contact = Fred::InfoContactByHandle(different_contact_handle).exec(ctx).info_contact_data;
    }
};

BOOST_FIXTURE_TEST_CASE(test_incorrect_authinfopw_other_contact, has_keyset_and_a_different_registrar_and_a_different_contact)
{
    Fred::TransferKeyset transfer(
        keyset.id,
        the_different_registrar.handle,
        different_contact.handle, /* <= !!! */
        Nullable<unsigned long long>()
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::IncorrectAuthInfoPw
    );
}

BOOST_AUTO_TEST_CASE(test_registrar_is_already_sponsoring)
{
    Fred::TransferKeyset transfer(
        keyset.id,
        registrar.handle, /* <= !!! */
        keyset.authinfopw,
        Nullable<unsigned long long>()
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        Fred::NewRegistrarIsAlreadySponsoring
    );
}

BOOST_AUTO_TEST_SUITE_END()
