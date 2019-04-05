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

#include "libfred/notifier/process_one_notification_request.hh"
#include "libfred/notifier/enqueue_notification.hh"

#include "src/bin/corba/mailer_manager.hh"

#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(SendNotification)

struct MockMailerManager : public ::LibFred::Mailer::Manager {
    struct MailData {
        std::string from;
        std::string to;
        std::string subject;
        std::string mailTemplate;
        ::LibFred::Mailer::Parameters params;
        ::LibFred::Mailer::Handles handles;
        ::LibFred::Mailer::Attachments attach;
        std::string reply_to;

        MailData(
            const std::string& _from,
            const std::string& _to,
            const std::string& _subject,
            const std::string& _mailTemplate,
            const ::LibFred::Mailer::Parameters& _params,
            const ::LibFred::Mailer::Handles& _handles,
            const ::LibFred::Mailer::Attachments& _attach,
            const std::string& _reply_to
        ) :
            from(_from),
            to(_to),
            subject(_subject),
            mailTemplate(_mailTemplate),
            params(_params),
            handles(_handles),
            attach(_attach),
            reply_to(_reply_to)
        { }
    };

    std::vector<MailData> accumulated_data;

    virtual unsigned long long sendEmail(
      const std::string& _from,
      const std::string& _to,
      const std::string& _subject,
      const std::string& _mailTemplate,
      const ::LibFred::Mailer::Parameters& _params,
      const ::LibFred::Mailer::Handles& _handles,
      const ::LibFred::Mailer::Attachments& _attach,
      const std::string& _reply_to = std::string("")

    ) {
        accumulated_data.push_back(
            MailData(
                _from,
                _to,
                _subject,
                _mailTemplate,
                _params,
                _handles,
                _attach,
                _reply_to
            )
        );

        return 0;
    }

    virtual bool checkEmailList(std::string &_email_list) const { return true; }
};

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
                    .set_email("keyset1.tech.c.1@nic.cz")
                    .set_notifyemail("keyset1.tech.c.1.notify@nic.cz"),
                ctx
            )
        ),
        keyset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("KEYSET1_TECH_C2", registrar.handle)
                    .set_email("keyset1.tech.c.2@nic.cz")
                    .set_notifyemail("keyset1.tech.c.2.notify@nic.cz"),
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
                    .set_email("nsset1.tech.c.1@nic.cz")
                    .set_notifyemail("nsset1.tech.c.1.notify@nic.cz"),
                ctx
            )
        ),
        nsset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("NSSET1_TECH_C2", registrar.handle)
                    .set_email("nsset1.tech.c.2@nic.cz")
                    .set_notifyemail("nsset1.tech.c.2.notify@nic.cz"),
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
                        .set_email("history.registrant1@nic.cz")
                        .set_notifyemail("history.registrant1notify@.nic.cz"),
                    ctx
                ).historyid;
            const unsigned long long to_be_notified_hid =
                ::LibFred::UpdateContactByHandle(registrant_handle, registrar.handle)
                    .set_email("registrant1@.nic.cz")
                    .set_notifyemail("registrant1notify@nic.cz")
                    .exec(ctx);
            const unsigned long long future_begin_hid = ::LibFred::UpdateContactByHandle(registrant_handle, registrar.handle)
                .set_email("future.registrant1@.nic.cz")
                .set_notifyemail("future.registrant1notify@nic.cz")
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
    ::LibFred::InfoDomainData domain_data_post_update;

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
                        .set_email("different.registrant@nic.cz")
                        .set_notifyemail("different.registrant.notify@nic.cz"),
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

        domain_data_post_update = ::LibFred::InfoDomainHistoryByHistoryid(to_be_notified_hid).exec(_ctx).info_domain_data;
    }
};

struct has_domain_big_update : has_domain {

    const ::LibFred::InfoContactData new_registrant;

    const ::LibFred::InfoContactData added_admin_c;

    const ::LibFred::InfoContactData new_nsset_tech_c1;
    const ::LibFred::InfoContactData new_nsset_tech_c2;
    const ::LibFred::InfoNssetData new_nsset;

