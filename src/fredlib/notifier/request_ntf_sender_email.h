#ifndef REQUEST_NTF_SENDER_EMAIL_H__
#define REQUEST_NTF_SENDER_EMAIL_H__

#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

#include "request_notification.h"
#include "log/logger.h"


namespace Fred {


template<class T>
std::string container2comma_list(const T &_cont)
{
    if (_cont.empty()) {
        return "";
    }

    std::stringstream tmp;
    typename T::const_iterator it = _cont.begin();
    tmp << *it;
    for (++it; it != _cont.end(); ++it) {
        tmp << ", " << *it;
    }
    return tmp.str();
}



class NotificationEmailSender
{
public:
    void send(const RequestNotification &_ntf) const
    {
        Logging::Context context("notifier:email_sender");
        LOGGER(PACKAGE).info(boost::format(
                    "request_id=%1%  object_id=%2%  request_type=%3%")
                    % _ntf.get_request_id()
                    % _ntf.get_object_id()
                    % _ntf.get_request_type());

        /* collect email addreses from notification recipients */
        std::string anyarray = "{" + container2comma_list(_ntf.get_recipients()) + "}";
        Database::Connection conn = Database::Manager::acquire();
        Database::Result nemails = conn.exec_params(
                "SELECT notifyemail FROM contact_history"
                " WHERE historyid = ANY ($1::integer[]) AND notifyemail IS NOT NULL",
                Database::query_param_list(anyarray));

        std::vector<std::string> rcpts_emails;
        for (unsigned int i = 0; i < nemails.size(); ++i) {
            std::string e = boost::trim_copy(static_cast<std::string>(nemails[i][0]));
            if (find(rcpts_emails.begin(), rcpts_emails.end(), e) ==  rcpts_emails.end()) {
                rcpts_emails.push_back(e);
            }
        }
        LOGGER(PACKAGE).debug(boost::format("recipients: %1%")
                % container2comma_list(rcpts_emails));
    }


private:
    std::string get_template(const unsigned short &_req_type) const
    {
    }
};

}


#endif /*REQUEST_NTF_SENDER_EMAIL_H__*/

