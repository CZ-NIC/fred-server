/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef SEND_JOINED_ADDRESS_EMAIL_HH_AE7242E0D3EE4527ADDE0B27DE242FE7
#define SEND_JOINED_ADDRESS_EMAIL_HH_AE7242E0D3EE4527ADDE0B27DE242FE7

#include "src/libfred/mailer.hh"

#include <memory>
#include <string>
#include <set>
#include <map>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

struct FailedToSendMailToRecipient : std::exception
{
    const char* what() const noexcept
    {
        return "failed to send mail to recipient";
    }
};

struct EmailData
{
    EmailData(
            const std::set<std::string>& _recipient_email_addresses,
            const std::string& _template_name,
            const std::map<std::string, std::string>& _template_parameters,
            const std::vector<unsigned long long>& _attachments)
        : recipient_email_addresses(_recipient_email_addresses),
          template_name(_template_name),
          template_parameters(_template_parameters),
          attachments(_attachments)
    {
    }
    const std::set<std::string> recipient_email_addresses;
    const std::string template_name;
    const std::map<std::string, std::string> template_parameters;
    const std::vector<unsigned long long> attachments;
};

unsigned long long send_joined_addresses_email(
        std::shared_ptr<LibFred::Mailer::Manager> mailer,
        const EmailData& data);

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
