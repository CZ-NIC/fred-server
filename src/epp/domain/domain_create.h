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

#ifndef DOMAIN_CREATE_H_B6C01C595C524BD983B8E61AFD2C607A
#define DOMAIN_CREATE_H_B6C01C595C524BD983B8E61AFD2C607A

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/domain/domain_registration_time.h"
#include "src/epp/domain/domain_enum_validation.h"

#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

namespace Epp {

    struct DomainCreateInputData
    {
        std::string fqdn;
        std::string registrant;
        std::string nsset;
        std::string keyset;
        boost::optional<std::string> authinfo;
        DomainRegistrationTime period;
        std::vector<std::string> admin_contacts;
        std::vector<Epp::ENUMValidationExtension> enum_validation_list;


        DomainCreateInputData(
            const std::string& _fqdn,
            const std::string& _registrant,
            const std::string& _nsset,
            const std::string& _keyset,
            const boost::optional<std::string>& _authinfo,
            const DomainRegistrationTime& _period,
            const std::vector<std::string>& _admin_contacts,
            const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list)
        : fqdn(_fqdn)
        , registrant(_registrant)
        , nsset(_nsset)
        , keyset(_keyset)
        , authinfo(_authinfo)
        , period(_period)
        , admin_contacts(_admin_contacts)
        , enum_validation_list(_enum_validation_list)
        {}
    };


struct LocalizedCreateDomainResponse {
    const LocalizedSuccessResponse ok_response;
    const boost::posix_time::ptime crtime;
    const boost::gregorian::date expiration_date;

    LocalizedCreateDomainResponse(
        const LocalizedSuccessResponse& _ok_response,
        const boost::posix_time::ptime& _crtime,
        const boost::gregorian::date& _expiration_date
    ) :
        ok_response(_ok_response),
        crtime(_crtime),
        expiration_date(_expiration_date)
    { }
};

LocalizedCreateDomainResponse domain_create(
    const DomainCreateInputData& _data,
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
