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
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(Update)

/* time: -inf ---> history ---> notified_version ---> future ---> +inf */
const unsigned history_starts_years_ago = 4;
const unsigned notified_version_starts_years_ago = 3;
const unsigned notification_is_years_ago = 2;
const unsigned future_starts_years_ago = 1;

struct has_domain : has_autocomitting_ctx {
    ::LibFred::InfoRegistrarData registrar;

    ::LibFred::InfoContactData keyset_tech_c1;
    ::LibFred::InfoContactData keyset_tech_c2;
    ::LibFred::InfoKeysetData keyset;

    ::LibFred::InfoContactData nsset_tech_c1;
    ::LibFred::InfoContactData nsset_tech_c2;
    ::LibFred::InfoNssetData nsset;

    ::LibFred::InfoContactData old_domain_registrant_history_to_be_notified;
    ::LibFred::InfoContactData old_admin_c1_history_to_be_notified;
    ::LibFred::InfoContactData old_admin_c2_history_to_be_notified;

    std::string fqdn;

    has_domain()
    :
        registrar(Test::registrar(ctx).info_data),
        keyset_tech_c1(
            Test::exec(
                ::LibFred::CreateContact("KEYSET1_TECH_C1", registrar.handle)
                    .set_email("keyset1.tech.c.1@.nic.cz")
                    .set_notifyemail("keyset1.tech.c.1.notify@.nic.cz"),
                ctx
            )
        ),
        keyset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("KEYSET1_TECH_C2", registrar.handle)
                    .set_email("keyset1.tech.c.2@.nic.cz")
                    .set_notifyemail("keyset1.tech.c.2.notify@.nic.cz"),
                ctx
            )
        ),
        keyset(
            Test::exec(
                ::LibFred::CreateKeyset("KEYSET1", registrar.handle)
                    .set_tech_contacts(
                        boost::assign::list_of(keyset_tech_c1.handle)(keyset_tech_c2.handle)
                    ),
                ctx
            )
        ),
        nsset_tech_c1(
            Test::exec(
                ::LibFred::CreateContact("NSSET1_TECH_C1", registrar.handle)
                    .set_email("nsset1.tech.c.1@.nic.cz")
                    .set_notifyemail("nsset1.tech.c.1.notify@.nic.cz"),
                ctx
            )
        ),
        nsset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("NSSET1_TECH_C2", registrar.handle)
                    .set_email("nsset1.tech.c.2@.nic.cz")
                    .set_notifyemail("nsset1.tech.c.2.notify@.nic.cz"),
                ctx
            )
        ),
        nsset(
            Test::exec(
                ::LibFred::CreateNsset("NSSET1", registrar.handle)
                    .set_tech_contacts(
                        boost::assign::list_of(nsset_tech_c1.handle)(nsset_tech_c2.handle)
                    ),
                ctx
            )
        )
    {
        make_history_version_begin_older( ctx, keyset.crhistoryid, notified_version_starts_years_ago, false );
        make_history_version_begin_older( ctx, keyset_tech_c1.crhistoryid, notified_version_starts_years_ago, false );
        make_history_version_begin_older( ctx, keyset_tech_c2.crhistoryid, notified_version_starts_years_ago, false );
        make_history_version_begin_older( ctx, nsset.crhistoryid, notified_version_starts_years_ago, false );
        make_history_version_begin_older( ctx, nsset_tech_c1.crhistoryid, notified_version_starts_years_ago, false );
        make_history_version_begin_older( ctx, nsset_tech_c2.crhistoryid, notified_version_starts_years_ago, false );

        const std::string registrant_handle = "REGISTRANT1";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(registrant_handle, registrar.handle)
                        .set_email("history.registrant1@.nic.cz")
                        .set_notifyemail("history.registrant1notify@.nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(registrant_handle, registrar.handle)
                    .set_email("registrant1@.nic.cz")
                    .set_notifyemail("registrant1notify@.nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(registrant_handle, registrar.handle)
                .set_email("future.registrant1@.nic.cz")
                .set_notifyemail("future.registrant1notify@.nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, false );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, false );

            old_domain_registrant_history_to_be_notified = ::LibFred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        const std::string admin_c1_handle = "ADMINC1";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(admin_c1_handle, registrar.handle)
                        .set_email("history.adminc1@nic.cz")
                        .set_notifyemail("history.adminc1notify@nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(admin_c1_handle, registrar.handle)
                    .set_email("adminc1@nic.cz")
                    .set_notifyemail("adminc1notify@nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(admin_c1_handle, registrar.handle)
                .set_email("future.adminc1@nic.cz")
                .set_notifyemail("future.adminc1notify@nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, false );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, false );

            old_admin_c1_history_to_be_notified = ::LibFred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        const std::string admin_c2_handle = "ADMINC2";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(admin_c2_handle, registrar.handle)
                        .set_email("history.adminc2@nic.cz")
                        .set_notifyemail("history.adminc2notify@nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(admin_c2_handle, registrar.handle)
                    .set_email("adminc2@nic.cz")
                    .set_notifyemail("adminc2notify@nic.cz")
                    .exec(ctx);

            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(admin_c2_handle, registrar.handle)
                .set_email("future.adminc2@nic.cz")
                .set_notifyemail("future.adminc2notify@nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, false );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid, future_starts_years_ago, false );

            old_admin_c2_history_to_be_notified = ::LibFred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        fqdn = "mydomain123.cz";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateDomain(fqdn, registrar.handle, old_domain_registrant_history_to_be_notified.handle )
                        .set_admin_contacts(
                            boost::assign::list_of(old_admin_c1_history_to_be_notified.handle)(old_admin_c2_history_to_be_notified.handle)
                        )
                        .set_keyset(keyset.handle)
                        .set_nsset(nsset.handle),
                    ctx
                ).historyid;
            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
        }
    }
};

struct has_updated_domain_followed_by_future_changes {
    ::LibFred::InfoDomainData dom_data_to_be_notified;

    has_updated_domain_followed_by_future_changes(
        const std::string _fqdn,
        const std::string _registrar_handle,
        ::LibFred::UpdateDomain& _update,
        ::LibFred::OperationContext& _ctx
    ) {
        const unsigned long long to_be_notified_hid = _update.exec(_ctx);

        /* future */
        const std::string different_registrant_handle = "DIFFREGISTRANT";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(different_registrant_handle, _registrar_handle)
                        .set_email("different.registrant@.nic.cz")
                        .set_notifyemail("different.registrant.notify@.nic.cz"),
                    _ctx
                ).historyid;

            make_history_version_begin_older( _ctx, crhid, history_starts_years_ago, true );
        }

        const std::string different_admin_handle = "DIFFADMINC";
        {
            const unsigned long long crhid =
                Test::exec(
                    ::LibFred::CreateContact(different_admin_handle, _registrar_handle)
                        .set_email("different.adminc@nic.cz")
                        .set_notifyemail("different.adminc1notify@nic.cz"),
                    _ctx
                ).historyid;

            make_history_version_begin_older( _ctx, crhid, history_starts_years_ago, true );
        }


        ::LibFred::UpdateDomain future_update(_fqdn, _registrar_handle);

        future_update
            .set_registrant(different_registrant_handle)
            .add_admin_contact(different_admin_handle);

        for (const auto& a_c : ::LibFred::InfoDomainByFqdn(_fqdn).exec(_ctx).info_domain_data.admin_contacts)
        {
            future_update.rem_admin_contact(a_c.handle);
        }

        const unsigned long long future_hid = future_update.exec(_ctx);

        make_history_version_end_older( _ctx, ::LibFred::InfoDomainByFqdn(_fqdn).exec(_ctx).info_domain_data.crhistoryid, notification_is_years_ago );
        make_history_version_begin_older( _ctx, to_be_notified_hid, notification_is_years_ago, false );

        make_history_version_end_older( _ctx, to_be_notified_hid, future_starts_years_ago );
        make_history_version_begin_older( _ctx, future_hid, future_starts_years_ago, false );

        dom_data_to_be_notified = ::LibFred::InfoDomainHistoryByHistoryid(to_be_notified_hid).exec(_ctx).info_domain_data;
    }
};

struct has_domain_changed_authinfo : has_domain, has_updated_domain_followed_by_future_changes {

    has_domain_changed_authinfo()
    :   has_domain(),
        has_updated_domain_followed_by_future_changes(
            fqdn,
            registrar.handle,
            ::LibFred::UpdateDomain(fqdn, registrar.handle).set_authinfo("4sadf4sa34sf64as3f54as3f4g5"),
            ctx
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_updated_domain_authinfo, has_domain_changed_authinfo)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( old_domain_registrant_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::domain, Notification::updated),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_domain_removed_admin_c : has_domain, has_updated_domain_followed_by_future_changes {

    has_domain_removed_admin_c()
    :   has_updated_domain_followed_by_future_changes(
            fqdn,
            registrar.handle,
            ::LibFred::UpdateDomain(fqdn, registrar.handle)
                .rem_admin_contact(old_admin_c1_history_to_be_notified.handle),
            ctx
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_updated_domain_removed_admin_contact, has_domain_removed_admin_c)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( old_domain_registrant_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::domain, Notification::updated),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_domain_added_admin_c : has_domain {

    const ::LibFred::InfoContactData added_admin_c;
    ::LibFred::InfoDomainData dom_data_to_be_notified;

    has_domain_added_admin_c()
    :   added_admin_c(
            Test::exec(
                ::LibFred::CreateContact("ADMINC_NEW1", registrar.handle)
                    .set_email("new_adminc_1@nic.cz")
                    .set_notifyemail("new_adminc_1notify@nic.cz"),
                ctx
            )
        )
    {
        make_history_version_begin_older( ctx, added_admin_c.crhistoryid, history_starts_years_ago, true );

        dom_data_to_be_notified =
            has_updated_domain_followed_by_future_changes(
                fqdn,
                registrar.handle,
                ::LibFred::UpdateDomain(fqdn, registrar.handle)
                    .add_admin_contact(added_admin_c.handle),
                ctx
            ).dom_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_domain_added_admin_contact, has_domain_added_admin_c)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( old_domain_registrant_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( added_admin_c.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::domain, Notification::updated),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_domain_changed_admin_c : has_domain {

    const ::LibFred::InfoContactData added_admin_c;
    const ::LibFred::InfoContactData& removed_admin_c;
    ::LibFred::InfoDomainData dom_data_to_be_notified;

    has_domain_changed_admin_c()
    :   added_admin_c(
            Test::exec(
                ::LibFred::CreateContact("ADMINC_NEW1", registrar.handle)
                    .set_email("adminc_new1@nic.cz")
                    .set_notifyemail("adminc_new1notify@nic.cz"),
                ctx
            )
        ),
        removed_admin_c(old_admin_c1_history_to_be_notified)

    {
        make_history_version_begin_older( ctx, added_admin_c.crhistoryid, history_starts_years_ago, true );

        dom_data_to_be_notified =
            has_updated_domain_followed_by_future_changes(
                fqdn,
                registrar.handle,
                ::LibFred::UpdateDomain(fqdn, registrar.handle)
                    .rem_admin_contact(removed_admin_c.handle)
                    .add_admin_contact(added_admin_c.handle),
                ctx
            ).dom_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_domain_changed_admin_contact, has_domain_changed_admin_c)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( old_domain_registrant_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( added_admin_c.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::domain, Notification::updated),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_domain_changed_registrant : has_domain {

    const ::LibFred::InfoContactData new_registrant;
    const ::LibFred::InfoContactData& previous_registrant;
    ::LibFred::InfoDomainData dom_data_to_be_notified;

    has_domain_changed_registrant()
    :   new_registrant(
            Test::exec(
                ::LibFred::CreateContact("REGISTRANT_NEW1", registrar.handle)
                    .set_email("registrant_new1@nic.cz")
                    .set_notifyemail("registrant_new1notify@nic.cz"),
                ctx
            )
        ),
        previous_registrant(old_domain_registrant_history_to_be_notified)
    {
        make_history_version_begin_older( ctx, new_registrant.crhistoryid, history_starts_years_ago, true );

        dom_data_to_be_notified =
            has_updated_domain_followed_by_future_changes(
                fqdn,
                registrar.handle,
                ::LibFred::UpdateDomain(fqdn, registrar.handle)
                    .set_registrant(new_registrant.handle),
                ctx
            ).dom_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_domain_changed_registrant, has_domain_changed_registrant)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( previous_registrant.notifyemail.get_value() );
    email_addresses.insert( new_registrant.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::domain, Notification::updated),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_domain_changed_keyset : has_domain {

    const ::LibFred::InfoContactData new_keyset_tech_c1;
    const ::LibFred::InfoContactData new_keyset_tech_c2;
    const ::LibFred::InfoKeysetData new_keyset;
    ::LibFred::InfoDomainData dom_data_to_be_notified;

    has_domain_changed_keyset()
    :   new_keyset_tech_c1(
            Test::exec(
                ::LibFred::CreateContact("NEW_KEYSET_TECH_C1", registrar.handle)
                    .set_email("new_keyset.tech.c.1@.nic.cz")
                    .set_notifyemail("new_keyset.tech.c.1.notify@.nic.cz"),
                ctx
            )
        ),
        new_keyset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("NEW_KEYSET_TECH_C2", registrar.handle)
                    .set_email("new_keyset.tech.c.2@.nic.cz")
                    .set_notifyemail("new_keyset.tech.c.2.notify@.nic.cz"),
                ctx
            )
        ),
        new_keyset(
            Test::exec(
                ::LibFred::CreateKeyset("NEW_KEYSET", registrar.handle)
                    .set_tech_contacts(
                        boost::assign::list_of(new_keyset_tech_c1.handle)(new_keyset_tech_c2.handle)
                    ),
                ctx
            )
        )
    {
        make_history_version_begin_older( ctx, new_keyset_tech_c1.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_keyset_tech_c2.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_keyset.crhistoryid, history_starts_years_ago, true );

        dom_data_to_be_notified =
            has_updated_domain_followed_by_future_changes(
                fqdn,
                registrar.handle,
                ::LibFred::UpdateDomain(fqdn, registrar.handle)
                    .set_keyset(new_keyset.handle),
                ctx
            ).dom_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_domain_changed_keyset, has_domain_changed_keyset)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( old_domain_registrant_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( keyset_tech_c1.notifyemail.get_value() );
    email_addresses.insert( keyset_tech_c2.notifyemail.get_value() );
    email_addresses.insert( new_keyset_tech_c1.notifyemail.get_value() );
    email_addresses.insert( new_keyset_tech_c2.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::domain, Notification::updated),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

struct has_domain_changed_nsset : has_domain {

    const ::LibFred::InfoContactData new_nsset_tech_c1;
    const ::LibFred::InfoContactData new_nsset_tech_c2;
    const ::LibFred::InfoNssetData new_nsset;
    ::LibFred::InfoDomainData dom_data_to_be_notified;

    has_domain_changed_nsset()
    :   new_nsset_tech_c1(
            Test::exec(
                ::LibFred::CreateContact("NEW_NSSET_TECH_C1", registrar.handle)
                    .set_email("new_nsset.tech.c.1@.nic.cz")
                    .set_notifyemail("new_nsset.tech.c.1.notify@.nic.cz"),
                ctx
            )
        ),
        new_nsset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("NEW_NSSET_TECH_C2", registrar.handle)
                    .set_email("new_nsset.tech.c.2@.nic.cz")
                    .set_notifyemail("new_nsset.tech.c.2.notify@.nic.cz"),
                ctx
            )
        ),
        new_nsset(
            Test::exec(
                ::LibFred::CreateNsset("NEW_NSSET", registrar.handle)
                    .set_tech_contacts(
                        boost::assign::list_of(new_nsset_tech_c1.handle)(new_nsset_tech_c2.handle)
                    ),
                ctx
            )
        )
    {
        make_history_version_begin_older( ctx, new_nsset_tech_c1.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_nsset_tech_c2.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_nsset.crhistoryid, history_starts_years_ago, true );

        dom_data_to_be_notified =
            has_updated_domain_followed_by_future_changes(
                fqdn,
                registrar.handle,
                ::LibFred::UpdateDomain(fqdn, registrar.handle)
                    .set_nsset(new_nsset.handle),
                ctx
            ).dom_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_updated_domain_changed_nsset, has_domain_changed_nsset)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( old_domain_registrant_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( nsset_tech_c1.notifyemail.get_value() );
    email_addresses.insert( nsset_tech_c2.notifyemail.get_value() );
    email_addresses.insert( new_nsset_tech_c1.notifyemail.get_value() );
    email_addresses.insert( new_nsset_tech_c2.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(::LibFred::domain, Notification::updated),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

/* TODO (jakmiel to bude naimplementovano) tady budou testy pro zmenu nssetu/keysetu a "soucasnou" zmenu jejich technickych kontaktu - aby bylo videt, ze se notifikuji spravne historicke verze*/

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
