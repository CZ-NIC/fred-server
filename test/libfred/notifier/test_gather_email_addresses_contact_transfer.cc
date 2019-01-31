/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 */

#include <boost/test/unit_test.hpp>
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/notifier/util.hh"

#include "libfred/notifier/gather_email_data/gather_email_addresses.hh"
#include "libfred/registrable_object/contact/transfer_contact.hh"


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailAddresses)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(Transfer)

/* time: -inf ---> history ---> notified_version ---> future ---> +inf */
const unsigned history_starts_years_ago = 4;
const unsigned notified_version_starts_years_ago = 3;
const unsigned notification_is_years_ago = 2;
const unsigned future_starts_years_ago = 1;

struct has_contact : has_autocomitting_ctx {
    ::LibFred::InfoRegistrarData registrar;
    ::LibFred::InfoContactData cont;

    has_contact() {
        const std::string cont_handle = "CONTACT1";
        registrar = Test::registrar(ctx).info_data;

        const unsigned long long crhid =
            Test::exec(
                ::LibFred::CreateContact(cont_handle, registrar.handle)
                    .set_email("history.contact.1@nic.cz")
                    .set_notifyemail("history.contact.1.notify@.nic.cz"),
                ctx
            ).crhistoryid;

        const unsigned long long hid_to_be_notified =
            ::LibFred::UpdateContactByHandle(cont_handle, registrar.handle)
                .set_email("contact.1%nic.cz")
                .set_notifyemail("contact.1.notify#nic.cz")
                .exec(ctx);

        make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
        make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);
        make_history_version_begin_older( ctx, hid_to_be_notified, notified_version_starts_years_ago, false );

        cont = ::LibFred::InfoContactHistoryByHistoryid(hid_to_be_notified).exec(ctx).info_contact_data;
    }
};

struct has_transferred_contact : public has_contact {
    ::LibFred::InfoRegistrarData new_registrar;
    ::LibFred::InfoContactData post_transfer_contact_data;

    has_transferred_contact() {
        new_registrar = Test::registrar(ctx).info_data;

        const unsigned long long post_transfer_hid = ::LibFred::TransferContact(cont.id, new_registrar.handle, cont.authinfopw).exec(ctx);
        const unsigned long long future_begin_hid =
            ::LibFred::UpdateContactByHandle(cont.handle, registrar.handle)
                .set_email("future.contact.1%nic.cz")
                .set_notifyemail("future.contact.1.notify#nic.cz")
                .exec(ctx);

        make_history_version_end_older( ctx, post_transfer_hid, notification_is_years_ago);
        make_history_version_begin_older( ctx, future_begin_hid, future_starts_years_ago, false);

        post_transfer_contact_data = ::LibFred::InfoContactHistoryByHistoryid( post_transfer_hid ).exec(ctx).info_contact_data;

    }
};

BOOST_FIXTURE_TEST_CASE(test_transferred_contact, has_transferred_contact)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( post_transfer_contact_data.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::transferred),
            post_transfer_contact_data.historyid
        ) == email_addresses
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
