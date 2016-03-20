#include "src/fredlib/notifier2/process_one_notification_request.h"

#include "src/fredlib/notifier2/gather_email_data/gather_email_addresses.h"
#include "src/fredlib/notifier2/gather_email_data/gather_email_content.h"
#include "src/fredlib/notifier2/exception.h"

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Notification {

struct email_data {
    const std::set<std::string>                 recipient_email_addresses;
    const std::string                           template_name;
    const std::map<std::string, std::string>    template_parameters;

    email_data(
        const std::set<std::string>&                _recipient_email_addresses,
        const std::string&                          _template_name,
        const std::map<std::string, std::string>&   _template_parameters
    ) :
        recipient_email_addresses(_recipient_email_addresses),
        template_name(_template_name),
        template_parameters(_template_parameters)
    { }
};

/**
 * @returns template name as specified by table 'mail_type' column 'name'
 */
static std::string get_template_name(notified_event _event) {

    switch(_event) {
        case created        : return "notification_create";
        case updated        : return "notification_update";
        case transferred    : return "notification_transfer";
        case renewed        : return "notification_renew";
        case deleted        : return "notification_delete";
    }

    throw ExceptionUnknownEmailTemplate();
}

struct FailedToSendMailToRecipient {
    const std::string failed_recipient;
    const std::set<std::string> skipped_recipients;

    FailedToSendMailToRecipient(
        const std::string& _failed_recipient,
        const std::set<std::string>& _skipped_recipients
    ) :
        failed_recipient(_failed_recipient),
        skipped_recipients(_skipped_recipients)
    { }
};

/**
 * @throws FailedToSendMailToRecipient
 */
static void send_email(boost::shared_ptr<Fred::Mailer::Manager> _mailer, const email_data& _data) {

    std::set<std::string> trimmed_recipient_email_addresses;
    BOOST_FOREACH(const std::string& email, _data.recipient_email_addresses) {
        trimmed_recipient_email_addresses.insert( boost::trim_copy(email) );
    }

    for(
        std::set<std::string>::const_iterator it = trimmed_recipient_email_addresses.begin();
        it != trimmed_recipient_email_addresses.end();
        ++it
    ) {
        try {
            _mailer->sendEmail(
                "",
                *it,
                "",
                _data.template_name,
                _data.template_parameters,
                Fred::Mailer::Handles(),
                Fred::Mailer::Attachments()
            );
        } catch(const Fred::Mailer::NOT_SEND& e) {
            throw FailedToSendMailToRecipient(
                *it,
                std::set<std::string>(it, trimmed_recipient_email_addresses.end())
            );
        }
    }
}

bool process_one_notification_request(Fred::OperationContext& _ctx, boost::shared_ptr<Fred::Mailer::Manager> _mailer) {

    std::string log_prefix = "process_one_notification_request() ";

    struct process_postgres_locking_exception {
        static Database::Result get_notification_to_send(Fred::OperationContext& _ctx) {

            /* There is no hard guarantee that records in notification_queue are unique. It is no problem though. */
            try {
                return _ctx.get_conn().exec(
                    /* lock exclusively... */
                    "WITH locked AS ("
                        "SELECT "
                            "change, "
                            "done_by_registrar, "
                            "historyid_post_change, "
                            "svtrid "
                        "FROM notification_queue "
                        "FOR UPDATE NOWAIT "
                        "LIMIT 1 "
                    ")"
                    /* ...and delete (it is locked until commit and deleted immediately after) */
                    "DELETE "
                    "FROM notification_queue AS q "
                    "USING locked "
                    "WHERE "
                        "q.change = locked.change "
                        "AND q.done_by_registrar = locked.done_by_registrar "
                        "AND q.historyid_post_change = locked.historyid_post_change "
                        "AND q.svtrid = locked.svtrid "
                    "RETURNING "
                        "locked.*, "
                        "( "
                            "SELECT "
                                "e_o_t.name "
                            "FROM    object_history      o_h "
                            "JOIN    object_registry     o_r     USING(id) "
                            "JOIN    enum_object_type    e_o_t  ON o_r.type = e_o_t.id "
                            "WHERE o_h.historyid = locked.historyid_post_change "
                        ") AS object_type_ "
                );

            } catch(const std::exception& ex) {
                const std::string what_string(ex.what());
                if(what_string.find("could not obtain lock on row in relation \"notification_queue\"") != std::string::npos) {
                    throw FailedToLockRequest();
                }
                throw;
            }
        }
    };

    try {

        const Database::Result notification_to_send_res = process_postgres_locking_exception::get_notification_to_send(_ctx);

        if(notification_to_send_res.size() < 1) {
            _ctx.get_log().info(log_prefix + "no record found in notification_queue");
            return false;
        }

        notification_request request(
            EventOnObject(
                Fred::object_type_from_db_handle( static_cast<std::string>( notification_to_send_res[0]["object_type_"] ) ),
                notified_event_from_db_handle( static_cast<std::string>( notification_to_send_res[0]["change"] ) )
            ),
            static_cast<unsigned long long>( notification_to_send_res[0]["done_by_registrar"] ),
            static_cast<unsigned long long>( notification_to_send_res[0]["historyid_post_change"] ),
            static_cast<std::string>( notification_to_send_res[0]["svtrid"] )
        );

        log_prefix +=
            "event=\"" + to_db_handle( request.event.get_event() ) + "\" "
            "object_type=\"" + to_db_handle( request.event.get_type() ) + "\" "
            "done_by_registrar=\"" + boost::lexical_cast<std::string>( request.done_by_registrar ) + "\" "
            "history_id_post_change=\"" + boost::lexical_cast<std::string>( request.history_id_post_change ) + "\" "
            "svtrid=\"" + request.svtrid + "\" ";

        const email_data data(
            gather_email_addresses( _ctx, request.event, request.history_id_post_change ),
            get_template_name( request.event.get_event() ),
            gather_email_content(_ctx, request)
        );

        // Ticket #6547 send update notifications only if there are some changes
        if( request.event.get_event() == updated ) {
            const std::map<std::string, std::string>::const_iterator changes_it = data.template_parameters.find("changes");
            if( changes_it == data.template_parameters.end() ) {
                throw ExceptionMissingChangesFlagInUpdateNotificationContent();
            }
            if( changes_it->second == "0" ) {
                return true;
            }
        }

        _ctx.get_log().info( log_prefix + "completed - transaction not yet comitted" );

        try {

            send_email(_mailer, data);

        } catch(const FailedToSendMailToRecipient& e) {

            throw FailedToSendMail(
                request,
                e.failed_recipient,
                e.skipped_recipients,
                data.template_name,
                data.template_parameters
            );
        }

        return true;

    } catch(const ExceptionInterface& e) {
        _ctx.get_log().error( log_prefix + "exception :" + std::string(e.what()) );
        throw;

    } catch(const std::exception& e) {
        _ctx.get_log().error( log_prefix + "exception :" + std::string(e.what()) );
        throw;

    } catch(...) {
        _ctx.get_log().error( log_prefix + "unknown exception" );
        throw;

    }
}

}
