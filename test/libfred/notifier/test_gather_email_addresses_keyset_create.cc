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
BOOST_AUTO_TEST_SUITE(Keyset)
BOOST_AUTO_TEST_SUITE(Create)

/* time: -inf ---> history ---> notified_version ---> future ---> +inf */
const unsigned history_starts_years_ago = 4;
const unsigned notified_version_starts_years_ago = 3;
const unsigned notification_is_years_ago = 2;
const unsigned future_starts_years_ago = 1;

struct has_keyset : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;

    ::LibFred::InfoContactData keyset_tech_c1_to_be_notified;
    ::LibFred::InfoContactData keyset_tech_c2_to_be_notified;
    ::LibFred::InfoKeysetData keyset;
    has_keyset()
    :
        registrar(Test::registrar(ctx).info_data)
    {
        const std::string tech_c1_handle = "KEYSET1_TECH_C1";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(tech_c1_handle, registrar.handle)
                        .set_email("history.keyset1.tech.c.1@.nic.cz")
                        .set_notifyemail("history.keyset1.tech.c.1.notify@.nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(tech_c1_handle, registrar.handle)
                    .set_email("keyset1.tech.c.1@.nic.cz")
                    .set_notifyemail("keyset1.tech.c.1.notify@.nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(tech_c1_handle, registrar.handle)
                .set_email("future.keyset1.tech.c.1@.nic.cz")
                .set_notifyemail("future.keyset1.tech.c.1.notify@.nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, true );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, true );

            keyset_tech_c1_to_be_notified = ::LibFred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        const std::string tech_c2_handle = "KEYSET2_TECH_C2";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(tech_c2_handle, registrar.handle)
                        .set_email("history.keyset2.tech.c.2@.nic.cz")
                        .set_notifyemail("history.keyset2.tech.c.2.notify@.nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(tech_c2_handle, registrar.handle)
                    .set_email("keyset2.tech.c.2@.nic.cz")
                    .set_notifyemail("keyset2.tech.c.2.notify@.nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(tech_c2_handle, registrar.handle)
                .set_email("future.adminc2@nic.cz")
                .set_notifyemail("future.adminc2notify@nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, true );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, true );

            keyset_tech_c2_to_be_notified = ::LibFred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        const std::string keyset_handle = "KEYSET1";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateKeyset(keyset_handle, registrar.handle)
                        .set_tech_contacts(
                            boost::assign::list_of(tech_c1_handle)(tech_c2_handle)
                        ),
                    ctx
                ).crhistoryid;
            make_history_version_begin_older( ctx, crhid, notification_is_years_ago, true );
            keyset = ::LibFred::InfoKeysetByHandle(keyset_handle).exec(ctx).info_keyset_data;
        }
    }
};

BOOST_FIXTURE_TEST_CASE(test_created_keyset, has_keyset)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( keyset_tech_c1_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( keyset_tech_c2_to_be_notified.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::keyset, Notification::created),
            keyset.crhistoryid
        ) == email_addresses
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
