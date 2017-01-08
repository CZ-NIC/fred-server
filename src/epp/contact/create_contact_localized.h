/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_CONTACT_LOCALIZED_H_8134301928FF41759B9B4E7061469BE4
#define CREATE_CONTACT_LOCALIZED_H_8134301928FF41759B9B4E7061469BE4

#include "src/epp/contact/contact_change.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/session_lang.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace Epp {
namespace Contact {

struct CreateContactInputData
{
    std::string name;
    std::string organization;
    std::vector<std::string> streets;
    std::string city;
    std::string state_or_province;
    std::string postal_code;
    std::string country_code;
    std::string telephone;
    std::string fax;
    std::string email;
    std::string notify_email;
    std::string VAT;
    std::string ident;
    Nullable<ContactChange::IdentType::Enum> identtype;
    boost::optional<std::string> authinfo;
    boost::optional<ContactDisclose> disclose;

    explicit CreateContactInputData(const ContactChange& src);
};

struct CreateContactLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const boost::posix_time::ptime crdate;

    CreateContactLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response_success_localized,
        const boost::posix_time::ptime& _crdate)
    :   epp_response_success_localized(_epp_response_success_localized),
        crdate(_crdate)
    { }
};

CreateContactLocalizedResponse create_contact_localized(
        const std::string& _contact_handle,
        const CreateContactInputData& _data,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix);

} // namespace Epp::Contact
} // namespace Epp

#endif
