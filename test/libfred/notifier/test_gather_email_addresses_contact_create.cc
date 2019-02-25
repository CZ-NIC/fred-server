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
/**
 *  @file
 */

#include <boost/test/unit_test.hpp>
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/notifier/util.hh"

#include "libfred/notifier/gather_email_data/gather_email_addresses.hh"


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailAddresses)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(Create)

/* time: -inf ---> notified_version ---> future ---> +inf */
const unsigned notification_is_years_ago = 4;
const unsigned future_starts_years_ago = 2;

struct has_contact : has_autocomitting_ctx {
    ::LibFred::InfoRegistrarData registrar;
    ::LibFred::InfoContactData cont;

    has_contact() {
        const std::string cont_handle = "CONTACT1";
        registrar = Test::registrar(ctx).info_data;

        const unsigned long long crhid =
            Test::exec(
                ::LibFred::CreateContact(cont_handle, registrar.handle)
                    .set_email("contact.1@nic.cz")
                    .set_notifyemail("contact.1.notify@.nic.cz"),
                ctx
            ).crhistoryid;

        const unsigned long long future_begin_hid =
            ::LibFred::UpdateContactByHandle(cont_handle, registrar.handle)
                .set_email("future.contact.1%nic.cz")
                .set_notifyemail("future.contact.1.notify#nic.cz")
                .exec(ctx);

        make_history_version_begin_older( ctx, crhid, notification_is_years_ago, true );
        make_history_version_end_older( ctx, crhid, future_starts_years_ago);
        make_history_version_begin_older( ctx, future_begin_hid, future_starts_years_ago, false );

        cont = ::LibFred::InfoContactHistoryByHistoryid(crhid).exec(ctx).info_contact_data;
    }
};

BOOST_FIXTURE_TEST_CASE(test_created_contact, has_contact)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( cont.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::created),
            cont.crhistoryid
        ) == email_addresses
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
