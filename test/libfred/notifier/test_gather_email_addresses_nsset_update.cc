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

#include <boost/foreach.hpp>


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailAddresses)
BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(Update)

/* time: -inf ---> history ---> notified_version ---> future ---> +inf */
const unsigned history_starts_years_ago = 4;
const unsigned notified_version_starts_years_ago = 3;
const unsigned notification_is_years_ago = 2;
const unsigned future_starts_years_ago = 1;

struct has_nsset : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const std::string nsset_handle;

    ::LibFred::InfoContactData nsset_tech_c1_to_be_notified;
    ::LibFred::InfoContactData nsset_tech_c2_to_be_notified;
    has_nsset()
    :
        registrar(Test::registrar(ctx).info_data),
        nsset_handle("NSSET1")
    {
        const std::string tech_c1_handle = "NSSET1_TECH_C1";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(tech_c1_handle, registrar.handle)
                        .set_email("history.nsset1.tech.c.1@.nic.cz")
                        .set_notifyemail("history.nsset1.tech.c.1.notify@.nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(tech_c1_handle, registrar.handle)
                    .set_email("nsset1.tech.c.1@.nic.cz")
                    .set_notifyemail("nsset1.tech.c.1.notify@.nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(tech_c1_handle, registrar.handle)
                .set_email("future.nsset1.tech.c.1@.nic.cz")
                .set_notifyemail("future.nsset1.tech.c.1.notify@.nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, true );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, true );

            nsset_tech_c1_to_be_notified = ::LibFred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        const std::string tech_c2_handle = "NSSET2_TECH_C2";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(tech_c2_handle, registrar.handle)
                        .set_email("history.nsset2.tech.c.2@.nic.cz")
                        .set_notifyemail("history.nsset2.tech.c.2.notify@.nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(tech_c2_handle, registrar.handle)
                    .set_email("nsset2.tech.c.2@.nic.cz")
                    .set_notifyemail("nsset2.tech.c.2.notify@.nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(tech_c2_handle, registrar.handle)
                .set_email("future.nsset2.tech.c.2@.nic.cz")
                .set_notifyemail("future.nsset2.tech.c.2.notify@.nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, true );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, true );

            nsset_tech_c2_to_be_notified = ::LibFred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateNsset(nsset_handle, registrar.handle)
                        .set_tech_contacts(
                            boost::assign::list_of(tech_c1_handle)(tech_c2_handle)
                        ),
                    ctx
                ).crhistoryid;
            make_history_version_begin_older( ctx, crhid, notification_is_years_ago, true );
        }
    }
};

struct has_updated_nsset_followed_by_future_changes {
    ::LibFred::InfoNssetData nsset_data_to_be_notified;

    has_updated_nsset_followed_by_future_changes(
        const std::string _handle,
        const std::string _registrar_handle,
        ::LibFred::UpdateNsset& _update,
        ::LibFred::OperationContext& _ctx
    ) {
        const unsigned long long to_be_notified_hid = _update.exec(_ctx);

        /* future */
        const std::string different_tech_c_handle = "DIFFTECHC";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(different_tech_c_handle, _registrar_handle)
                        .set_email("different.adminc@nic.cz")
                        .set_notifyemail("different.adminc1notify@nic.cz"),
                    _ctx
                ).historyid;

            make_history_version_begin_older( _ctx, crhid, history_starts_years_ago, true );
        }

        ::LibFred::UpdateNsset future_update(_handle, _registrar_handle);
        future_update.add_tech_contact(different_tech_c_handle);

        for (const auto& a_c : ::LibFred::InfoNssetByHandle(_handle).exec(_ctx).info_nsset_data.tech_contacts)
        {
            future_update.rem_tech_contact(a_c.handle);
        }

        const unsigned long long future_hid = future_update.exec(_ctx);

        make_history_version_end_older( _ctx, ::LibFred::InfoNssetByHandle(_handle).exec(_ctx).info_nsset_data.crhistoryid, notification_is_years_ago );
        make_history_version_begin_older( _ctx, to_be_notified_hid, notification_is_years_ago, false );

        make_history_version_end_older( _ctx, to_be_notified_hid, future_starts_years_ago );
        make_history_version_begin_older( _ctx, future_hid, future_starts_years_ago, false );

        nsset_data_to_be_notified = ::LibFred::InfoNssetHistoryByHistoryid(to_be_notified_hid).exec(_ctx).info_nsset_data;
    }
};

struct has_nsset_changed_authinfo : has_nsset {
    private:
        const ::LibFred::InfoNssetData nsset_historical_data;
    public:
    const std::string new_authinfo;
    ::LibFred::InfoNssetData nsset_data_to_be_notified;

    has_nsset_changed_authinfo()
    :
        has_nsset(),
        nsset_historical_data( ::LibFred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data ),
        new_authinfo( nsset_historical_data.authinfopw + "xyz" )
    {
        nsset_data_to_be_notified = has_updated_nsset_followed_by_future_changes(
            nsset_handle,
            registrar.handle,
            ::LibFred::UpdateNsset(nsset_handle, nsset_historical_data.create_registrar_handle).set_authinfo(new_authinfo),
            ctx
        ).nsset_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_nsset_authinfo, has_nsset_changed_authinfo)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( nsset_tech_c1_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( nsset_tech_c2_to_be_notified.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::nsset, Notification::updated),
            nsset_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_nsset_removed_tech_c : has_nsset {
    private:
        const ::LibFred::InfoNssetData nsset_historical_data;
    public:
    ::LibFred::InfoNssetData nsset_data_to_be_notified;

    has_nsset_removed_tech_c()
    :
        has_nsset(),
        nsset_historical_data( ::LibFred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data )
    {
        nsset_data_to_be_notified = has_updated_nsset_followed_by_future_changes(
            nsset_handle,
            registrar.handle,
            ::LibFred::UpdateNsset(nsset_handle, nsset_historical_data.create_registrar_handle).rem_tech_contact(nsset_historical_data.tech_contacts.begin()->handle),
            ctx
        ).nsset_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_nsset_removed_admin_contact, has_nsset_removed_tech_c)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( nsset_tech_c1_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( nsset_tech_c2_to_be_notified.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::nsset, Notification::updated),
            nsset_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_nsset_added_tech_c : has_nsset {
    private:
        const ::LibFred::InfoNssetData nsset_historical_data;
    public:
    const ::LibFred::InfoContactData added_tech_c;
    ::LibFred::InfoNssetData nsset_data_to_be_notified;

    has_nsset_added_tech_c()
    :
        has_nsset(),
        nsset_historical_data( ::LibFred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data ),
        added_tech_c(
            Test::exec(
                ::LibFred::CreateContact("TECHC_NEW1", registrar.handle)
                    .set_email("techc_new1@nic.cz")
                    .set_notifyemail("techc_new1notify@nic.cz"),
                ctx
            )
        )
    {
        make_history_version_begin_older( ctx, added_tech_c.historyid, history_starts_years_ago, true );

        nsset_data_to_be_notified = has_updated_nsset_followed_by_future_changes(
            nsset_handle,
            registrar.handle,
            ::LibFred::UpdateNsset(nsset_handle, nsset_historical_data.create_registrar_handle).add_tech_contact(added_tech_c.handle),
            ctx
        ).nsset_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_nsset_added_admin_contact, has_nsset_added_tech_c)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( nsset_tech_c1_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( nsset_tech_c2_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( added_tech_c.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::nsset, Notification::updated),
            nsset_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_nsset_changed_tech_c : has_nsset {
    private:
        const ::LibFred::InfoNssetData nsset_historical_data;
    public:
    const ::LibFred::InfoContactData added_tech_c;
    ::LibFred::InfoNssetData nsset_data_to_be_notified;

    has_nsset_changed_tech_c()
    :
        has_nsset(),
        nsset_historical_data( ::LibFred::InfoNssetByHandle(nsset_handle).exec(ctx).info_nsset_data ),
        added_tech_c(
            Test::exec(
                ::LibFred::CreateContact("TECHC_NEW1", registrar.handle)
                    .set_email("techc_new1@nic.cz")
                    .set_notifyemail("techc_new1notify@nic.cz"),
                ctx
            )
        )
    {
        make_history_version_begin_older( ctx, added_tech_c.historyid, history_starts_years_ago, true );

        nsset_data_to_be_notified = has_updated_nsset_followed_by_future_changes(
            nsset_handle,
            registrar.handle,
            ::LibFred::UpdateNsset(nsset_handle, nsset_historical_data.create_registrar_handle)
                .add_tech_contact(added_tech_c.handle)
                .rem_tech_contact(nsset_historical_data.tech_contacts.begin()->handle),
            ctx
        ).nsset_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_nsset_changed_admin_contact, has_nsset_changed_tech_c)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( nsset_tech_c1_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( nsset_tech_c2_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( added_tech_c.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::nsset, Notification::updated),
            nsset_data_to_be_notified.historyid
        ) == email_addresses
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
