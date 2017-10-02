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
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/notifier/util.h"

#include "src/fredlib/notifier/gather_email_data/gather_email_addresses.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/transfer_domain.h"

#include <boost/foreach.hpp>


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailAddresses)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(Transfer)

/* time: -inf ---> history ---> notified_version ---> future ---> +inf */
const unsigned history_starts_years_ago = 4;
const unsigned notified_version_starts_years_ago = 3;
const unsigned notification_is_years_ago = 2;
const unsigned future_starts_years_ago = 1;

struct has_domain : has_autocomitting_ctx {
    Fred::InfoRegistrarData registrar;

    Fred::InfoContactData keyset_tech_c1;
    Fred::InfoContactData keyset_tech_c2;
    Fred::InfoKeysetData keyset;

    Fred::InfoContactData nsset_tech_c1;
    Fred::InfoContactData nsset_tech_c2;
    Fred::InfoNssetData nsset;

    Fred::InfoContactData old_domain_registrant_history_to_be_notified;
    Fred::InfoContactData old_admin_c1_history_to_be_notified;
    Fred::InfoContactData old_admin_c2_history_to_be_notified;

    std::string fqdn;

    has_domain()
    :
        registrar(Test::registrar(ctx).info_data),
        keyset_tech_c1(
            Test::exec(
                Fred::CreateContact("KEYSET1_TECH_C1", registrar.handle)
                    .set_email("keyset1.tech.c.1@.nic.cz")
                    .set_notifyemail("keyset1.tech.c.1.notify@.nic.cz"),
                ctx
            )
        ),
        keyset_tech_c2(
            Test::exec(
                Fred::CreateContact("KEYSET1_TECH_C2", registrar.handle)
                    .set_email("keyset1.tech.c.2@.nic.cz")
                    .set_notifyemail("keyset1.tech.c.2.notify@.nic.cz"),
                ctx
            )
        ),
        keyset(
            Test::exec(
                Fred::CreateKeyset("KEYSET1", registrar.handle)
                    .set_tech_contacts(
                        boost::assign::list_of(keyset_tech_c1.handle)(keyset_tech_c2.handle)
                    ),
                ctx
            )
        ),
        nsset_tech_c1(
            Test::exec(
                Fred::CreateContact("NSSET1_TECH_C1", registrar.handle)
                    .set_email("nsset1.tech.c.1@.nic.cz")
                    .set_notifyemail("nsset1.tech.c.1.notify@.nic.cz"),
                ctx
            )
        ),
        nsset_tech_c2(
            Test::exec(
                Fred::CreateContact("NSSET1_TECH_C2", registrar.handle)
                    .set_email("nsset1.tech.c.2@.nic.cz")
                    .set_notifyemail("nsset1.tech.c.2.notify@.nic.cz"),
                ctx
            )
        ),
        nsset(
            Test::exec(
                Fred::CreateNsset("NSSET1", registrar.handle)
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
                    Fred::CreateContact(registrant_handle, registrar.handle)
                        .set_email("history.registrant1@.nic.cz")
                        .set_notifyemail("history.registrant1notify@.nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                Fred::UpdateContactByHandle(registrant_handle, registrar.handle)
                    .set_email("registrant1@.nic.cz")
                    .set_notifyemail("registrant1notify@.nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = Fred::UpdateContactByHandle(registrant_handle, registrar.handle)
                .set_email("future.registrant1@.nic.cz")
                .set_notifyemail("future.registrant1notify@.nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, false );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, false );

            old_domain_registrant_history_to_be_notified = Fred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        const std::string admin_c1_handle = "ADMINC1";
        {
            const unsigned long long crhid =
                Test::exec(
                    Fred::CreateContact(admin_c1_handle, registrar.handle)
                        .set_email("history.adminc1@nic.cz")
                        .set_notifyemail("history.adminc1notify@nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                Fred::UpdateContactByHandle(admin_c1_handle, registrar.handle)
                    .set_email("adminc1@nic.cz")
                    .set_notifyemail("adminc1notify@nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = Fred::UpdateContactByHandle(admin_c1_handle, registrar.handle)
                .set_email("future.adminc1@nic.cz")
                .set_notifyemail("future.adminc1notify@nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, false );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid , future_starts_years_ago, false );

            old_admin_c1_history_to_be_notified = Fred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        const std::string admin_c2_handle = "ADMINC2";
        {
            const unsigned long long crhid =
                Test::exec(
                    Fred::CreateContact(admin_c2_handle, registrar.handle)
                        .set_email("history.adminc2@nic.cz")
                        .set_notifyemail("history.adminc2notify@nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                Fred::UpdateContactByHandle(admin_c2_handle, registrar.handle)
                    .set_email("adminc2@nic.cz")
                    .set_notifyemail("adminc2notify@nic.cz")
                    .exec(ctx);

            const unsigned long long future_begin_hid = Fred::UpdateContactByHandle(admin_c2_handle, registrar.handle)
                .set_email("future.adminc2@nic.cz")
                .set_notifyemail("future.adminc2notify@nic.cz")
                .exec(ctx);

            make_history_version_begin_older( ctx, crhid, history_starts_years_ago, true );
            make_history_version_end_older( ctx, crhid, notified_version_starts_years_ago);

            make_history_version_begin_older( ctx, to_be_notified_hid, notified_version_starts_years_ago, false );
            make_history_version_end_older( ctx, to_be_notified_hid, future_starts_years_ago);

            make_history_version_begin_older( ctx, future_begin_hid, future_starts_years_ago, false );

            old_admin_c2_history_to_be_notified = Fred::InfoContactHistoryByHistoryid(to_be_notified_hid).exec(ctx).info_contact_data;
        }

        fqdn = "mydomain123.cz";
        {
            const unsigned long long crhid =
                Test::exec(
                    Fred::CreateDomain(fqdn, registrar.handle, old_domain_registrant_history_to_be_notified.handle )
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

template<typename Tdomainoperation = Fred::UpdateDomain> struct has_domain_operation_followed_by_future_changes {
    Fred::InfoDomainData dom_data_to_be_notified;

    has_domain_operation_followed_by_future_changes(
        const std::string _fqdn,
        const std::string _registrar_handle,
        Tdomainoperation& _operation,
        Fred::OperationContext& _ctx
    ) {
        const unsigned long long to_be_notified_hid = _operation.exec(_ctx);

        /* future */
        const std::string different_registrant_handle = "DIFFREGISTRANT";
        {
            const unsigned long long crhid =
                Test::exec(
                    Fred::CreateContact(different_registrant_handle, _registrar_handle)
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
                    Fred::CreateContact(different_admin_handle, _registrar_handle)
                        .set_email("different.adminc@nic.cz")
                        .set_notifyemail("different.adminc1notify@nic.cz"),
                    _ctx
                ).historyid;

            make_history_version_begin_older( _ctx, crhid, history_starts_years_ago, true );
        }


            Fred::UpdateDomain future_update(_fqdn, _registrar_handle);

            future_update
                .set_registrant(different_registrant_handle)
                .add_admin_contact(different_admin_handle);

            BOOST_FOREACH(
                const Fred::ObjectIdHandlePair& a_c,
                Fred::InfoDomainByFqdn(_fqdn).exec(_ctx).info_domain_data.admin_contacts
            ) {
                future_update.rem_admin_contact(a_c.handle);
            }

            const unsigned long long future_hid = future_update.exec(_ctx);

        make_history_version_end_older( _ctx, Fred::InfoDomainByFqdn(_fqdn).exec(_ctx).info_domain_data.crhistoryid, notification_is_years_ago );
        make_history_version_begin_older( _ctx, to_be_notified_hid, notification_is_years_ago, false );

        make_history_version_end_older( _ctx, to_be_notified_hid, future_starts_years_ago );
        make_history_version_begin_older( _ctx, future_hid, future_starts_years_ago, false );

        dom_data_to_be_notified = Fred::InfoDomainHistoryByHistoryid(to_be_notified_hid).exec(_ctx).info_domain_data;
    }
};

struct has_transferred_domain : has_domain {
    const Fred::InfoRegistrarData new_registrar;
    Fred::InfoDomainData dom_data_to_be_notified;

    has_transferred_domain()
    :
        has_domain(),
        new_registrar(Test::registrar(ctx).info_data)
    {
        const Fred::InfoDomainData domain_data = Fred::InfoDomainByFqdn(fqdn).exec(ctx).info_domain_data;
        Fred::TransferDomain transfer(domain_data.id, new_registrar.handle, domain_data.authinfopw, Nullable<unsigned long long>());

        dom_data_to_be_notified = has_domain_operation_followed_by_future_changes<Fred::TransferDomain>(
            fqdn,
            registrar.handle,
            transfer,
            ctx
        ).dom_data_to_be_notified;
    }
};

BOOST_FIXTURE_TEST_CASE(test_transferred_domain, has_transferred_domain)
{
    std::set<std::string> email_addresses;

    email_addresses.insert( old_domain_registrant_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c1_history_to_be_notified.notifyemail.get_value() );
    email_addresses.insert( old_admin_c2_history_to_be_notified.notifyemail.get_value() );

    BOOST_CHECK(
        Notification::gather_email_addresses(
            ctx,
            Notification::EventOnObject(Fred::domain, Notification::transferred),
            dom_data_to_be_notified.historyid
        ) == email_addresses
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
