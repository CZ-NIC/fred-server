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


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailAddresses)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(Update)

/* time: -inf ---> history ---> notified_version ---> future ---> +inf */
const unsigned history_starts_years_ago = 4;
const unsigned notified_version_starts_years_ago = 3;
const unsigned notification_is_years_ago = 2;
const unsigned future_starts_years_ago = 1;

struct has_contact_with_notify_mail : has_autocomitting_ctx {
    ::LibFred::InfoRegistrarData registrar;
    ::LibFred::InfoContactData pre_update_contact_data;

    has_contact_with_notify_mail() {

        registrar = Test::registrar(ctx).info_data;
        const std::string cont_handle = "CONTACT1";

        const unsigned long long crhid =
            Test::exec(
                ::LibFred::CreateContact(cont_handle, registrar.handle)
                    .set_email("history.contact.1@nic.cz")
                    .set_notifyemail("history.contact.1.notify#nic.cz"),
                ctx
            ).crhistoryid;

        make_history_version_begin_older( ctx, crhid, notified_version_starts_years_ago, true );

        pre_update_contact_data = ::LibFred::InfoContactHistoryByHistoryid(crhid).exec(ctx).info_contact_data;
    }
};

struct has_contact_without_notify_mail : has_autocomitting_ctx {
    ::LibFred::InfoRegistrarData registrar;
    ::LibFred::InfoContactData pre_update_contact_data;

    has_contact_without_notify_mail() {

        registrar = Test::registrar(ctx).info_data;
        const std::string cont_handle = "CONTACT1";

        const unsigned long long crhid =
            Test::exec(
                ::LibFred::CreateContact(cont_handle, registrar.handle)
                    .set_email("pre.history.contact.1@nic.cz")
                    .set_notifyemail("pre.history.contact.1.notify#nic.cz"),
                ctx
            ).crhistoryid;


        const unsigned long long pre_update_hid =
            ::LibFred::UpdateContactByHandle(cont_handle, registrar.handle)
                .set_email("history.contact.1@nic.cz")
                .set_notifyemail("")
                .exec(ctx);

        make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
        make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);
        make_history_version_begin_older( ctx, pre_update_hid, notified_version_starts_years_ago, false );

        pre_update_contact_data = ::LibFred::InfoContactHistoryByHistoryid(pre_update_hid).exec(ctx).info_contact_data;
    }
};

template<typename T_has_contact> struct has_updated_contact_name : public T_has_contact {
    ::LibFred::InfoContactData post_update_contact_data;

    has_updated_contact_name() {

        const unsigned long long post_update_hid =
            ::LibFred::UpdateContactByHandle(T_has_contact::pre_update_contact_data.handle, T_has_contact::registrar.handle)
                .set_name(T_has_contact::pre_update_contact_data.name.get_value_or("") + "abc")
                .exec(T_has_contact::ctx);

        make_history_version_end_older( T_has_contact::ctx, T_has_contact::pre_update_contact_data.historyid, notification_is_years_ago);
        make_history_version_begin_older( T_has_contact::ctx, post_update_hid, notification_is_years_ago, false );

        post_update_contact_data = ::LibFred::InfoContactHistoryByHistoryid( post_update_hid ).exec(T_has_contact::ctx).info_contact_data;
    }
};

template<typename T_has_contact> struct has_set_contact_notifymail : public T_has_contact {
    ::LibFred::InfoContactData post_update_contact_data;

    has_set_contact_notifymail() {

        const unsigned long long post_update_hid =
            ::LibFred::UpdateContactByHandle(T_has_contact::pre_update_contact_data.handle, T_has_contact::registrar.handle)
                .set_notifyemail("contact.1.notify@nic.cz")
                .exec(T_has_contact::ctx);

        make_history_version_end_older( T_has_contact::ctx, T_has_contact::pre_update_contact_data.historyid, notification_is_years_ago);
        make_history_version_begin_older( T_has_contact::ctx, post_update_hid, notification_is_years_ago, false );

        post_update_contact_data = ::LibFred::InfoContactHistoryByHistoryid( post_update_hid ).exec(T_has_contact::ctx).info_contact_data;
   }
};

template<typename T_has_contact> struct has_set_contact_empty_notifymail : public T_has_contact {
    ::LibFred::InfoContactData post_update_contact_data;

    has_set_contact_empty_notifymail() {
        const unsigned long long post_update_hid =
            ::LibFred::UpdateContactByHandle(T_has_contact::pre_update_contact_data.handle, T_has_contact::registrar.handle)
                .set_notifyemail("")
                .exec(T_has_contact::ctx);

        make_history_version_end_older( T_has_contact::ctx, T_has_contact::pre_update_contact_data.historyid, notification_is_years_ago);
        make_history_version_begin_older( T_has_contact::ctx, post_update_hid, notification_is_years_ago, false );

        post_update_contact_data = ::LibFred::InfoContactHistoryByHistoryid( post_update_hid ).exec(T_has_contact::ctx).info_contact_data;
    }
};

template<typename T_has_contact> struct has_future_with_notifymail : public T_has_contact {
    has_future_with_notifymail() {
        const unsigned long long future_begin_hid =
            ::LibFred::UpdateContactByHandle(T_has_contact::pre_update_contact_data.handle, T_has_contact::registrar.handle)
                .set_email("future.contact.1%nic.cz")
                .set_notifyemail("future.contact.1.notify#nic.cz")
                .exec(T_has_contact::ctx);

        make_history_version_end_older( T_has_contact::ctx, T_has_contact::post_update_contact_data.historyid, future_starts_years_ago);
        make_history_version_begin_older( T_has_contact::ctx, future_begin_hid, future_starts_years_ago, false );
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_contact0, has_future_with_notifymail<has_updated_contact_name<has_contact_with_notify_mail> >)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( post_update_contact_data.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::updated),
            post_update_contact_data.historyid
        ) == email_addresses
    );
}

BOOST_FIXTURE_TEST_CASE(test_updated_contact1, has_future_with_notifymail<has_updated_contact_name<has_contact_without_notify_mail> >)
{
    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::updated),
            post_update_contact_data.historyid
        ).empty()
    );
}

BOOST_FIXTURE_TEST_CASE(test_updated_contact10, has_future_with_notifymail<has_set_contact_notifymail<has_contact_with_notify_mail> >)
{
    std::set<std::string> email_addresses;
    email_addresses.insert( pre_update_contact_data.notifyemail.get_value() );
    email_addresses.insert( post_update_contact_data.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::updated),
            post_update_contact_data.historyid
        ) == email_addresses
    );
}

BOOST_FIXTURE_TEST_CASE(test_updated_contact11, has_future_with_notifymail<has_set_contact_notifymail<has_contact_without_notify_mail> >)
{
    std::set<std::string> email_addresses;
    email_addresses.insert( post_update_contact_data.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::updated),
            post_update_contact_data.historyid
        ) == email_addresses
    );
}

BOOST_FIXTURE_TEST_CASE(test_updated_contact20, has_future_with_notifymail<has_set_contact_empty_notifymail<has_contact_with_notify_mail> >)
{
    std::set<std::string> email_addresses;
    email_addresses.insert( pre_update_contact_data.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::updated),
            post_update_contact_data.historyid
        ) == email_addresses
    );
}

BOOST_FIXTURE_TEST_CASE(test_updated_contact21, has_future_with_notifymail<has_set_contact_empty_notifymail<has_contact_without_notify_mail> >)
{

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::contact, Notification::updated),
            post_update_contact_data.historyid
        ).empty()
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
