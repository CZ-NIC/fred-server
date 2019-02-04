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

#include "libfred/registrable_object/contact/transfer_contact.hh"
#include "libfred/object/transfer_object_exception.hh"
#include "libfred/exception.hh"

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/util.hh"

namespace {
    struct has_contact_with_mailing_address_and_a_different_registrar : Test::has_contact_and_a_different_registrar {
        has_contact_with_mailing_address_and_a_different_registrar() {
            ::LibFred::UpdateContactByHandle(contact.handle, registrar.handle)
                .set_address<::LibFred::ContactAddressType::MAILING>(
                    ::LibFred::ContactAddress(
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

BOOST_AUTO_TEST_CASE(test_transfer_ok_data)
{
    const unsigned long long logd_request_id = 123123;

    const unsigned long long post_transfer_history_id = ::LibFred::TransferContact(contact.id, the_different_registrar.handle, contact.authinfopw, logd_request_id).exec(ctx);

    const std::string timezone = "Europe/Prague";

    const ::LibFred::InfoContactOutput post_transfer_contact_metadata = ::LibFred::InfoContactById(contact.id).exec(ctx);
    const ::LibFred::InfoContactData& post_transfer_contact_data = post_transfer_contact_metadata.info_contact_data;


    BOOST_CHECK_EQUAL(
        ::LibFred::InfoRegistrarByHandle(post_transfer_contact_data.sponsoring_registrar_handle).exec(ctx).info_registrar_data.id,
        the_different_registrar.id
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

    BOOST_CHECK_EQUAL(
        post_transfer_contact_data,
        ::LibFred::InfoContactHistoryById(contact.id).exec(ctx).at(0).info_contact_data
    );
}

BOOST_AUTO_TEST_CASE(test_unknown_registrar)
{
    ::LibFred::TransferContact transfer(
        contact.id,
        "nonexistentregistrar", /* <= !!! */
        contact.authinfopw
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::UnknownRegistrar
    );
}

BOOST_AUTO_TEST_CASE(test_unknown_object)
{
    ::LibFred::TransferContact transfer(
        Test::get_nonexistent_object_id(ctx), /* <= !!! */
        the_different_registrar.handle,
        contact.authinfopw
    );

    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::UnknownContactId
    );
}

BOOST_AUTO_TEST_CASE(test_incorrect_authinfopw)
{
    ::LibFred::TransferContact transfer(
        contact.id,
        the_different_registrar.handle,
        "IwouldBEverySURPRISEDifANYONEhasSUCHauthinfopw486413514543154144178743404566387036415415051"  /* <= !!! */
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::IncorrectAuthInfoPw
    );
}

BOOST_AUTO_TEST_CASE(test_registrar_is_already_sponsoring)
{
    ::LibFred::TransferContact transfer(
        contact.id,
        registrar.handle, /* <= !!! */
        contact.authinfopw
    );
    BOOST_CHECK_THROW(
        transfer.exec(ctx),
        ::LibFred::NewRegistrarIsAlreadySponsoring
    );
}

BOOST_AUTO_TEST_SUITE_END()