    const ::LibFred::InfoContactData new_keyset_tech_c1;
    const ::LibFred::InfoContactData new_keyset_tech_c2;
    const ::LibFred::InfoKeysetData new_keyset;

    ::LibFred::InfoDomainData domain_data_pre_update;
    ::LibFred::InfoDomainData domain_data_post_update;

    const std::string input_svtrid;

    has_domain_big_update()
    :   new_registrant(
            Test::exec(
                ::LibFred::CreateContact("REGISTRANT_NEW1", registrar.handle)
                    .set_email("registrant_new1@nic.cz")
                    .set_notifyemail("registrant_new1notify@nic.cz"),
                ctx
            )
        ),
        added_admin_c(
            Test::exec(
                ::LibFred::CreateContact("ADMINC_NEW1", registrar.handle)
                    .set_email("adminc_new1@nic.cz")
                    .set_notifyemail("adminc_new1notify@nic.cz"),
                ctx
            )
        ),
        new_nsset_tech_c1(
            Test::exec(
                ::LibFred::CreateContact("NEW_NSSET_TECH_C1", registrar.handle)
                    .set_email("new_nsset.tech.c.1@nic.cz")
                    .set_notifyemail("new_nsset.tech.c.1.notify@.nic.cz"),
                ctx
            )
        ),
        new_nsset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("NEW_NSSET_TECH_C2", registrar.handle)
                    .set_email("new_nsset.tech.c.2@nic.cz")
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
        ),
        new_keyset_tech_c1(
            Test::exec(
                ::LibFred::CreateContact("NEW_KEYSET_TECH_C1", registrar.handle)
                    .set_email("new_keyset.tech.c.1@nic.cz")
                    .set_notifyemail("new_keyset.tech.c.1.notify@nic.cz"),
                ctx
            )
        ),
        new_keyset_tech_c2(
            Test::exec(
                ::LibFred::CreateContact("NEW_KEYSET_TECH_C2", registrar.handle)
                    .set_email("new_keyset.tech.c.2@nic.cz")
                    .set_notifyemail("new_keyset.tech.c.2.notify@nic.cz"),
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
        ),
        input_svtrid("abc-123")
    {
        /* assuming ::LibFred::InfoDomainHistory::exec() result ordering - implementation does so, documentation is probably missing */
        domain_data_pre_update = ::LibFred::InfoDomainByFqdn(fqdn).exec(ctx).info_domain_data;

        make_history_version_begin_older( ctx, new_registrant.crhistoryid, history_starts_years_ago, true );

        make_history_version_begin_older( ctx, added_admin_c.crhistoryid, history_starts_years_ago, true );

        make_history_version_begin_older( ctx, new_nsset_tech_c1.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_nsset_tech_c2.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_nsset.crhistoryid, history_starts_years_ago, true );

        make_history_version_begin_older( ctx, new_keyset_tech_c1.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_keyset_tech_c2.crhistoryid, history_starts_years_ago, true );
        make_history_version_begin_older( ctx, new_keyset.crhistoryid, history_starts_years_ago, true );

        domain_data_post_update =
            has_updated_domain_followed_by_future_changes(
                fqdn,
                registrar.handle,
                ::LibFred::UpdateDomain(fqdn, registrar.handle)
                    .set_authinfo("abc123zxyfdasf35534")
                    .set_registrant(new_registrant.handle)
                    .add_admin_contact(added_admin_c.handle)
                    .set_nsset(new_nsset.handle)
                    .set_keyset(new_keyset.handle),
                ctx
            ).domain_data_post_update;

        Notification::enqueue_notification(
            ctx,
            Notification::updated,
            registrar.id,
            domain_data_post_update.historyid,
            input_svtrid
        );
    }
};

