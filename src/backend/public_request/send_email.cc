#include "src/backend/public_request/send_email.hh"

#include <boost/algorithm/string/trim.hpp>

#include <sstream>

namespace Fred {
namespace Backend {
namespace PublicRequest {

unsigned long long send_joined_addresses_email(
        std::shared_ptr<LibFred::Mailer::Manager> mailer,
        const EmailData& data)
{
    std::ostringstream recipients;
    for (const auto& email: data.recipient_email_addresses)
    {
        recipients << boost::trim_copy(email) << ' ';
    }

    try
    {
        return mailer->sendEmail(
                "",
                recipients.str(),
                "",
                data.template_name,
                data.template_parameters,
                LibFred::Mailer::Handles(),
                data.attachments);
    }
    catch (const LibFred::Mailer::NOT_SEND&)
    {
        throw FailedToSendMailToRecipient();
    }
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
