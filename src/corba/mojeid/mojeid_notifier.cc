#include "mojeid_notifier.h"
#include "fredlib/notifier/request_ntf_handler_contact.h"
#include "fredlib/notifier/request_ntf_sender_email.h"


namespace Fred {
namespace MojeID {


void notify_contact_create(const unsigned long long &_request_id,
                           boost::shared_ptr<Fred::Mailer::Manager> _mailer)
{
    Logging::Context ctx("notifier");

    Fred::RequestNotification ntf(_request_id);

    if (ntf.get_request_type() == CMD_CONTACT_CREATE) {
        Fred::request_contact_create(ntf);
        ntf.set_template_name("notification_create");
        NotificationEmailSender sender(_mailer);
        sender.send(ntf);
    }
}


void notify_contact_update(const unsigned long long &_request_id,
                           boost::shared_ptr<Fred::Mailer::Manager> _mailer)
{
    Logging::Context ctx("notifier");

    Fred::RequestNotification ntf(_request_id);

    if (ntf.get_request_type() == CMD_CONTACT_UPDATE) {
        Fred::request_contact_update(ntf);
        ntf.set_template_name("notification_update");
        NotificationEmailSender sender(_mailer);
        sender.send(ntf);
    }
}


void notify_contact_transfer(const unsigned long long &_request_id,
                           boost::shared_ptr<Fred::Mailer::Manager> _mailer)
{
    Logging::Context ctx("notifier");

    Fred::RequestNotification ntf(_request_id);

    if (ntf.get_request_type() == CMD_CONTACT_TRANSFER) {
        Fred::request_contact_transfer(ntf);
        ntf.set_template_name("notification_transfer");
        NotificationEmailSender sender(_mailer);
        sender.send(ntf);
    }
}


}
}

