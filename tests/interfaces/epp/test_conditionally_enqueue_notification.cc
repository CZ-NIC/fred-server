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

#include "tests/interfaces/epp/contact/fixture.h"
#include "tests/interfaces/epp/util.h"
#include "tests/setup/fixtures.h"

#include "src/corba/mailer_manager.h"
#include "src/epp/impl/conditionally_enqueue_notification.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "src/epp/session_lang.h"
#include "src/fredlib/notifier/process_one_notification_request.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ConditionallyEnqueueNotification)

struct MockMailerManager : public Fred::Mailer::Manager {
    struct MailData {
        std::string from;
        std::string to;
        std::string subject;
        std::string mailTemplate;
        Fred::Mailer::Parameters params;
        Fred::Mailer::Handles handles;
        Fred::Mailer::Attachments attach;
        std::string reply_to;

        MailData(
            const std::string& _from,
            const std::string& _to,
            const std::string& _subject,
            const std::string& _mailTemplate,
            const Fred::Mailer::Parameters& _params,
            const Fred::Mailer::Handles& _handles,
            const Fred::Mailer::Attachments& _attach,
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
      const Fred::Mailer::Parameters& _params,
      const Fred::Mailer::Handles& _handles,
      const Fred::Mailer::Attachments& _attach,
      const std::string& _reply_to = std::string("")

    ) throw (Fred::Mailer::NOT_SEND) {
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

template<bool SystemReg>struct has_updated_contact_and_empty_notification_queue : virtual Test::Fixture::instantiate_db_template {
    boost::shared_ptr<Fred::Mailer::Manager> mailer;
    unsigned long long post_update_contact_history_id;
    unsigned long long registrar_id;
    static const bool epp_notification_disabled = true;
    static const bool epp_notification_enabled = false;
    static const Epp::SessionLang::Enum epp_session_lang_default = Epp::SessionLang::en;

    has_updated_contact_and_empty_notification_queue()
    :
        mailer(new MockMailerManager)
    {
        Fred::OperationContextCreator ctx;

        const std::string reg_handle = "REGISTRAR1";
        Fred::CreateRegistrar(reg_handle).set_system(SystemReg).exec(ctx);
        const Fred::InfoRegistrarData registrar = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;
        registrar_id = registrar.id;

        const std::string contact_handle = "CONTACT1";
        Fred::CreateContact(contact_handle, registrar.handle).exec(ctx);
        const Fred::InfoContactData contact = Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;

        post_update_contact_history_id = Fred::UpdateContactByHandle(contact.handle, registrar.handle)
            .set_email(contact.email)
            .exec(ctx);

        while( Notification::process_one_notification_request(ctx, mailer) ) { }

        ctx.commit_transaction();
    }
};

typedef has_updated_contact_and_empty_notification_queue<true> has_system_registrar_updated_contact_and_empty_notification_queue;
typedef has_updated_contact_and_empty_notification_queue<false> has_nonsystem_registrar_updated_contact_and_empty_notification_queue;

BOOST_FIXTURE_TEST_CASE(notification_created, has_nonsystem_registrar_updated_contact_and_empty_notification_queue)
{
    Epp::conditionally_enqueue_notification(
            Notification::updated,
            post_update_contact_history_id,
            Epp::SessionData(
                    registrar_id,
                    epp_session_lang_default,
                    "srv-trx-007",
                    0ULL),
            Epp::NotificationData(
                    "cl-trx-007",
                    epp_notification_enabled,
                    "somethingElseAndNotMatching"));

    Fred::OperationContextCreator ctx;

    BOOST_CHECK( Notification::process_one_notification_request(ctx, mailer) );

    ctx.commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(notification_created_because_of_nonsystem_registrar, has_nonsystem_registrar_updated_contact_and_empty_notification_queue)
{
    Epp::conditionally_enqueue_notification(
            Notification::updated,
            post_update_contact_history_id,
            Epp::SessionData(
                    registrar_id,
                    epp_session_lang_default,
                    "srv-trx-007",
                    0ULL),
            Epp::NotificationData(
                    "DOnotNOTIFY-cl-trx-007",
                    epp_notification_enabled,
                    "DOnotNOTIFY"));

    Fred::OperationContextCreator ctx;

    BOOST_CHECK( Notification::process_one_notification_request(ctx, mailer) );

    ctx.commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(notification_created_because_of_nonmatching_prefix, has_system_registrar_updated_contact_and_empty_notification_queue)
{
    Epp::conditionally_enqueue_notification(
            Notification::updated,
            post_update_contact_history_id,
            Epp::SessionData(
                    registrar_id,
                    epp_session_lang_default,
                    "srv-trx-007",
                    0ULL),
            Epp::NotificationData(
                    "DOnotNOTIFY-cl-trx-007",
                    epp_notification_enabled,
                    "somethingElseAndNotMatching"));

    Fred::OperationContextCreator ctx;

    BOOST_CHECK( Notification::process_one_notification_request(ctx, mailer) );

    ctx.commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(notification_not_created_because_of_sys_reg_and_prefix, has_system_registrar_updated_contact_and_empty_notification_queue)
{
    Epp::conditionally_enqueue_notification(
            Notification::updated,
            post_update_contact_history_id,
            Epp::SessionData(
                    registrar_id,
                    epp_session_lang_default,
                    "srv-trx-007",
                    0ULL),
            Epp::NotificationData(
                    "DOnotNOTIFY-cl-trx-007",
                    epp_notification_enabled,
                    "DOnotNOTIFY"));

    Fred::OperationContextCreator ctx;

    BOOST_CHECK( !Notification::process_one_notification_request(ctx, mailer) );

    ctx.commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(notification_not_created_because_of_config, has_nonsystem_registrar_updated_contact_and_empty_notification_queue)
{
    Epp::conditionally_enqueue_notification(
            Notification::updated,
            post_update_contact_history_id,
            Epp::SessionData(
                    registrar_id,
                    epp_session_lang_default,
                    "srv-trx-007",
                    0ULL),
            Epp::NotificationData(
                    "DOnotNOTIFY-cl-trx-007",
                    epp_notification_disabled,
                    "somethingElseAndNotMatching"));

    Fred::OperationContextCreator ctx;

    BOOST_CHECK( !Notification::process_one_notification_request(ctx, mailer) );

    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