BOOST_FIXTURE_TEST_CASE(test_process_update_domain, has_domain_big_update)
{
    std::shared_ptr<::LibFred::Mailer::Manager> mocked_mailer(new MockMailerManager);
    MockMailerManager& mocked_mailer_data_access = *reinterpret_cast<MockMailerManager*>( mocked_mailer.get() );

    BOOST_CHECK_EQUAL(
        Notification::process_one_notification_request(ctx, mocked_mailer),
        true
    );

    BOOST_CHECK_EQUAL(mocked_mailer_data_access.accumulated_data.size(), 13);

    {
        std::set<std::string> found_email_addresses;
        BOOST_FOREACH(const MockMailerManager::MailData& mail, mocked_mailer_data_access.accumulated_data) {
            found_email_addresses.insert(mail.to);
        }

        const std::set<std::string> etalon_email_addresses =
            boost::assign::list_of
                (old_domain_registrant_history_to_be_notified.notifyemail.get_value())
                (new_registrant.notifyemail.get_value())

                (old_admin_c1_history_to_be_notified.notifyemail.get_value())
                (old_admin_c2_history_to_be_notified.notifyemail.get_value())
                (added_admin_c.notifyemail.get_value())

                (keyset_tech_c1.notifyemail.get_value())
                (keyset_tech_c2.notifyemail.get_value())
                (new_keyset_tech_c1.notifyemail.get_value())
                (new_keyset_tech_c2.notifyemail.get_value())

                (nsset_tech_c1.notifyemail.get_value())
                (nsset_tech_c2.notifyemail.get_value())
                (new_nsset_tech_c1.notifyemail.get_value())
                (new_nsset_tech_c2.notifyemail.get_value());

        BOOST_CHECK(found_email_addresses == etalon_email_addresses);
    }
    {
        std::map<std::string, std::string> params_etalon = boost::assign::map_list_of
            (std::string("type"),       std::string("3"))
            ("handle",                  domain_data_post_update.fqdn)
            ("ticket",                  input_svtrid)
            ("registrar",               registrar.name.get_value_or("") + " (" + registrar.url.get_value_or("") + ")")
            ("changes",                 "1")
            ("changes.object.authinfo",         "1")
            ("changes.object.authinfo.old",     domain_data_pre_update .authinfopw)
            ("changes.object.authinfo.new",     domain_data_post_update.authinfopw)
            ("changes.domain.registrant",       "1")
            ("changes.domain.registrant.old",   domain_data_pre_update .registrant.handle)
            ("changes.domain.registrant.new",   domain_data_post_update.registrant.handle)
            ("changes.domain.nsset",            "1")
            ("changes.domain.nsset.old",        domain_data_pre_update .nsset.get_value().handle)
            ("changes.domain.nsset.new",        domain_data_post_update.nsset.get_value().handle)
            ("changes.domain.keyset",           "1")
            ("changes.domain.keyset.old",       domain_data_pre_update .keyset.get_value().handle)
            ("changes.domain.keyset.new",       domain_data_post_update.keyset.get_value().handle)
            ("changes.domain.admin_c",          "1");
        {
            std::string admin_c_list_pre_update;

            std::vector<std::string> admin_contact_handles;
            for (const auto& a_c : domain_data_pre_update.admin_contacts) {
                admin_contact_handles.push_back(a_c.handle);
            }
            std::sort( admin_contact_handles.begin(), admin_contact_handles.end() );
            admin_c_list_pre_update = boost::algorithm::join(admin_contact_handles, " ");

            params_etalon["changes.domain.admin_c.old"]    = admin_c_list_pre_update;
        }
        {
            std::string admin_c_list_post_update;

            std::vector<std::string> admin_contact_handles;
            for (const auto& a_c : domain_data_post_update.admin_contacts)
            {
                admin_contact_handles.push_back(a_c.handle);
            }
            std::sort( admin_contact_handles.begin(), admin_contact_handles.end() );
            admin_c_list_post_update = boost::algorithm::join(admin_contact_handles, " ");

            params_etalon["changes.domain.admin_c.new"]    = admin_c_list_post_update;
        }


        BOOST_FOREACH(const MockMailerManager::MailData& mail, mocked_mailer_data_access.accumulated_data) {
            BOOST_CHECK_EQUAL( mail.from,           std::string() );
            BOOST_CHECK_EQUAL( mail.mailTemplate,   "notification_update" );
            BOOST_CHECK( mail.handles == ::LibFred::Mailer::Handles() );
            BOOST_CHECK( mail.attach == ::LibFred::Mailer::Attachments() );
            BOOST_CHECK_EQUAL( mail.reply_to,       std::string() );

            check_maps_are_equal( mail.params,      params_etalon );
        }
    }
}

