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

namespace {
    struct has_contact_with_mailing_address_and_a_different_registrar : Test::has_contact_and_a_different_registrar {
        has_contact_with_mailing_address_and_a_different_registrar() {
            Fred::UpdateContactByHandle(contact.handle, registrar.handle)
                .set_address<Fred::ContactAddressType::MAILING>(
                    Fred::ContactAddress(
                        Optional<std::string>(),
                        "ulice 1",
                        Optional<std::string>(),
                        Optional<std::string>(),
                        "mesto",
                        Optional<std::string>(),
                        "12345",
                        "CZ"
                    )
                )
                .exec(ctx);
        }

    };
}

BOOST_FIXTURE_TEST_SUITE(TestTransferContact, has_contact_with_mailing_address_and_a_different_registrar)

BOOST_AUTO_TEST_CASE(test_transfer_ok)
{
    const unsigned long long logd_request_id = 123123;

    const unsigned long long post_transfer_history_id = Fred::TransferContact(contact.id, the_different_registrar.handle, contact.authinfopw, logd_request_id).exec(ctx);

    const Fred::InfoContactOutput post_transfer_contact_metadata = Fred::InfoContactById(contact.id).exec(ctx);
    const Fred::InfoContactData& post_transfer_contact_data = post_transfer_contact_metadata.info_contact_data;

    BOOST_CHECK_EQUAL(
        Fred::InfoRegistrarByHandle(post_transfer_contact_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id,
        the_different_registrar.id
    );

    BOOST_CHECK_EQUAL(
        post_transfer_contact_data.transfer_time,
        post_transfer_contact_metadata.local_timestamp
    );

    BOOST_CHECK_EQUAL(
        post_transfer_history_id,
        post_transfer_contact_data.historyid
    );

    BOOST_CHECK_EQUAL(
        logd_request_id,
        post_transfer_contact_metadata.logd_request_id
    );

    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>(
            ctx.get_conn().exec_params(
                "SELECT MAX(historyid) AS current_hid_ FROM contact_history WHERE id = $1::integer",
                Database::query_param_list(contact.id)
            )[0]["current_hid_"]
        ),
        post_transfer_history_id
    );

    BOOST_CHECK_EQUAL(
        static_cast<unsigned long long>(
            ctx.get_conn().exec_params(
                "SELECT MAX(historyid) AS current_hid_ FROM contact_address_history WHERE contactid = $1::integer AND type = 'MAILING'::contact_address_type ",
                Database::query_param_list(contact.id)
            )[0]["current_hid_"]
        ),
        post_transfer_history_id
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
        Test::get_nonexistent_object_id(ctx), /* <= !!! */
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
