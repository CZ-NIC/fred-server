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

#ifndef RENEW_DOMAIN_LOCALIZED_H_AADAB1519F8C4D6BAA4E0766858D8E49
#define RENEW_DOMAIN_LOCALIZED_H_AADAB1519F8C4D6BAA4E0766858D8E49

#include "src/epp/domain/impl/domain_enum_validation.h"
#include "src/epp/domain/impl/domain_registration_time.h"
#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/session_lang.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <set>
#include <string>

namespace Epp {
namespace Domain {

struct RenewDomainInputData
{
    std::string fqdn;
    std::string current_exdate;
    DomainRegistrationTime period;
    std::vector<EnumValidationExtension> enum_validation_list;

    RenewDomainInputData(
        const std::string& _fqdn,
        const std::string& _current_exdate,
        const DomainRegistrationTime& _period,
        const std::vector<EnumValidationExtension>& _enum_validation_list)
    :   fqdn(_fqdn),
        current_exdate(_current_exdate),
        period(_period),
        enum_validation_list(_enum_validation_list)
    { }
};

struct RenewDomainLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const boost::gregorian::date expiration_date;

    RenewDomainLocalizedResponse(
        const EppResponseSuccessLocalized& _epp_response_success_localized,
        const boost::gregorian::date& _expiration_date)
    :
        epp_response_success_localized(_epp_response_success_localized),
        expiration_date(_expiration_date)
    { }
};

RenewDomainLocalizedResponse renew_domain_localized(
        const RenewDomainInputData& _data,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        bool _epp_notification_disabled,
        const std::string& _dont_notify_client_transaction_handles_with_this_prefix,
        bool _rifd_epp_operations_charging);

} // namespace Epp::Domain
} // namespace Epp

#endif
