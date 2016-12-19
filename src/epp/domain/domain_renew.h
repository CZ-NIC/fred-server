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

/**
 *  @file
 */

#ifndef EPP_DOMAIN_RENEW_H_a8e407273a344ff38249263566f3e51f
#define EPP_DOMAIN_RENEW_H_a8e407273a344ff38249263566f3e51f

#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"
#include "src/epp/domain/domain_registration_time.h"
#include "src/epp/domain/domain_enum_validation.h"

#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

    struct DomainRenewInputData
    {
        std::string fqdn;
        std::string current_exdate;
        DomainRegistrationTime period;
        std::vector<Epp::ENUMValidationExtension> enum_validation_list;

        DomainRenewInputData(
            const std::string& _fqdn,
            const std::string& _current_exdate,
            const DomainRegistrationTime& _period,
            const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list)
        : fqdn(_fqdn)
        , current_exdate(_current_exdate)
        , period(_period)
        , enum_validation_list(_enum_validation_list)
        {}
    };


struct LocalizedRenewDomainResponse {
    const LocalizedSuccessResponse ok_response;
    const boost::gregorian::date expiration_date;

    LocalizedRenewDomainResponse(
        const LocalizedSuccessResponse& _ok_response,
        const boost::gregorian::date& _expiration_date
    ) :
        ok_response(_ok_response),
        expiration_date(_expiration_date)
    { }
};

LocalizedRenewDomainResponse domain_renew(
    const DomainRenewInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string& _dont_notify_client_transaction_handles_with_this_prefix,
    const bool _rifd_epp_operations_charging
);

}

#endif
