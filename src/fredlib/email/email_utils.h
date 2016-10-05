#include "src/fredlib/mailer.h"

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <set>
#include <map>

namespace Fred
{

struct EmailData
{
    const std::set<std::string>                     recipient_email_addresses;
    const std::string                               template_name;
    const std::map<std::string, std::string>        template_parameters;

    EmailData(
        const std::set<std::string>&                _recipient_email_addresses,
        const std::string&                          _template_name,
        const std::map<std::string, std::string>&   _template_parameters)
    : recipient_email_addresses(_recipient_email_addresses),
      template_name(_template_name),
      template_parameters(_template_parameters)
    {}
};

struct FailedToSendMailToRecipient : std::exception
{
    const std::string                  failed_recipient;
    const std::set<std::string>        skipped_recipients;

    FailedToSendMailToRecipient(
        const std::string&             _failed_recipient,
        const std::set<std::string>&   _skipped_recipients)
    : failed_recipient(_failed_recipient),
      skipped_recipients(_skipped_recipients)
    {}

    virtual ~FailedToSendMailToRecipient() throw() {}
};

void send_email(boost::shared_ptr<Fred::Mailer::Manager> _mailer, const EmailData& _data)
{
    std::set<std::string> trimmed_recipient_email_addresses;
    BOOST_FOREACH(const std::string& email, _data.recipient_email_addresses)
    {
        trimmed_recipient_email_addresses.insert( boost::trim_copy(email) );
    }

    for (std::set<std::string>::const_iterator it = trimmed_recipient_email_addresses.begin();
            it != trimmed_recipient_email_addresses.end(); ++it)
    {
        try
        {
            _mailer->sendEmail(
                    "",
                    *it,
                    "",
                    _data.template_name,
                    _data.template_parameters,
                    Fred::Mailer::Handles(),
                    Fred::Mailer::Attachments());
        }
        catch(const Fred::Mailer::NOT_SEND& e)
        {
            throw FailedToSendMailToRecipient(
                    *it,
                    std::set<std::string>(it, trimmed_recipient_email_addresses.end()));
        }
    }
}

unsigned long long send_joined_addresses_email(boost::shared_ptr<Fred::Mailer::Manager> _mailer, const EmailData& _data)
{
    std::set<std::string> trimmed_recipient_email_addresses;
    BOOST_FOREACH(const std::string& email, _data.recipient_email_addresses)
    {
        trimmed_recipient_email_addresses.insert( boost::trim_copy(email) );
    }

    std::ostringstream oss;
    for (std::set<std::string>::const_iterator it = trimmed_recipient_email_addresses.begin();
            it != trimmed_recipient_email_addresses.end(); ++it)
    {
        oss << *it;
    }
    try
    {
        return _mailer->sendEmail(
                "",
                oss.str(),
                "",
                _data.template_name,
                _data.template_parameters,
                Fred::Mailer::Handles(),
                Fred::Mailer::Attachments());
    }
    catch(const Fred::Mailer::NOT_SEND& e)
    {
        throw FailedToSendMailToRecipient(
                oss.str(),
                std::set<std::string>());
    }
}

} // namespace Fred
