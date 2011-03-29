#ifndef REQUEST_NTF_SENDER_EMAIL_H__
#define REQUEST_NTF_SENDER_EMAIL_H__

#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

#include "request_notification.h"
#include "log/logger.h"
#include "log/context.h"
#include "fredlib/mailer.h"


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
    NotificationEmailSender()
    {
    }


    NotificationEmailSender(boost::shared_ptr<Mailer::Manager> _mm) 
        : mm_(_mm)
    {
    }


    void send(const RequestNotification &_ntf) const
    {
        Logging::Context context("email_sender");
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
        if (rcpts_emails.empty()) {
            LOGGER(PACKAGE).debug("recipients: empty list; send canceled");
            return;
        }
        else {
            LOGGER(PACKAGE).debug(boost::format("recipients: %1%")
                    % container2comma_list(rcpts_emails));
        }

        Mailer::Handles     m_handles;
        Mailer::Attachments m_attachs;

        std::string         m_template_name = _ntf.get_template_name();
        Mailer::Parameters  m_params;

        m_params["ticket"]    = _ntf.get_ticket_id();
        m_params["handle"]    = _ntf.get_object_handle();
        m_params["type"]      = boost::lexical_cast<std::string>(_ntf.get_object_type());
        m_params["registrar"] = _ntf.get_registrar_info();

        const ChangesMap &changes = _ntf.get_object_changes();
        m_params["changes"] = changes.size() > 0 ? "1" : "0";
        ChangesMap::const_iterator it = changes.begin();
        for (; it != changes.end(); ++it) {
            m_params["changes." + it->first] = "1";
            m_params["changes." + it->first + ".old"] = it->second.first;
            m_params["changes." + it->first + ".new"] = it->second.second;
        }


        for (unsigned int i = 0; i < rcpts_emails.size(); ++i) {
            try {
                unsigned long long msg_id = mm_->sendEmail("", rcpts_emails[i], "", m_template_name, m_params, m_handles, m_attachs);
                try {
                    _ntf.save_relation(msg_id);
                }
                catch (...) {
                    LOGGER(PACKAGE).error(boost::format("request notification save failure"
                                " (request_id=%1% msg_id=%2%)") % _ntf.get_request_id() % msg_id);
                }
            }
            catch (...) {
                LOGGER(PACKAGE).error(boost::format("sending failure (email=%1%  ticket=%2%)")
                        % rcpts_emails[i] % m_params["ticket"]);
            }
        }
    }


private:
    boost::shared_ptr<Mailer::Manager> mm_;
};

}


#endif /*REQUEST_NTF_SENDER_EMAIL_H__*/