BOOST_FIXTURE_TEST_CASE(test_process_empty_queue, has_autocomitting_ctx)
{
    std::shared_ptr<::LibFred::Mailer::Manager> mocked_mailer(new MockMailerManager);
    MockMailerManager& mocked_mailer_data_access = *reinterpret_cast<MockMailerManager*>( mocked_mailer.get() );

    BOOST_CHECK_EQUAL(
        Notification::process_one_notification_request(ctx, mocked_mailer),
        false
    );

    BOOST_CHECK_EQUAL(mocked_mailer_data_access.accumulated_data.size(), 0);
}

struct has_domain_uncomitted : Test::instantiate_db_template {

    ::LibFred::OperationContextCreator ctx;

    ::LibFred::InfoRegistrarData registrar;
    ::LibFred::InfoContactData registrant;
    ::LibFred::InfoDomainData domain;

    has_domain_uncomitted()
    :
        registrar(Test::registrar(ctx).info_data)
    {
        const std::string registrant_handle = "REGISTRANT1";
        {
            registrant =
                Test::exec(
                    ::LibFred::CreateContact(registrant_handle, registrar.handle)
                        .set_email("history.registrant1@nic.cz")
                        .set_notifyemail("registrant@notify.cz"),
                    ctx
                );
        }

        const std::string fqdn = "mydomain123.cz";
        {

            domain = Test::exec(
                ::LibFred::CreateDomain(fqdn, registrar.handle, registrant.handle ),
                ctx
            );
        }
    }
};

BOOST_FIXTURE_TEST_CASE(test_process_request_parallel_notification_request_access, has_domain_uncomitted)
{
    Notification::enqueue_notification(
        ctx,
        Notification::created,
        registrar.id,
        domain.crhistoryid,
        "abc"
    );

    ctx.commit_transaction();

    std::shared_ptr<::LibFred::Mailer::Manager> mocked_mailer(new MockMailerManager);

    ::LibFred::OperationContextCreator evil_uncomitted_parallel_transaction;
    Notification::process_one_notification_request(evil_uncomitted_parallel_transaction, mocked_mailer);

    ::LibFred::OperationContextCreator ctx_my_poor_transaction;
    BOOST_CHECK_THROW(
        Notification::process_one_notification_request(ctx_my_poor_transaction, mocked_mailer),
        Notification::FailedToLockRequest
    );
}

struct MockThrowingMailerManager : public ::LibFred::Mailer::Manager {

    virtual unsigned long long sendEmail(
      const std::string& _from,
      const std::string& _to,
      const std::string& _subject,
      const std::string& _mailTemplate,
      const ::LibFred::Mailer::Parameters& _params,
      const ::LibFred::Mailer::Handles& _handles,
      const ::LibFred::Mailer::Attachments& _attach,
      const std::string& _reply_to = std::string("")

    ) {
        throw ::LibFred::Mailer::NOT_SEND();
    }

    virtual bool checkEmailList(std::string &_email_list) const { return true; }
};

BOOST_FIXTURE_TEST_CASE(test_process_request_invalid_notify_email, has_domain_big_update)
{
    Notification::enqueue_notification(
        ctx,
        Notification::created,
        registrar.id,
        domain_data_post_update.historyid,
        "Svtrid-007"
    );

    std::shared_ptr<::LibFred::Mailer::Manager> mocked_mailer(new MockThrowingMailerManager);

    BOOST_CHECK_THROW(
        Notification::process_one_notification_request(ctx, mocked_mailer),
        Notification::FailedToSendMail
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
