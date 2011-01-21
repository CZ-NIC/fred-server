#ifndef REQUEST_NTF_SENDER_EMAIL_H__
#define REQUEST_NTF_SENDER_EMAIL_H__

#include "request_notification.h"
#include "log/logger.h"


namespace Fred {


class NotificationEmailSender
{
public:
    void send(const RequestNotification &_ntf) const
    {
        LOGGER(PACKAGE).debug(boost::format(
                    "notifier(email_sender): request_id=%1%  object_id=%2%")
                    % _ntf.get_request_id() % _ntf.get_object_id());
    }
};

}


#endif /*REQUEST_NTF_SENDER_EMAIL_H__*/

